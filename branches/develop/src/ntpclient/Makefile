include ../EMU2000/config.mk

SRC=$(wildcard ./*.c)
OBJS=$(patsubst %.c, %.o, $(SRC))

CFLAGS+= -c -g -Wall $(INC) -o
OCFLAGS= -o

NAME=ntpclient

all:$(NAME)

$(NAME):$(OBJS)
	$(CC) $^ $(OCFLAGS) $(NAME)
%.o:%.c
	$(CC) $^ $(CFLAGS)  $@

install:
	mkdir -p $(BINARY_PATH)
	cp $(NAME) $(BINARY_PATH)

uninstall:
	rm  $(BINARY_PATH)/$(NAME)

clean:
	rm *.o $(NAME)

.PHONY :all clean install uninstall
