all: modern c64

iunnuh2.c: Iunnuh2Maledizione.AWS
	../aws2c -d -m Iunnuh2Maledizione.AWS iunnuh2.c

iunnuh2_en.c: Iunnuh2TheCurse.AWS
	../aws2c -d -m Iunnuh2TheCurse.AWS iunnuh2_en.c

iunnuh2_no_utf8.c: Iunnuh2Maledizione.AWS
	../aws2c -c -d -m -s -n -3 Iunnuh2Maledizione.AWS iunnuh2_no_utf8.c

iunnuh2_no_utf8_en.c: Iunnuh2TheCurse.AWS
	../aws2c -c -d -m -s -n -3 Iunnuh2TheCurse.AWS iunnuh2_no_utf8_en.c


modern: iunnuh2.c iunnuh2_en.c inout.c iunnuh2_en.c
	gcc iunnuh2.c inout.c loadsave.c -o iunnuh2_it
	gcc iunnuh2_en.c inout.c loadsave.c -o iunnuh2_en

modern_alt: iunnuh2_no_utf8.c iunnuh2_no_utf8_en.c inout.c iunnuh2_en.c
	gcc iunnuh2_no_utf8.c inout.c loadsave.c -o iunnuh2_it_a
	gcc iunnuh2_no_utf8_en.c inout.c loadsave.c -o iunnuh2_en_a

c64: inout.c iunnuh2_no_utf8.c iunnuh2_no_utf8_en.c
	cl65 -t c64 -C c64_lostack.cfg -D C64 -Or iunnuh2_no_utf8_en.c inout.c loadsave.c  -o iunnuh2-en-64.prg
	cl65 -t c64 -C c64_lostack.cfg -D C64 -Or iunnuh2_no_utf8.c inout.c  loadsave.c -o iunnuh2-it-64.prg


clean:
	rm iunnuh2.c iunnuh2_it iunnuh2_no_utf8.c iunnuh2_en.c iunnuh2_en iunnuh2_it  iunnuh2_no_utf8_en.c iunnuh2_it_a iunnuh2_en_a
