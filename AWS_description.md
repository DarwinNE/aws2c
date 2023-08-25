# AWS Format description

By Davide Bucci

## Introduction

The Adventure Writing System (AWS) is a program written by Aristide Torrelli to allow creating text adventures in a pleasant environment. An extensive documentation about it is available on Aristide's website and it is in Italian. Moreover, AWS only runs on Microsoft Windows and it may be cumbersome to use on other OS'es, even if Wine helps in some situations.

I developed complete text adventure games by working directly on the AWS file with a text editor. I also put together a tool, called `aws2c` that generates standard C code directly from the AWS file that describes the game.

I thought it would be interesting to briefly document the file format here in English language, so that AWS may become more accessible worldwide.

Aristide reported that AWS was strongly inspired by Graphic Adventure Creator, so you may feel at home if you already used GAC.

## File organization

Currently an AWS file is a UTF-8 encoded text file (Windows-style CR+LF end of line) that is organized in the following way:

1. A header
2. The high priority conditions, started by the `CONDIZIONIHI` tag
3. The low priority conditions, started by the `CONDIZIONILOW` tag
4. The local conditions, started by the `CONDIZIONILOCALI` tag
5. The dictionary, started by `DIZIONARIO`
6. The room descriptions and connections, started by the `LOCAZIONI`tag
7. The various numbered messages used in the game, started by the `MESSAGGI` tag
8. The objects used in the game, started by the `OGGETTI` tag
9. The end-of-file delimiter tag `FINEDATI`

Commands are not case-sensitive, but of course you may be careful about that in your messages and everything that is shown to the player.

Colours are as follows:

0. Black
1. Red
2. Green
3. Yellow
4. Blue
5. Magenta
6. Cyan
7. White

They are not used in `aws2c` for the moment.

## Game cycle and priority of logic conditions

In the game cycle, the conditions represent the logic of the game. When you enter a new location, for example:

* The high priority conditions are executed.
* Then the description of the current room is provided, followed by the objects seen and the directions where the player can move and the prompt.
* Just after the prompt, the local conditions are evaluated, if applicable. Each one of them is associated to a particular room.
* At last, the low priority conditions are executed.
* The cycle is repeated.

There's the possibility of shunting that mechanism by means of the commands `WAIT` and `OKAY` that I'll describe later in greater detail. For example, if a local conditions is executed and ends with `WAIT`, the game cycle repeats immediately and the low priority conditions are not evaluated.

## Header

The file header is always composed of 19 lines:

1. Always the `AWS` tag.
2. Always the `VERSIONE` tag.
3. The version number of `AWS`, such as `3.0`.
4. The text colour *
5. The background colour *
6. The text colour when the player is in a dark area *
7. The background colour when the player is in a dark area *
8. The name of the game.
9. The author of the game.
10. The release date.
11. The description of the game.
12. A numerical code for the game *
13. The font name *
14. The font size *
15. The font style *
16. The room number at the beginning of the game.
17. `TRUE` if the game shows graphical images, `FALSE` otherwise *
18. The maximum total weight of objects that can be carried.
19. The maximum total size of objects that can be carried.

The tool `aws2c` currently ignores all the lines marked with an asterisk.

## Structure of high and low priority conditions

A typical logic condition has the following structure:

~~~~
IF VERB 110 AND NOUN 58 THEN MESS 1009 WAIT ENDIF  Look at the bench
~~~~

It always has the form of a rather classic `IF`-command. In this case, this was a low-priority condition in one of my games. If the player typed the verb 110 and the noun 58, then the message 1009 is shown. Since there is the `WAIT` command at the end, the successive logic conditions will not be evaluated.

Everything that comes after `ENDIF` (that must terminate the condition) can be used as a comment. Here verb 110 means `look` or a synonim and noun 58 is `bench`. Using comments is important to understand what happens.

## Structure of local conditions

Local conditions are identical to high-priority and low-priority conditions, with the difference that they will be evaluated only if you are in a certain room. They are therefore preceded by the room number in the previous line.
For example, let us imagine you are in room 7 and you have this chunk of code in the local conditions of your file:

