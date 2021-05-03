OBJS= compress.o aws2c.o
CFLAGS= -O2

all: aws2c

aws2c: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

demox: commanderX16.o
	cl65 -t cx16 -o $@ -D DEMOX $?

loadsave.o: loadsave.c config.h

inout.o: inout.c systemd.h aws.h inout.h config.h

compress.o: compress.c compress.h

commanderX16.o: commanderX16.c commanderX16.h

aws2c.o: aws2c.c aws_c.h compress.h

config.h:
	echo >$@


clean:
	$(RM) $(OBJS) loadsave.o inout.o commanderX16.o config.h

check:
	cd test && sh test.sh
