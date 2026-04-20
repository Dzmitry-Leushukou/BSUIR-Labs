all: parser

lexer.c: lexer.l
	flex -o lexer.c lexer.l

parser.tab.c parser.tab.h: parser.y
	bison -d parser.y

parser: lexer.c parser.tab.c
	gcc -o parser lexer.c parser.tab.c -lfl -lm

clean:
	rm -f lexer.c parser.tab.c parser.tab.h parser