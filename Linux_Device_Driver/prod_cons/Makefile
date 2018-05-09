all: prod_cons
prod_cons:
	gcc -g -Wall -o consumer consumer.c -lm
	gcc -g -Wall -o producer producer.c -lm

clean:
	rm producer
	rm consumer
