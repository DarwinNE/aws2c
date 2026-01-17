all: modern

innuh.c: ../../aws_sources/Innuh.AWS
	../../../aws2c $< $@

innuh_no_utf8.c: ../../aws_sources/Innuh.AWS
	../../../aws2c -c -d -s -m $< $@

modern: innuh.c inout.c loadsave.c
	gcc innuh.c inout.c loadsave.c -o iunnuh

modern_alt: innuh_no_utf8.c inout.c loadsave.c
	gcc innuh_no_utf8.c inout.c loadsave.c -o iunnuh_a

c64: innuh_no_utf8.c inout.c loadsave.c
	cl65 -t c64 -D C64 innuh_no_utf8.c inout.c loadsave.c -o innuh-64.prg

vic-20: innuh_no_utf8.c inout.c loadsave.c
	cl65 -t vic20 -C vic20-16k_exp.cfg -D VIC20 -T -Os -r innuh_no_utf8.c inout.c -o innuh-vic20.prg

clean:
	rm innuh.c innuh_no_utf8.c *.o
