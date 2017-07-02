PROGRAM = ryzen_segv_test
CFLAGS = -O2 -Wall -fpie
LDFLAGS = -pie
CC = cc
SRCS = ryzen_segv_test.c
OBJS = $(SRCS:.c=.o)

$(PROGRAM): $(OBJS)
	$(CC) -pthread -o $(PROGRAM) $(LDFLAGS) $(OBJS)

#SUFFIXES: .o .c
.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(PROGRAM) $(OBJS) log.txt

