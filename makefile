all: modern

modern: aws2c.c compress.h compress.c
	gcc aws2c.c compress.c -o aws2c

