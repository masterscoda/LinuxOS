/*
 * Editied by Scott Bossard and renamed scullbuffer.c
 *
 * Main edits were to remove multiple devices and allow only one. And to rename pipe to buffer
 *
 * Original authors:
 * pipe.c -- fifo driver for scull
 *
 * Copyright (C) 2001 Alessandro Rubini and Jonathan Corbet
 * Copyright (C) 2001 O'Reilly & Associates
 *
 * The source code in this file can be freely used, adapted,
 * and redistributed in source or binary form, so long as an
 * acknowledgment appears in derived source files.  The citation
 * should list that the code comes from the book "Linux Device
 * Drivers" by Alessandro Rubini and Jonathan Corbet, published
 * by O'Reilly & Associates.   No warranty is attached;
 * we cannot take responsibility for errors or fitness for use.
 *
 */

#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>	/* printk(), min() */
#include <linux/slab.h>		/* kmalloc() */
#include <linux/fs.h>		/* everything... */
#include <linux/proc_fs.h>
#include <linux/errno.h>	/* error codes */
#include <linux/types.h>	/* size_t */
#include <linux/fcntl.h>
#include <linux/poll.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include "scull.h"		/* local definitions */

#define init_MUTEX(_m) sema_init(_m, 1);


struct scull_buffer {
				wait_queue_head_t inq, outq;       /* read and write queues */
        char *buffer, *end;                       /* begin of buf */
        int buffersize;                    /* used in pointer arithmetic */
        char *rp, *wp;                     /* where to read, where to write */
        int nreaders, nwriters;            /* number of openings for r/w */
        struct semaphore sem;              /* mutual exclusion semaphore */
        struct cdev cdev;                  /* Char device structure */
};

/* parameters */
static int scull_p_nr_devs = 1;	/* number of pipe devices */
int NITEMS = 20; /* default value of NITEMS */
dev_t scull_p_devno;			/* Our first device number */

module_param(NITEMS, int, 0); /* NITEMS as a module parameter */

static struct scull_buffer *scull_p_device;

static int spacefree(struct scull_buffer *dev);

/*
 * Open and close
 */
static int scull_p_open(struct inode *inode, struct file *filp)
{
	struct scull_buffer *dev;

	dev = container_of(inode->i_cdev, struct scull_buffer, cdev);
	filp->private_data = dev;

	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;

	/* use f_mode,not  f_flags: it's cleaner (fs/open.c tells why) */
	if (filp->f_mode & FMODE_READ)
		dev->nreaders++;
	if (filp->f_mode & FMODE_WRITE)
		dev->nwriters++;
	up(&dev->sem);

	return nonseekable_open(inode, filp);
}

static int scull_p_release(struct inode *inode, struct file *filp)
{
	struct scull_buffer *dev = filp->private_data;

	down(&dev->sem);
	if (filp->f_mode & FMODE_READ)
		dev->nreaders--;
	if (filp->f_mode & FMODE_WRITE)
		dev->nwriters--;
	up(&dev->sem);
	return 0;
}

/*
 * Data management: read and write
*/
static ssize_t scull_p_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	struct scull_buffer *dev = filp->private_data;

	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;

	PDEBUG("\" (scull_p_read) dev->wp:%p    dev->rp:%p\" \n",dev->wp,dev->rp);

	while (dev->rp == dev->wp) { /* nothing to read */
    if(dev->nwriters == 0)
    {
      up(&dev->sem);  /* release the lock */
      return 0; /* no writers so get out */
    }
		up(&dev->sem); /* release the lock */
		if (filp->f_flags & O_NONBLOCK)
			return -EAGAIN;
		PDEBUG("\"%s\" reading: going to sleep\n", current->comm);
		if (wait_event_interruptible(dev->inq, (dev->rp != dev->wp)))
			return -ERESTARTSYS; /* signal: tell the fs layer to handle it */
		/* otherwise loop, but first reacquire the lock */
		if (down_interruptible(&dev->sem))
			return -ERESTARTSYS;
	}
	/* ok, data is there, return something */
	if (dev->wp > dev->rp)
		count = min(count, (size_t)(dev->wp - dev->rp));
	else /* the write pointer has wrapped, return data up to dev->end */
		count = min(count, (size_t)(dev->buffer + dev->buffersize - dev->rp));
	if (copy_to_user(buf, dev->rp, count)) {
		up (&dev->sem);
		return -EFAULT;
	}
	dev->rp += count;
	if (dev->rp == dev->buffer + dev->buffersize)
		dev->rp = dev->buffer; /* wrapped */
	up (&dev->sem);

	PDEBUG("\"%s\" did read %li bytes\n",current->comm, (long)count);
	return count;
}

/* Wait for space for writing; caller must hold device semaphore.  On
 * error the semaphore will be released before returning. */
static int scull_getwritespace(struct scull_buffer *dev, struct file *filp)
{
	while (spacefree(dev) == 0) { /* full */
		DEFINE_WAIT(wait);

		up(&dev->sem);
		if (filp->f_flags & O_NONBLOCK)
			return -EAGAIN;
		PDEBUG("\"%s\" writing: going to sleep\n",current->comm);
		prepare_to_wait(&dev->outq, &wait, TASK_INTERRUPTIBLE);
		if (spacefree(dev) == 0)
			schedule();
		finish_wait(&dev->outq, &wait);
		if (signal_pending(current))
			return -ERESTARTSYS; /* signal: tell the fs layer to handle it */
		if (down_interruptible(&dev->sem))
			return -ERESTARTSYS;
	}
	return 0;
}

