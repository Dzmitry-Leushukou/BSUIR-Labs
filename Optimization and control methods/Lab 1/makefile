CC = gcc
CFLAGS = -Wall -g
LEX = flex
YACC = bison -d

all: matrix_inv

matrix_inv: lex.yy.c parser.tab.c
	$(CC) $(CFLAGS) -o $@ lex.yy.c parser.tab.c -lm

lex.yy.c: lexer.l parser.tab.h
	$(LEX) lexer.l

parser.tab.c parser.tab.h: parser.y
	$(YACC) -v parser.y

clean:
	rm -f lex.yy.c parser.tab.c parser.tab.h parser.output matrix_inv

run: matrix_inv
	./matrix_inv input.json

.PHONY: all clean run