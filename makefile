all: modern

modern: aws2c.c
	gcc aws2c.c -o aws2c

game: game.c inout.c
	gcc game.c inout.c -o game