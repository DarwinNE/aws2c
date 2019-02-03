all: modern c128 c64 vic20 spectrum

dawn_patrol.c: dawn_patrol.aws
	../aws2c -d -n dawn_patrol.aws dawn_patrol.c

dawn_patrol_no_UTF8.c.c: dawn_patrol.aws
	../aws2c -d -c -m -n -s dawn_patrol.aws dawn_patrol_no_UTF8.c.c

modern: dawn_patrol.c inout.c dawn_patrol.c
	gcc dawn_patrol.c inout.c -o patrol

c128: inout.c dawn_patrol_no_UTF8.c.c
	cl65 -t c128 -D C128 -Or dawn_patrol_no_UTF8.c.c inout.c -o patrol-128.prg

c64: inout.c dawn_patrol_no_UTF8.c.c
	cl65 -t c64 -D C64 -Or dawn_patrol_no_UTF8.c.c inout.c -o patrol-64.prg

vic20: inout.c dawn_patrol_no_UTF8.c.c
	cl65 -t vic20 -C vic20-16k_exp.cfg -D VIC20 -Or dawn_patrol_no_UTF8.c.c inout.c -o patrol-VIC+16.prg

spectrum: dawn_patrol_no_UTF8.c.c inout.c
	zcc +zx -clib=ansi -lndos -create-app dawn_patrol_no_UTF8.c.c inout.c -o ZXpatrol
	appmake +zx -b ZXpatrol.tap --audio --fast --dumb


clean:
	rm dawn_patrol_no_UTF8.c.c dawn_patrol.c patrol-128.prg patrol-64.prg patrol-VIC+16.prg ZXpatrol.tap ZXpatrol.wav