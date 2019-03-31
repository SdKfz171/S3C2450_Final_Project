TOP := $(shell pwd)


#CC		= gcc
#CC 	= /project/toolchain/arm-2010q1/bin/arm-none-linux-gnueabi-gcc
CC 	= arm-unknown-linux-gnueabi-gcc
CFLAGS 	= -I.
#LIBS 	=



TARGET		= multitab_control
TARTGET2	= client
TARGET_OBJ	= main.o
TARGET2_OBJ	= chatting_client.o

PROGS   	:= $(TARGET) 
PROGS2		:= $(TARGET2)

all : $(PROGS) $(PROGS2)


$(TARGET): $(TARGET_OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) $(TARGET_OBJ) -o $@ $(LIBS)

$(TARGET2): $(TARGET2_OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) $(TARGET2_OBJ) -o $@ $(LIBS)

clean:
	rm -f *.o $(PROGS)
	rm -f *.o $(PROGS2)

allclean:
	rm -f *.o $(PROGS)
	rm -f *.o $(PROGS2)

.SUFFIXES: .c .o