/* How much space is free? */
static int spacefree(struct scull_buffer *dev)
{
	if (dev->rp == dev->wp)
		return dev->buffersize - 1;
	return ((dev->rp + dev->buffersize - dev->wp) % dev->buffersize) - 1;
}

static ssize_t scull_p_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
	struct scull_buffer *dev = filp->private_data;
  int result;

	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;
  /* Make sure there's space to write */
  result = scull_getwritespace(dev, filp);
	if (result)
		return result; /* scull_getwritespace called up(&dev->sem) */

	while (spacefree(dev) == 0) {
    if(dev->nreaders == 0)
    {
      up(&dev->sem);  /* release the lock */
      return 0; /* no readers so get out */
    }
		up(&dev->sem); /* release the lock */
		if (filp->f_flags & O_NONBLOCK)
			return -EAGAIN;
		PDEBUG("\"%s\" writinging: going to sleep\n", current->comm);
		/* otherwise loop, but first reacquire the lock */
		if (down_interruptible(&dev->sem))
			return -ERESTARTSYS;
	}

	/* ok, space is there, accept something */
	count = min(count, (size_t)spacefree(dev));
	if (dev->wp >= dev->rp)
		count = min(count, (size_t)(dev->buffer + dev->buffersize - dev->wp)); /* to end-of-buf */
	else /* the write pointer has wrapped, fill up to rp-1 */
		count = min(count, (size_t)(dev->rp - dev->wp - 1));
	PDEBUG("Going to accept %li bytes to %p from %p\n", (long)count, dev->wp, buf);
	if (copy_from_user(dev->wp, buf, count)) {
		up (&dev->sem);
		return -EFAULT;
	}
	dev->wp += count;
	if (dev->wp == dev->buffer + dev->buffersize)
		dev->wp = dev->buffer; /* wrapped */
	PDEBUG("\" (scull_p_write) dev->wp:%p    dev->rp:%p\" \n",dev->wp,dev->rp);
	up(&dev->sem);

	PDEBUG("\"%s\" did write %li bytes\n",current->comm, (long)count);
	return count;
}

static unsigned int scull_p_poll(struct file *filp, poll_table *wait)
{
	struct scull_buffer *dev = filp->private_data;
	unsigned int mask = 0;

	/*
	 * The buffer is circular; it is considered full
	 * if "wp" is right behind "rp" and empty if the
	 * two are equal.
	 */
	down(&dev->sem);
	poll_wait(filp, &dev->inq,  wait);
	poll_wait(filp, &dev->outq, wait);
	if (dev->rp != dev->wp)
		mask |= POLLIN | POLLRDNORM;	/* readable */
	if (spacefree(dev))
		mask |= POLLOUT | POLLWRNORM;	/* writable */
	up(&dev->sem);
	return mask;
}

/*
 * The file operations for the pipe device
 * (some are overlayed with bare scull)
 */
struct file_operations scull_p_fops = {
	.owner =	THIS_MODULE,
	.llseek =	no_llseek,
	.read =		scull_p_read,
	.write =	scull_p_write,
	.poll =		scull_p_poll,
	.open =		scull_p_open,
	.release =	scull_p_release,
};

/*
 * Set up a cdev entry.
 */
static void scull_p_setup_cdev(struct scull_buffer *dev)
{
	int err;

	cdev_init(&dev->cdev, &scull_p_fops);
	dev->cdev.owner = THIS_MODULE;
	err = cdev_add (&dev->cdev, scull_p_devno, 1);
	/* Fail gracefully if need be */
	if (err)
		printk(KERN_NOTICE "Error %d adding scullbuffer", err);
}

/*
 * Initialize the buffer devs; return how many we did.
 */
int scull_p_init(dev_t firstdev)
{
	int result;

	result = register_chrdev_region(firstdev, scull_p_nr_devs, "scullbuffer");
	if (result < 0) {
		printk(KERN_NOTICE "Unable to get scullbuffer region, error %d\n", result);
		return 0;
	}
	scull_p_devno = firstdev;
  scull_p_device = kmalloc(scull_p_nr_devs * sizeof(struct scull_buffer), GFP_KERNEL);
	if (scull_p_device == NULL) {
		unregister_chrdev_region(firstdev, scull_p_nr_devs);
		return 0;
	}
	memset(scull_p_device, 0, sizeof(struct scull_buffer));

  /*Initialize*/
	scull_p_device->buffer = kmalloc(32 * sizeof(char) * NITEMS , GFP_KERNEL);
  scull_p_device->nwriters = 0;
	scull_p_device->nreaders = 0;
  scull_p_device->buffersize = 32 * sizeof(char) * NITEMS;
  scull_p_device->rp = scull_p_device->buffer;
  scull_p_device->wp = scull_p_device->buffer;
  sema_init(&scull_p_device->sem, scull_p_nr_devs);
  scull_p_setup_cdev(scull_p_device);

	return scull_p_nr_devs;
}

/*
 * This is called by cleanup_module or on failure.
 * It is required to never fail, even if nothing was initialized first
 */
void scull_p_cleanup(void)
{
	if (!scull_p_device)
		return; /* nothing else to release */

  kfree(scull_p_device->buffer);
	kfree(scull_p_device);
	unregister_chrdev_region(scull_p_devno, scull_p_nr_devs);
	scull_p_device = NULL; /* pedantic */
}
