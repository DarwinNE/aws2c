OBJS= compress.o aws2c.o
CFLAGS= -O2

all: aws2c

aws2c: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

demox: commanderX16.o
	cl65 -t cx16 -o $@ -D DEMOX $?

compress.o: compress.c compress.h

aws2c.o: aws2c.c aws_c.h compress.h

commanderX16.o: commanderX16.c commanderX16.h


clean:
	$(RM) $(OBJS) commanderX16.o

check:
	cd test && sh test.sh
