all: modern c64

innuh.c: Innuh.AWS
	../aws2c Innuh.AWS innuh.c

innuh_no_utf8.c: Innuh.AWS
	../aws2c -c -d -s -m Innuh.AWS innuh_no_utf8.c

modern: innuh.c inout.c
	gcc innuh.c inout.c -o innuh

c64: innuh_no_utf8.c inout.c
	cl65 -t c64 -D C64 innuh_no_utf8.c inout.c -o innuh-64.prg

clean:
	rm innuh.c innuh_no_utf8.c
