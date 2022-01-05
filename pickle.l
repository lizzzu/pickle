%{
#include <stdio.h>
#include "pickle.tab.h"
%}

%option noyywrap

%%

\+\+\+                         { ECHO; printf("\n"); return DELIM1; }
~~~                            { ECHO; printf("\n"); return DELIM2; }
\-\-\-                         { ECHO; printf("\n"); return DELIM3; }

(int|float|char|string|bool)   { ECHO; printf("\n"); return TYPE; }
void                           { ECHO; printf("\n"); return VOID; }
const                          { ECHO; printf("\n"); return CONST; }

if                             { ECHO; printf("\n"); return IF; }
elif                           { ECHO; printf("\n"); return ELIF; }
else                           { ECHO; printf("\n"); return ELSE; }
while                          { ECHO; printf("\n"); return WHILE; }
for                            { ECHO; printf("\n"); return FOR; }
let                            { ECHO; printf("\n"); return LET; }
from                           { ECHO; printf("\n"); return FROM; }
to                             { ECHO; printf("\n"); return TO; }
step                           { ECHO; printf("\n"); return STEP; }

break                          { ECHO; printf("\n"); return BREAK; }
continue                       { ECHO; printf("\n"); return CONTINUE; }
return                         { ECHO; printf("\n"); return RETURN; }

and                            { ECHO; printf("\n"); return AND; }
or                             { ECHO; printf("\n"); return OR; }
not                            { ECHO; printf("\n"); return NOT; }
(==|<>)                        { ECHO; printf("\n"); return EQNE; }
(<|<=|>|>=)                    { ECHO; printf("\n"); return LTGT; }
[+-]                           { ECHO; printf("\n"); return ADDT; }
[*\/%]                         { ECHO; printf("\n"); return PROD; }
=                              { ECHO; printf("\n"); return EQ; }
(\+=|-=|\*=|\/=|%=)            { ECHO; printf("\n"); return ASGN; }

(true|false)                   { ECHO; printf("\n"); return BOOL; }
[_a-zA-Z][_a-zA-Z0-9]*         { ECHO; printf("\n"); return ID; }
[+-]?(0|[1-9][0-9]*)           { ECHO; printf("\n"); return INT; }
[+-]?(0|[1-9][0-9]*)?\.[0-9]+  { ECHO; printf("\n"); return FLT; }
'[^\r\n'\"]'                   { ECHO; printf("\n"); return CHR; }
\"[^\r\n'\"]*\"                { ECHO; printf("\n"); return STR; }

[ \t\r\n]                      { }
.                              { return yytext[0]; }

%%
