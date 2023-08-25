all: modern

iunnuh2.c: ../../aws_sources/Iunnuh2Maledizione.AWS
	../../../aws2c -d -m -f config_it.h ../../aws_sources/Iunnuh2Maledizione.AWS iunnuh2.c

iunnuh2_en.c: ../../aws_sources/Iunnuh2TheCurse.AWS
	../../../aws2c -d -m -f config_en.h ../../aws_sources/Iunnuh2TheCurse.AWS iunnuh2_en.c

iunnuh2_no_utf8.c: ../../aws_sources/Iunnuh2Maledizione.AWS
	../../../aws2c -c -d -m -s -n -5 -f config_it_nu.h ../../aws_sources/Iunnuh2Maledizione.AWS iunnuh2_no_utf8.c

iunnuh2_no_utf8_en.c: ../../aws_sources/Iunnuh2TheCurse.AWS
	../../../aws2c -c -d -m -s -n -5 -f config_en_nu.h ../../aws_sources/Iunnuh2TheCurse.AWS iunnuh2_no_utf8_en.c


modern: iunnuh2.c iunnuh2_en.c inout.c loadsave.c
	gcc iunnuh2.c inout.c loadsave.c    -DCONFIG_FILENAME=\"config_it.h\" -o iunnuh2_it
	gcc iunnuh2_en.c inout.c loadsave.c -DCONFIG_FILENAME=\"config_en.h\" -o iunnuh2_en

modern_alt: iunnuh2_no_utf8.c iunnuh2_no_utf8_en.c inout.c loadsave.c
	gcc iunnuh2_no_utf8.c inout.c loadsave.c    -DCONFIG_FILENAME=\"config_it_nu.h\" -o iunnuh2_it_a
	gcc iunnuh2_no_utf8_en.c inout.c loadsave.c -DCONFIG_FILENAME=\"config_en_nu.h\" -o iunnuh2_en_a


clean:
	rm iunnuh2.c iunnuh2_it iunnuh2_no_utf8.c iunnuh2_en.c iunnuh2_en iunnuh2_it  iunnuh2_no_utf8_en.c iunnuh2_it_a iunnuh2_en_a config_en.h config_it.h config_it_nu.h config_en_nu.h
