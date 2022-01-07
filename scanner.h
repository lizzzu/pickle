#ifndef SCANNER_H
#define SCANNER_H

#if !defined(yyFlexLexerOnce)
#undef yyFlexLexer
#define yyFlexLexer Pickle_FlexLexer
#include <FlexLexer.h>
#endif

#undef YY_DECL
#define YY_DECL Pickle::Parser::symbol_type Pickle::Scanner::get_next_token()

#include <bits/stdc++.h>
#include "parser.hpp"

namespace Pickle {
    class Interpreter;

    class Scanner : public yyFlexLexer {
        Interpreter &driver;
    public:
        Scanner(Interpreter& driver) : driver(driver) { }
        virtual ~Scanner() { }
        virtual Pickle::Parser::symbol_type get_next_token();
    };
}

#endif
