cc65_options = -O -C apple2enh-system.cfg -t apple2enh -D APPLE2E -D NCOL=80 -D CV_IS_A_FUNCTION 
aws_options  = -s -p -m -d -n -w -kk

all: Apple2e

iunnuh_no_utf8.c: ../../aws_sources/Innuh.AWS
	../../../aws2c $(aws_options) -f config_it_nu.h $< $@

iunnuh_it.bin: inout.c iunnuh_no_utf8.c
	cl65  -DCONFIG_FILENAME=\"config_it_nu.h\" --mapfile iunnuh_it.map $(cc65_options) iunnuh_no_utf8.c inout.c loadsave.c resource.c -o iunnuh_it.bin

Apple2e: iunnuh_it.bin 

clean:
	rm iunnuh_no_utf8.c iunnuh_it config_it_nu.h *.o *.dat *.po *.map *.bin mapfile_*.h *.lst
