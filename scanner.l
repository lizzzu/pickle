%{
    #include "scanner.h"
    #include "parser.hpp"
    #include "interpreter.h"
    using namespace std;
    using namespace Pickle;
%}

%option c++
%option noyywrap
%option nodefault
%option yyclass = "Scanner"
%option prefix = "Pickle_"

%%

\+\+\+                             { return Parser::make_DELIM1(); }
~~~                                { return Parser::make_DELIM2(); }
\-\-\-                             { return Parser::make_DELIM3(); }

(int|float|char|string|bool|void)  { return Parser::make_TYPE(yytext); }
const                              { return Parser::make_CONST(); }

if                                 { return Parser::make_IF(); }
elif                               { return Parser::make_ELIF(); }
else                               { return Parser::make_ELSE(); }
while                              { return Parser::make_WHILE(); }
for                                { return Parser::make_FOR(); }
let                                { return Parser::make_LET(); }
from                               { return Parser::make_FROM(); }
to                                 { return Parser::make_TO(); }
step                               { return Parser::make_STEP(); }

break                              { return Parser::make_BREAK(); }
continue                           { return Parser::make_CONTINUE(); }
return                             { return Parser::make_RETURN(); }

and                                { return Parser::make_AND(yytext); }
or                                 { return Parser::make_OR(yytext); }
not                                { return Parser::make_NOT(yytext); }
(==|<>)                            { return Parser::make_EQNE(yytext); }
(<|<=|>|>=)                        { return Parser::make_LTGT(yytext); }
[+-]                               { return Parser::make_ADDT(yytext); }
[*\/%]                             { return Parser::make_PROD(yytext); }
=                                  { return Parser::make_EQ(yytext); }
(\+=|-=|\*=|\/=|%=)                { return Parser::make_ASGN(yytext); }

(true|false)                       { return Parser::make_BOOL(yytext[0] == 't'); }
[_a-zA-Z][_a-zA-Z0-9]*             { return Parser::make_ID(yytext); }
[+-]?(0|[1-9][0-9]*)               { return Parser::make_INT(atoi(yytext)); }
[+-]?(0|[1-9][0-9]*)?\.[0-9]+      { return Parser::make_FLOAT(atof(yytext)); }
'[^\r\n'\"]'                       { return Parser::make_CHAR(yytext[0]); }
\"[^\r\n'\"]*\"                    { return Parser::make_STRING(yytext); }

[ \t\r\n]                          { }
.                                  { return yytext[0]; }

%%
