all: modern c128 c64 vic20 spectrum

pattuglia.c: PattugliaAlba.AWS
	../aws2c -d -c -n PattugliaAlba.AWS pattuglia.c

pattuglia_en.c: PattugliaAlba_en.AWS
	../aws2c -d -c -n PattugliaAlba_en.AWS pattuglia_en.c

pattuglia_no_UTF8.c: PattugliaAlba.AWS
	../aws2c -d -c -m -n -s PattugliaAlba.AWS pattuglia_no_UTF8.c

pattuglia_no_UTF8_en.c: PattugliaAlba.AWS
	../aws2c -d -c -m -n -s PattugliaAlba.AWS pattuglia_no_UTF8.c


pattuglia_en_no_UTF8.c: PattugliaAlba_en.AWS
	../aws2c -d -c -m -n -s PattugliaAlba_en.AWS pattuglia_en_no_UTF8.c

modern: pattuglia.c inout.c pattuglia_en.c
	gcc pattuglia.c inout.c -o pattuglia
	gcc pattuglia_en.c inout.c -o patrol

c128: pattuglia_no_UTF8.c inout.c pattuglia_en_no_UTF8.c
	cl65 -t c128 -D C128 -Or pattuglia_no_UTF8.c inout.c -o pattuglia-128.prg
	cl65 -t c128 -D C128 -Or pattuglia_en_no_UTF8.c inout.c -o patrol-128.prg

c64: pattuglia_no_UTF8.c inout.c pattuglia_en_no_UTF8.c
	cl65 -t c64 -D C64 -Or pattuglia_no_UTF8.c inout.c -o pattuglia-64.prg
	cl65 -t c64 -D C64 -Or pattuglia_en_no_UTF8.c inout.c -o patrol-64.prg

vic20: pattuglia_no_UTF8.c inout.c pattuglia_en_no_UTF8.c
	cl65 -t vic20 -C vic20-16k_exp.cfg -D VIC20 -Or pattuglia_no_UTF8.c inout.c -o pattuglia-VIC+16.prg
	cl65 -t vic20 -C vic20-16k_exp.cfg -D VIC20 -Or pattuglia_en_no_UTF8.c inout.c -o patrol-VIC+16.prg

spectrum: pattuglia_no_UTF8.c pattuglia_en_no_UTF8.c inout.c
	zcc +zx -clib=ansi -lndos -create-app pattuglia_no_UTF8.c inout.c -o ZXpattuglia
	appmake +zx -b ZXpattuglia.tap --audio --fast --dumb
	zcc +zx -clib=ansi -lndos -create-app pattuglia_en_no_UTF8.c inout.c -o ZXpatrol
	appmake +zx -b ZXpatrol.tap --audio --fast --dumb


clean:
	rm pattuglia.c pattuglia_no_UTF8.c pattuglia_en_no_UTF8.c pattuglia_en.c