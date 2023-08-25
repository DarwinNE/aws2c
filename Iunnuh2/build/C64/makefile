all: c64

iunnuh2_no_utf8.c: ../../aws_sources/Iunnuh2Maledizione.AWS
	../../../aws2c -c -d -m -s -n -3 -f config_it_nu.h ../../aws_sources/Iunnuh2Maledizione.AWS iunnuh2_no_utf8.c

iunnuh2_no_utf8_en.c: ../../aws_sources/Iunnuh2TheCurse.AWS
	../../../aws2c -c -d -m -s -n -3 -f config_en_nu.h ../../aws_sources/Iunnuh2TheCurse.AWS iunnuh2_no_utf8_en.c


c64: inout.c iunnuh2_no_utf8.c iunnuh2_no_utf8_en.c
	cl65 -t c64 -DCONFIG_FILENAME=\"config_en_nu.h\" -D C64 -Or iunnuh2_no_utf8_en.c inout.c loadsave.c  -o iunnuh2-en-64.prg
	cl65 -t c64 -DCONFIG_FILENAME=\"config_it_nu.h\" -D C64 -Or iunnuh2_no_utf8.c inout.c  loadsave.c -o iunnuh2-it-64.prg


clean:
	rm iunnuh2.c iunnuh2_it iunnuh2_no_utf8.c iunnuh2_en.c iunnuh2_en iunnuh2_it  iunnuh2_no_utf8_en.c iunnuh2_it_a iunnuh2_en_a config_it_nu.h config_en_nu.h iunnuh2-it-64.prg iunnuh2-en-64.prg
