%{
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "pickle.h"
#include "pickle.tab.h"
%}

%option noyywrap

%%

\+\+\+                         { return DELIM1; }
~~~                            { return DELIM2; }
\-\-\-                         { return DELIM3; }

(int|float|char|string|bool)   { yylval.type = strdup(yytext); return TYPE; }
void                           { yylval.type = strdup(yytext); return VOID; }
const                          { return CONST; }

if                             { return IF; }
elif                           { return ELIF; }
else                           { return ELSE; }
while                          { return WHILE; }
for                            { return FOR; }
let                            { return LET; }
from                           { return FROM; }
to                             { return TO; }
step                           { return STEP; }

break                          { return BREAK; }
continue                       { return CONTINUE; }
return                         { return RETURN; }

and                            { yylval.operator = strdup(yytext); return AND; }
or                             { yylval.operator = strdup(yytext); return OR; }
not                            { yylval.operator = strdup(yytext); return NOT; }
(==|<>)                        { yylval.operator = strdup(yytext); return EQNE; }
(<|<=|>|>=)                    { yylval.operator = strdup(yytext); return LTGT; }
[+-]                           { yylval.operator = strdup(yytext); return ADDT; }
[*\/%]                         { yylval.operator = strdup(yytext); return PROD; }
=                              { yylval.operator = strdup(yytext); return EQ; }
(\+=|-=|\*=|\/=|%=)            { yylval.operator = strdup(yytext); return ASGN; }

(true|false)                   { yylval.boolVal = yytext[0] == 't'; printf("%d\n", yylval.boolVal); return BOOL; }
[_a-zA-Z][_a-zA-Z0-9]*         { yylval.identifier = strdup(yytext); return ID; }
[+-]?(0|[1-9][0-9]*)           { yylval.intVal = atoi(yytext); return INT; }
[+-]?(0|[1-9][0-9]*)?\.[0-9]+  { yylval.floatVal = atof(yytext); return FLT; }
'[^\r\n'\"]'                   { yylval.charVal = yytext[0]; return CHR; }
\"[^\r\n'\"]*\"                { yylval.stringVal = strdup(yytext); return STR; }

[ \t\r\n]                      { }
.                              { return yytext[0]; }

%%
