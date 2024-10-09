CC := gcc

CFLAGS = 

SRCS = anx7447_firmware_download.c 
OBJS = $(SRCS:.c=.o)

TARGET = anx7447_firmware_download

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@


clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
