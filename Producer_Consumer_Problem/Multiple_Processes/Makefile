CC = gcc

CFLAGS  = -pthread

TARGET1 = main
TARGET2 = consumer
TARGET3 = producer

all: $(TARGET1) $(TARGET2) $(TARGET3)

$(TARGET1): $(TARGET1).c
	$(CC) $(CFLAGS) -o $(TARGET1) $(TARGET1).c

$(TARGET2): $(TARGET2).c
	$(CC) $(CFLAGS) -o $(TARGET2) $(TARGET2).c

$(TARGET3): $(TARGET3).c
	$(CC) $(CFLAGS) -o $(TARGET3) $(TARGET3).c

clean:
	$(RM) $(TARGET1) $(TARGET2) $(TARGET3)
