PROGRAM = ryzen_segv_test
CFLAGS = -O2 -Wall
CC = cc
AS = as
SRC = ryzen_segv_test.c
OBJ = $(SRC:.c=.o)
ASM = $(SRC:.c=.s)

$(PROGRAM): $(OBJ)
	$(CC) -pthread -o $(PROGRAM) $(OBJ)

%.o: %.c

#SUFFIXES: .o .s
.s.o:
	$(AS) -o $@ $<

.PHONY: clean
clean:
	rm -f $(PROGRAM) $(OBJ) log.txt

.PHONY: asm
asm: $(SRC)
	$(CC) $(CFLAGS) -S $(SRC)
