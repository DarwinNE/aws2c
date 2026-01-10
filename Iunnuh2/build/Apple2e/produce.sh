#!/bin/bash


# All names of the tools used for accessing the disk images in the different
# platforms are defined in a single config file in the parent directory:
. ../config.sh

# Assemble the disk image for the italian version of the game
cp dsk/prodos.po ./iunnuh2e_it.po
java -jar $acjarfile -as ./iunnuh2e_it.po IUNNUH2.SYSTEM < iunnuh2e_it.bin
java -jar $acjarfile -p ./iunnuh2e_it.po EM.DRV 0 < dsk/a2e.auxmem.emd
java -jar $acjarfile -p ./iunnuh2e_it.po TEXT.DAT 0 < text_it.dat

# Assemble the disk image for the english version of the game
cp dsk/prodos.po ./iunnuh2e_en.po
java -jar $acjarfile -as ./iunnuh2e_en.po IUNNUH2.SYSTEM < iunnuh2e_en.bin
java -jar $acjarfile -p ./iunnuh2e_en.po EM.DRV 0 < dsk/a2e.auxmem.emd
java -jar $acjarfile -p ./iunnuh2e_en.po TEXT.DAT 0 < text_en.dat

cp ../readme.txt .

rm  AppleII_Iunnuh2.zip
zip -r AppleII_Iunnuh2.zip iunnuh2e_it.po iunnuh2e_en.po readme.txt notes_apple2e.txt
cp AppleII_Iunnuh2.zip $ditdir