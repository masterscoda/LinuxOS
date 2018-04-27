CC = gcc

CFLAGS  = -pthread

TARGET1 = prod_cons

all: $(TARGET1)

$(TARGET1): $(TARGET1).c
	$(CC) $(CFLAGS) -o $(TARGET1) $(TARGET1).c

clean:
	$(RM) $(TARGET1)
