cc65_options = -O -C apple2enh-system.cfg -t apple2enh -D APPLE2E -D NCOL=80 -D CV_IS_A_FUNCTION 
output_dir = Apple2e_Iunnuh
aws_options  = -m -c -s -d -n -w -3 -kk -p

ACMD=/Users/davidebucci/Documents/Synchronized/Vintage/Computing/Apple/AppleII/AppleCommander-ac-13.0.jar 

all: Apple2e

iunnuh2_no_utf8.c: ../../aws_sources/Iunnuh2Maledizione.AWS
	../../../aws2c $(aws_options) -R res_it.lst mapfile_it.h -f config_it_nu.h ../../aws_sources/Iunnuh2Maledizione.AWS iunnuh2_no_utf8.c

iunnuh2_no_utf8_en.c: ../../aws_sources/Iunnuh2TheCurse.AWS
	../../../aws2c $(aws_options) -R res_en.lst mapfile_en.h -f config_en_nu.h ../../aws_sources/Iunnuh2TheCurse.AWS iunnuh2_no_utf8_en.c

text_en.dat: iunnuh2_no_utf8_en.c
	python ../../../utils/retro/res_packer.py res_en.lst text_en.dat

text_it.dat: iunnuh2_no_utf8.c
	python ../../../utils/retro/res_packer.py res_it.lst text_it.dat

iunnuh2e_it.bin: inout.c iunnuh2_no_utf8.c resource.c text_it.dat
	cl65  -DCONFIG_FILENAME=\"config_it_nu.h\" --mapfile iunnuh2e_it.map $(cc65_options) iunnuh2_no_utf8.c inout.c loadsave.c resource.c -o iunnuh2e_it.bin

iunnuh2e_en.bin: inout.c iunnuh2_no_utf8_en.c resource.c text_en.dat
	cl65  -DCONFIG_FILENAME=\"config_en_nu.h\" --mapfile iunnuh2e_en.map $(cc65_options) iunnuh2_no_utf8_en.c inout.c loadsave.c resource.c -o iunnuh2e_en.bin
	

Apple2e: iunnuh2e_it.bin iunnuh2e_en.bin

clean:
	rm iunnuh2.c iunnuh2_it iunnuh2_no_utf8.c iunnuh2_en.c iunnuh2_en iunnuh2_it iunnuh2_no_utf8_en.c iunnuh2_it_a iunnuh2_en_a config_it_nu.h config_en_nu.h iunnuh2e iunnuh2i *.o *.dat *.po *.map *.bin mapfile_*.h *.lst
