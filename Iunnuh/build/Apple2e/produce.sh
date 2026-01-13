#!/bin/bash


# All names of the tools used for accessing the disk images in the different
# platforms are defined in a single config file in the parent directory:
. ../config.sh

# Assemble the disk image for the italian version of the game
cp dsk/prodos.po ./iunnuh_it.po
java -jar $acjarfile -as ./iunnuh_it.po IUNNUH.SYSTEM < iunnuh_it.bin

rm  AppleIIe_Iunnuh.zip
zip -r AppleIIe_Iunnuh.zip iunnuh_it.po
cp AppleIIe_Iunnuh.zip $ditdir