~~~~
3
IF VERB 70 AND NOUN 7 THEN WAIT ENDIF
7
IF VERB 110 AND NOUN 58 AND ADVE 75 AND RES? 12 THEN MESS 8 BRIN 2 WAIT ENDIF  Look under bench
8
IF VERB 110 AND NOUN 75 THEN MESS 12 WAIT ENDIF
~~~~

In this case, only the following line will be executed:

~~~~
IF VERB 110 AND NOUN 58 AND ADVE 75 AND RES? 12 THEN MESS 8 BRIN 2 WAIT ENDIF  Look under bench
~~~~

## The dictionary

Every word recognized by AWS is described by three lines:

1. The word (in capital letters)
2. The numerical code (I store it in a 16-bit unsigned integer in `aws2c)
3. The type of the word

There are different types of the word available:

* `NOME` indicates a noun
* `VERBO` indicates a verb
* `AVVERBIO` indicates an adverb
* `AGGETTIVO` indicates an adjective
* `ATTORE` indicates the name of a NPC
* `SEPARATORE` is used to separate two phrases

For example, if you have the following definitions in the dictionary section of your file:

~~~~
...
BENCH
58
NOME
LOOK
110
VERBO
...
~~~~

Then, the condition we discussed above should make sense:

~~~~
IF VERB 110 AND NOUN 58 THEN MESS 1009 WAIT ENDIF  Look at the bench
~~~~

Dictionary codes must not be provided in a particular order, but I like to keep them sorted so that I can find rapidly the word if I am searching for a particular code.

Here's an example of a small dictionary that you can use:

~~~~
DIZIONARIO
EXAMINE
70
VERBO
X
70
VERBO
I
72
NOME
INVENTORY
72
NOME
BYE
73
VERBO
SHOW
74
VERBO
GRAB
75
VERBO
TAKE
75
VERBO
LEAVE
76
VERBO
DROP
76
VERBO
ENTER
77
VERBO
N
80
NOME
NORTH
80
NOME
S
81
NOME
SOUTH
81
NOME
E
82
NOME
EAST
82
NOME
W
83
NOME
WEST
83
NOME
U
84
NOME
UP
84
NOME
UPSTAIRS
84
NOME
D
85
NOME
DOWN
85
NOME
DOWNSTAIRS
85
NOME
GO
100
VERBO
L
110
VERBO
LOOK
110
VERBO
A
150
AVVERBIO
RESTART
987
VERBO
SAVE
988
VERBO
LOAD
989
VERBO
THEN
9999
~~~~

## Room description and connections

The code for one room appears as follows:

~~~~
4
Here is the room where horses are shod. Racks and pegs keep all kind of horse riding equipment. Stables are to the west.

HORSE SHOE ROOM
0
0
0
3
0
0
0
0
0
0
~~~~

There are 14 lines, as follows:

1. The number of the room (here we are describing room 4). Code can not be 0.
2. The long description of the room.
3. A short description (that `aws2c` does not use).
4. Title of the room. You can put it in capital letters if you like the effect, but `aws2c` changes colour to display it.
5. Room number toward NORTH (0 if this direction is not available).
6. Room number toward SOUTH (0 if this direction is not available).
7. Room number toward EAST (0 if this direction is not available).
8. Room number toward WEST (0 if this direction is not available).
9. Room number toward UP (0 if this direction is not available).
10. Room number toward DOWN (0 if this direction is not available).
11. Room number toward NE (0 if this direction is not available).
12. Room number toward NW (0 if this direction is not available).
13. Room number toward SE (0 if this direction is not available).
14. Room number toward SW (0 if this direction is not available).

If you want to put a carriage return in the description, you can do that by putting the tag `^M`.

Rooms can not have the following codes:

* Code 0: it is used to indicate a place that does not exist, or a direction that is not available.
* Code 1500: if an object is placed there, it is carried by the player.
* Code 1600: if an object is placed there, it is worn.

## Messages

Here things are quite simple: a message is composed of a code and the text of the message itself. For example:

~~~~
16
A warning on the wall says: DO NOT SPIT ON FIRE!
~~~~

defines a message with the code 16 that can be displayed in the game by `MESS 16`

As for room descriptions, you can use `^M` to insert a carriage return in the message. For some reasons, the original AWS substitutes commas with a particular non-UTF-8 code like `ç`. I don't like that, I think it's a legacy of an old version of AWS and I found that using the comma is legitimate at least in the last version of Aristide's program. It is certainly legitimate in `aws2c`, it does not screw the spell checker in your text editor and does not create issues with UTF-8. `aws2c` is able to parse those strange `ç`, anyway.

Some messages are used by AWS and `aws2c` and have a special meaning:

~~~~
1000
Okay.
1001
Done.
1002
Fone.
1003
It's too heavy!
1004
It's too big to be handled!
1005
It does not move!
1006
I can't see that anywhere!
1007
I don't have that with me!
1008
I can't go in that direction!
1009
I beg your pardon?
1010
I can't
1011
I already have that with me!
1012
What shall I do
1013
It's dark, I can't see anything.
1014
You spent 
1015
 turns to obtain 
1016
 points.
1018
I wear 
1019
I already wear it
1020
I can go towards: 
1021
north
1022
south
1023
east
1024
west
1025
up
1026
down
1027
northeast
1028
northwest
1029
southeast
1030
southwest
1031
I can see: 
1032
I have with me: 
1033
I don't have anything with me.
1050
Place: 
1051
Points: 
1052
Turns: 
1053
I can't see that!
~~~~

## Objects

Every object is described as in this example:

~~~~
1

a torch
0
0
5
FALSE
FALSE
~~~~

where each line means:

1. The code of the object (1 in this example)
2. A short description of the object (not used by `aws2c`)
3. The name of the object
4. The weight of the object (if 0, weight is not important)
5. The size of the object (if 0, size is not important)
6. The room where the object is present at the beginning of the game (0 if it is not directly accessible in a room)
7. `TRUE` if the object can not be carried, `FALSE` otherwise
8. `TRUE` if the object can be worn, `FALSE` otherwise

Some special places are described in the "Room descriptions and connections" section of this document.

## Counters and markers

AWS employs 128 markers that can be `true` or `false, as well as 128 counters (signed, at least 16 bit integers with aws2c).

Some counters have a special meaning:

Number | Description
-------|-------------
118    | Number of objects worn by the player. It is automatically calculated by AWS and should not be changed.
119    | Number of carried objects. It should not be changed.
120    | Total weight carried. It should not be changed.
121    | Maximum size that can be carried
122    | Maximum weight that can be carried
123    | Points earned by the player. It should be updated manually.
124    | Total size carried. It should not be changed.
125    | Game turn counter. It should not be changed.
126    | Decrement automatically at each turn.
127    | Decrement automatically at each turn.


Some markers have a special meaning:

Number | Description
-------|-------------
120    | `true` if the current room has already been described.
121    | `true` if there is a source of light.
122    | `true` if the player has a source of light.
124    | `true` if AWS should print messages 1020 "I can go to" and 1021 to 1030 for the directions
125    | not implemented in `aws2c`
126    | not implemented in `aws2c`
127    | not implemented in `aws2c`

## Grammar for AWS logic conditions

Aristide Torrelli published a handy reference manual in Italian:

http://www.aristidetorrelli.it/aws3/Manuali/AWS%20-%20Riferimento%20Linguaggio.pdf

I report here the grammar of the AWS logic conditions, taked from the reference manual, with some obvious translations:

~~~
phrase ::= IF [decision [logic decision]] THEN {action} ENDIF 

logic ::= AND | OR

decision ::= VERB function | NOUN function | ADVE function | ACTOR
    function | ADJE function | AT function | NOTAT function | SET? function |
    RES? function | EQU? function function | NOTEQU? function function | HERE
    function | NOTHERE function | CARR function | NOTCARR function | AVAI
    function | NOTAVAI function | IN function function | NOTIN function
    function | NO1EQ function | NO1GT function | NO1LT function | NO2EQ
    function | NO2GT function | NO2LT function | ROOMEQ function | ROOMGT
    function | ROOMLT function | VBNOEQ function | VBNOGT function | VBNOLT
    function | ACTOREQ function | ACTORGT function | ACTORLT function | ADJEEQ
    function | ADJEGT function | ADJELT function | ADVEEQ function | ADVEGT
    function | ADVELT function | CTREQ function function | CTRGT function
    function | CTRLT function function | TURNEQ function | TURNGT function |
    TURNLT function | OBJLOCEQ function function | OBJLOCGT function function |
    OBJLOCLT function function | ISWEARING function | ISWEARABLE function |
    ISNOTWEARING function | ISNOTWEARABLE function | CONNEQ function function
    function | CONNGT function function function | CONNLT function function
    function | CONNCORREQ function function | CONNCORRGT function function |
    CONNCORRLT function function | WEIGEQ function function | WEIGGT function
    function | WEIGLT function function | PROB function

function ::= NO1 | NO2 | VBNO | CTR function | TURN | ROOM | CONN
    function function | CONNCORR function | RAND function | OBJLOC function |
    WEIG function | number

number ::= {digit}

action ::= SET function | RESE function | CSET function function | INCR
    function | DECR function | GET function | DROP function | SWAP function
    function | OKAY | WAIT | EXIT | QUIT | HOLD function | GOTO function | DESC
    function | LOOK | LF | SAVE | LOAD | PICT | TEXT | MESS function | PRIN
    function | TO function function | ADDC function function | SUBC function
    function | OBJ function | LIST function | INVE | STRE function | BRIN
    function | FIND function | NORD | SUD | EST | OVEST | ALTO | BASSO |
    NORDEST | NORDOVEST | SUDEST | SUDOVEST | WEAR function | UNWEAR function |
    MESSNOLF function | RESTART | SETCONN function function function | PRINNOLF
    function | PRESSKEY | GETALL | DROPALL |COLOR function function |FCOLO
    function |BCOLO function|RAMSAVE|RAMLOAD|SCRIPTON|SCRIPTOFF
~~~~

## Decisions

Decisions are used in the first part of the condition, after the `IF` command. Therefore, they represent a condition that can be either `true` or `false`.
They can be combined by logical operators `OR` and `AND`. Those logical operators have (much unfortunately) the same priority, a complex expression is evaluated from the left to the right. Trust me, avoid complex logic combining both operators: they can be error-prone and you can't use parenthesis in AWS. To add a certain degree of complexity, `aws2c` sometimes tries to shunt some oft-found combinations of things by exploiting ad-hoc functions. They are worth it, as they greatly optimize the code. In some cases, it's a good idea to have a look to the output C code if you have some doubts.

Decision |aws2c?| Decision becomes `true` if...
---------|------|-------------------------------------------------------------
`VERB v` | X    |  the phrase contains verb `v`
`NOUN n` | X    |  the phrase contains noun `n`
`ADVE a` | X    |  the phrase contains adverb `a`
`ACTOR p`| X    |  the phrase contains actor `a`
`ADJE g` | X    |  the phrase contains adjective `g`
`AT r`   | X    |  player is at room number `r`
`NOTAT r`| X    |  player is not at room number `r`
`SET? m` | X    |  marker `m` is set (`true`)
`RES? m` | X    |  marker `m` is reset (`false`)
`EQU? c x`|X    |  counter `c` is equal to `x`
`NOTEQU? c x`|X |  counter `c` is not equal to `x`
`HERE o` | X    |  object `o` is in the current room
`NOTHERE o`|X   |  object `o` is not in the current room
`CARR o` | X    |  object `o` is carried by the player
`NOTCARR o` | X |  object `o` is not carried by the player
`AVAI o` | X    |  object `o` is available (in the current room or carried)
`NOTAVAI o`| X  |  object `o` is not available
`IN o r` | X    |  object `o` is in the room `r`
`NOTIN o r`| X  |  object `o` is not in the room `r`
`NO1EQ n`| X    |  the first name in the phrase has a code equal to `n`
`NO1GT n`| X    |  the first name in the phrase has a code greater than `n`
`NO1LT n`| X    |  the first name in the phrase has a code less than `n`
`NO2EQ n`| X    |  the second name in the phrase has a code equal to `n`
`NO2GT n`| X    |  the second name in the phrase has a code greater than `n`
`NO2LT n`| X    |  the second name in the phrase has a code less than `n`
`ROOMEQ r`|X    |  the player is in room `r`
`ROOMGT r`|X    |  the player is in a room greater than `r`
`ROOMLT r`|X    |  the player is in room lower than `r`
`VBNOEQ v`|X    |  the verb in the phrase has a code equal to `v`
`VBNOGT v`|X    |  the verb in the phrase has a code greater than `v`
`VBNOLT v`|X    |  the verb in the phrase has a code less than `v`
`ACTOREQ p`|X   |  the phrase contains the actor `p`
`ACTORGT p`|X   |  the phrase contains the actor greater than`p`
`ACTORLT p`|X   |  the phrase contains the actor less than`p`
`ADJEEQ a`|X    |  the phrase contains an adjective equal to `a`
`ADJEGT a`|X    |  the phrase contains an adjective greater than `a`
`ADJELT a`|X    |  the phrase contains an adjective less than `a`
`ADVEEQ a`|X    |  the phrase contains an adverb equal to `a`
`ADVEGT a`|     |  the phrase contains an adverb greater than `a`
`ADVELT a`|     |  the phrase contains an adverb less than than `a`
`CTREQ c x`|X   |  counter `c` is equal to `x`
`CTRGT c x`|X   |  counter `c` is greater than `x`
`CTRLT c x`|X   |  counter `c` is less than `x`
`TURNEQ x`|     |  turn is equal to `x`
`TURNGT x`|     |  turn is greater than `x`
`TURNLT x`|     |  turn is less than `x`
`OBJLOCEQ o x`|X|  the object `o` is in the room `x`
`OBJLOCGT o x`|X|  the object `o` is in the room greater than `x`
`OBJLOCLT o x`|X|  the object `o` is in the room less than `x`
`ISWEARING o` |X|  the player is wearing object `o`
`ISWEARABLE o`|X|  the object `o` is wereable
`ISNOTWEARING o`|X|  the player is not wearing object `o`
`ISNOTWEARABLE o`| |  the object `o` is not wereable
`CONNEQ r d x`|X| there is a connection between rooms `r` and `x` in the direction `d`
`CONNGT r d x`|X| there is a connection between rooms `r` and a room greater than `x` in the direction `d`
`CONNLT r d x`|X| there is a connection between rooms `r` and a room less than `x` in the direction `d`
`CONNCORREQ d x`| | there is a connection between the current room and `x` in the direction `d`
`CONNCORRGT d x`| | there is a connection between the current room and a room greater than `x` in the direction `d`
`CONNCORRLT d x`| | there is a connection between the current room and a room less than `x` in the direction `d`
`WEIGEQ o x`|   | object `o` has a weight `x`
`WEIGGT o x`|   | object `o` has a weight greater than `x`
`WEIGLT o x`|   | object `o` has a weight less than `x`
`ISWEREABLE o`|X| object `o` can be worn
`ISNOTWEREABLE o`|X| object `o` can not be worn
`ISMOVABLE o`| X| true if object `o` can be moved
`ISNOTMOVABLE o`|X| true if object `o` can not be moved
`ISWEARING o` | X | true if the player wears object `o`
`ISNOTWEARING o`|X| true if the player does not wear object `o`
`ISCARRSOME` |X | true if the player carries at least one object
`ISCARRNOTH` |X | true if the player does not carry any object
`ISWEARSOME` |X | true if the player carries at least one object
`ISWEARNOTH` |X | true if the player does not carry any object
`PROB x`  | X   | with a probability `x` (in percent)

NOTE: X in the second column means that this command is implemented in `aws2c`, I means that it is ignored (but a warning message appears).

## Actions

Action   |aws2c?| Description
---------|------|-------------------------------------------------------------
`SET m`  | X    | Set marker `m`
`RESE m` | X    | Reset marker `m`
`CSET c x`|X    | Assign value `x` to counter `c`
`INCR c` | X    | Increment counter `c`
`DECR c` | X    | Decrement counter `c`. No effect if `c` is already zero
`GET o`  | X    | Check if possible (weight, size, presence), get object `o`.
`DROP o` | X    | Check if player carries it and drop object `o`
`SWAP o1 o2`|X  | Swap the place of two objects. They must have same size and same weight
`OKAY`   | X    | Writes `OKAY` and do a `WAIT`. Equivalent to `MESS 1000 WAIT`
`WAIT`   | X    | Skip all other conditions and restart game cycle
`EXIT`   | X    | Exit from game without asking for confirmation
`QUIT`   | X    | Exit from game asking for confirmation
`HOLD x` | X    | Wait for `x` seconds
`GOTO r` | X    | Go to room `r` and describe it
`DESC r` |      | Describe room `r`
`LOOK`   | X    | Describe current room
`LF`     | X    | Print a line feed
`SAVE`   | *    | Save the game
`LOAD`   | *    | Load the game
`PICT`   | I    | Game with pictures (if they are present!)
`TEXT`   | I    | Text-only game
`MESS m` | X    | Print message `m` and a line feed
`MESSNOLF m`|X  | Print message `m` and does not print a line feed
`PRIN x` | X    | Print value `x` and a line feed
`PRINNOLF x`|X  | Print value `x` and does not print a line feed
`TO o r` | X    | Move object `o` to room `r`
`ADDC c x`|X    | Add `x` to counter `c`
`SUBC c x`|     | Subtract `x` to counter `c`. Stop at counter zero
`OBJ o`  | X    | Describe object `o`
`LIST r` |      | List objects in room `r`
`INVE`   | X    | Print inventory
`STRE x` | X    | Set maximum weight that can be carried to `x` (counter 122)
`DIMENS x`|X    | Set maximum dimension that can be carried to `x` (c. 121)
`BRIN o` | X    | Bring object `o` in the current room
`FIND o` |      | Carry player in the room where object `o` is present
`NORD`   | X    | If possible, move to north
`SUD`    | X    | If possible, move to south
`EST`    | X    | If possible, move to east
`OVEST`  | X    | If possible, move to west
`ALTO`   | X    | If possible, move up
`BASSO`  | X    | If possible, move down
`NORDEST`|      | If possible, move to north-east
`NORDOVEST`|    | If possible, move to north-west
`SUDEST` |      | If possible, move to south-east
`SUDOVEST`|     | If possible, move to south-west
`RESTART`| X    | Ask for confirmation and restart game
`SETCONN r1 d r2`|X| Connect room `r1` to room `r2` trough direction `r2`
`WEAR o` | X    | Wear object `o` if carried and it is wearable
`UNWEAR o` |X   | Un-wear object `o` if worn and put it in the inventory
`PRESSKEY`|X    | Wait for a key press
`GETALL` | X    | Get all the object in the room, if possible
`DROPALL`| X    | Drop all the objects carried
`CLS`    |      | Clear screen
`SENDALLROOM r`|X| Move all the carried objects (not worn) in the room `r`
`RAMSAVE`|      | Save the current game in memory
`RAMLOAD`|      | Load the current game from memory
`SCRIPTON`| I   | Save all commands typed in the game in an output .txt file
`SCRIPTOFF`| I  | Stop saving commands typed in a file
`COLOR f b`|    | Set `f` as foreground colour and `b` as background
`FCOLO f` |     | Set `f` as foreground colour
`BCOLO b` |     | Set `b` as background colour
`RESETALL`| X   | Not present in AWS 3.0: restart game without confirmation.

NOTE: X in the second column means that this command is implemented in `aws2c`, I means that it is ignored (but a warning message appears). * means that it is implemented in some platforms.

## Functions

Functions return a numerical value. They can be used as argument of decisions or actions and can be employed recursively, i.e. a function can be an argument of another function.

Function |aws2c?| Value
---------|------|-------------------------------------------------------------
`ACTORNO`| X    | Code of the actor in the phrase (0 not present)
`ADVENO` | X    | Code of the adverb in the phrase (0 not present)
`ADJENO` | X    | Code of the adjective in the phrase (0 not present)
`VBNO`   | X    | Code of the verb in the phrase (0 not present)
`NO1`    | X    | Code of the first noun in the phrase (0 not present)
`NO2`    | X    | Code of the first noun in the phrase (0 not present)
`CTR c`  | X    | Value of the counter `c`
`TURN`   |      | Number of turns since the beginning of the game
`ROOM`   | X    | Current room
`CONN r d`|     | Room number connected to room `r` via direction `d`
`CONNCORR d`|   | Room number connected to current room via direction `d`
`RAND x` |      | A random number between 1 and `x` (integer)
`WEIG o` | X    | Weight of object `o`
`OBJLOC o`|X    | Location of object `o`

NOTE: X in the second column means that this command is implemented in `aws2c`, I means that it is ignored (but a warning message appears).

## An example of a reasonably complete file

Here is an example of a reasonably complete file that does not do anything in particular, but contains two rooms. I translated INITFI40.AWS provided by Aristide, I corrected some obvious errors, removed some conditions that I am going to discuss separately and added two rooms to it.

~~~~
AWS
VERSIONE
3.0
0
7
7
0
Starting file
Aristide Torrelli, edited and translated by D. Bucci
2014
File to be used to start writing a new adventure
1
Bookman Old Style
14
0
1
FALSE
0
0
CONDIZIONIHI
IF at 1 THEN mess 2 ENDIF
CONDIZIONILOW
if verb 73 then quit endif
if verb 74 or verb 0 and noun 72 then inve wait endif
if verb 987 then restart endif
if verb 110 then look wait endif
IF verb 100 OR vbnoeq 0 AND noun 80 THEN nord WAIT ENDIF  Go  NORTH  
IF verb 100 OR vbnoeq 0 AND noun 81 THEN sud WAIT ENDIF  Go  SOUTH
IF verb 100 OR vbnoeq 0 AND noun 82 THEN est WAIT ENDIF  Go  EAST
IF verb 100 OR vbnoeq 0 AND noun 83 THEN ovest WAIT ENDIF  Go WEST
IF verb 100 OR vbnoeq 0 AND noun 84 THEN alto WAIT ENDIF  Go UP
IF verb 100 OR vbnoeq 0 AND noun 85 THEN basso WAIT ENDIF  Go DOWN
CONDIZIONILOCALI
1
IF verb 70 and noun 1 THEN mess 1 WAIT ENDIF Examine the panel
DIZIONARIO
PANEL
1
NOME
EXAMINE
70
VERBO
X
70
VERBO
I
72
NOME
INVENTORY
72
NOME
BYE
73
VERBO
SHOW
74
VERBO
GRAB
75
VERBO
TAKE
75
VERBO
LEAVE
76
VERBO
DROP
76
VERBO
ENTER
77
VERBO
N
80
NOME
NORTH
80
NOME
S
81
NOME
SOUTH
81
NOME
E
82
NOME
EAST
82
NOME
W
83
NOME
WEST
83
NOME
U
84
NOME
UP
84
NOME
UPSTAIRS
84
NOME
D
85
NOME
DOWN
85
NOME
DOWNSTAIRS
85
NOME
GO
100
VERBO
L
110
VERBO
LOOK
110
VERBO
A
150
AVVERBIO
RESTART
987
VERBO
SAVE
988
VERBO
LOAD
989
VERBO
THEN
9999
SEPARATORE
LOCAZIONI
1
You are in room 1. There's a panel here.

ROOM 1
2
0
0
0
0
0
0
0
0
0
2
You are in room 2

ROOM 2
0
1
0
0
0
0
0
0
0
0
MESSAGGI
1
This is an example of a message
2
If you read this message, a high condition has been executed
1000
Okay.
1001
Done.
1002
Fone.
1003
It's too heavy!
1004
It's too big to be handled!
1005
It does not move!
1006
I can't see that anywhere!
1007
I don't have that with me!
1008
I can't go in that direction!
1009
I beg your pardon?
1010
I can't
1011
I already have that with me!
1012
What shall I do
1013
It's dark, I can't see anything.
1014
You spent 
1015
 turns to obtain 
1016
 points.
1018
I wear 
1019
I already wear it
1020
I can go towards: 
1021
north
1022
south
1023
east
1024
west
1025
up
1026
down
1027
northeast
1028
northwest
1029
southeast
1030
southwest
1031
I can see: 
1032
I have with me: 
1033
I don't have anything with me.
1050
Place: 
1051
Points: 
1052
Turns: 
1053
I can't see that!
OGGETTI
FINEDATI
~~~~

## Bibliography

Aristide Torrelli has described his Adventure Writing System in detail. Of course, the most important resource is his website and the Italian manuals:

* http://www.aristidetorrelli.it/aws3/AWS.html
* http://www.aristidetorrelli.it/aws3/Manuali/AWS%20-%20Manuale.pdf
* http://www.aristidetorrelli.it/aws3/Manuali/AWS%20-%20Manuale%20Avanzato.pdf
* http://www.aristidetorrelli.it/aws3/Manuali/AWS%20-%20Riferimento%20Linguaggio.pdf
* http://www.aristidetorrelli.it/aws3/Manuali/AWS%20-%20Analisi%20Avventure.pdf