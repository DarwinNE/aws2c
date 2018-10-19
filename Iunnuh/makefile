all: modern

innuh.c: Innuh.AWS
	../aws2c Innuh.AWS innuh.c

modern: innuh.c inout.c
	gcc innuh.c inout.c -o innuh

c64: innuh.c inout.c
	cl65 -t c64 -D C64 innuh.c inout.c -o innuh.prg

clean:
	rm innuh.c