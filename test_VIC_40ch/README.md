# 40-column driver for the Commodore VIC-20

## Introduction

The Commodore VIC-20 is crippled by a 22-column screen when dealing with
text adventures. AWS2C can use a 40-column driver that allows to work with
40 columns of monochrome text. This driver has been discussed on the Denial
VIC-20 forum: http://sleepingelephant.com/ipw-web/bulletin/bb/viewtopic.php?f=2&t=9257

The screen font is from Mike.

## How to compile and launch the code

If you have Cc65 installed in your machine, and if the make utility, just type
`make`. You will obtain a file named `hello40.prg` that can be loaded in a 
real or emulated VIC-20 with at least the 8KB ram expansion. The code loads at
0x2001 (or decimal 8193) and must be launched with `sys 8205`, unless you 
change BASIC start pointers:

~~~~
lo=1:hi=32

poke 43,lo:poke 44,hi:poke 256*hi+lo-1,0:new

load "hello40.prg",8,1
run
~~~~



## Conclusion

This is an example of a minimalist 40-column driver for the Commodore VIC-20.
