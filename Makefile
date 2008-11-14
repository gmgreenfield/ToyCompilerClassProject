#	%M%(%I%)	%U%	%E%
#
CFLAGS=		# -Wall -g
CC=		gcc
#
SOURCE=		tc.sh toy.y lex.l symbol.c symbol.h Makefile *.ty globals.h
#
all:		toy tc
toy:		toy.tab.o lex.yy.o symbol.o 
		gcc $(CFLAGS) -o $@ $^

lex.yy.c:	lex.l toy.tab.h
		flex lex.l
#
#	Bison options:
#
#	-v	Generate toy.output showing states of LALR parser
#	-d	Generate toy.tab.h containing token type definitions
#
toy.tab.h\
toy.tab.c:	toy.y
		bison -v -l -d $^
#
clean:
		rm -f lex.yy.c toy.tab.[hc] *.o tc toy toy.output
#
tar:
		tar cvf toy.tar $(SOURCE)
source:	
		@echo $(SOURCE)

