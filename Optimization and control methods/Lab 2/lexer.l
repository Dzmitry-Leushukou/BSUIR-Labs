%{
#include "parser.tab.h"
#include <string.h>
#include <stdlib.h>
%}

%option noyywrap
%option nounput
%option noinput

%%

\"n\"                return TOKEN_N;
\"m\"                return TOKEN_M;
\"direction\"        return TOKEN_DIRECTION;
\"c\"                return TOKEN_C;
\"A\"                return TOKEN_A;
\"b\"                return TOKEN_B;
\"r\"                return TOKEN_R;
\"sigma\"            return TOKEN_SIGMA;

[ \t\n\r]+          /* ignore whitespace */

-?[0-9]+(\.[0-9]+)? {
    yylval.num = atof(yytext);
    return NUMBER;
}

\"[^\"]*\" {
    yylval.string = strdup(yytext+1);
    yylval.string[strlen(yylval.string)-1] = '\0';
    return STRING;
}

","                 return ',';
":"                 return ':';
"["                 return '[';
"]"                 return ']';
"{"                 return '{';
"}"                 return '}';

.                  { fprintf(stderr, "Unexpected character: %c\n", yytext[0]); }

%%