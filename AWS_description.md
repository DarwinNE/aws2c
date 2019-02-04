# AWS Format description

By Davide Bucci

## Introduction

The Adventure Writing System (AWS) is a program written by Aristide Torrelli to allow creating text adventures in a pleasant environment. An extensive documentation about it is available on Aristide's website and it is in Italian. Moreover, AWS only runs on Microsoft Windows and it may be cumbersome to use on other OS even if Wine can help in some situations.

I developed a complete text adventure game by working directly on the AWS file with a text editor. I also put together a tool, called `aws2c` that can generate standard C code directly from the AWS file that contains the game.

I thought it would be interesting to briefly document the file format here in English language, so that it may become more accessible worldwide.

## File organization

Currently an AWS file is a UTF-8 encoded text file that is organized in the following way:

1. A header
2. The high priority conditions, started by the `CONDIZIONIHI` tag
3. The low priority conditions, started by the `CONDIZIONILOW` tag
4. The local conditions, started by the `CONDIZIONILOCALI` tag
5. The dictionary, started by `DIZIONARIO`
6. The room descriptions and connections, started by the `LOCAZIONI`tag
7. The various numbered messages used in the game, started by the `MESSAGGI` tag
8. The objects used in the game, started by the `OGGETTI` tag
9. The end-of-file delimiter tag `FINEDATI`

Commands are not case-sensitive, but of course you may be careful about that in your message and everything that is shown to the player.

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

The file header is composed by 19 lines:

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

My `aws2c` plainly ignores all the lines marked with an asterisk, as it aims to produce purely standard C code.

## Structure of high and low priority conditions

A typical logic condition has the following structure:

~~~~
IF VERB 110 AND NOUN 58 THEN MESS 1009 WAIT ENDIF  Look at the bench
~~~~

It always has the form of a rather classic `IF`-command. In this case, this was a low-priority condition in a game. If the player typed the verb 110 and the noun 58, then the message 1009 is shown. Since there is the `WAIT` command at the end, the successive logic conditions will not be evaluated.

Everything that comes after `ENDIF` (that must terminate the condition) can be used as a comment. Here verb 110 means `look` or a synonim and noun 58 is `bench`. Using comments is important.

## Structure of local conditions

Local conditions are identical to high-priority and low-priority conditions, with the difference that they will be evaluated only if you are in a certain room. They are therefore preceded by the room number in the previous line.
For example, let's imagine you are in room 7 and you have this chunk of code in the local conditions of your file:

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

Then, the condition we discussed above should make perfect sense:

~~~~
IF VERB 110 AND NOUN 58 THEN MESS 1009 WAIT ENDIF  Look at the bench
~~~~

Dictionary codes must not be provided in a particular order, but I like to keep them sorted so that I can find rapidly the word if I am searching for a particular code.

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

## Messages

Here things are quite simple: a message is composed by a code and the text of the message itself. For example:

~~~~
16
A warning on the wall says: DO NOT SPIT ON FIRE!
~~~~

defines a message with the code 16 that can be displayed in the game by `MESS 16`

As for room descriptions, you can use `^M` to insert a carriage return in the message. For some reasons, the original AWS substitutes commas with a particular non-UTF-8 code like `ç`. I don't like that, I think it's a legacy of an old version of AWS and I found that using the comma is legitimate at least in the last version of Aristide's program. It is certainly legitimate in `aws2c`, it does not screw the spell checker in your text editor and does not create issues with UTF-8. `aws2c` is able to parse those strange `ç`, anyway.

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
-------|-------------


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
-------|-------------

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

To be described...

## Actions

To be described...

## Functions

To be described...

## Bibliography

Aristide Torrelli has described his Adventure Writing System in detail. Of course, the most important resource is his website and the Italian manuals:

* http://www.aristidetorrelli.it/aws3/AWS.html
* http://www.aristidetorrelli.it/aws3/Manuali/AWS%20-%20Manuale.pdf
* http://www.aristidetorrelli.it/aws3/Manuali/AWS%20-%20Manuale%20Avanzato.pdf
* http://www.aristidetorrelli.it/aws3/Manuali/AWS%20-%20Riferimento%20Linguaggio.pdf
* http://www.aristidetorrelli.it/aws3/Manuali/AWS%20-%20Analisi%20Avventure.pdf