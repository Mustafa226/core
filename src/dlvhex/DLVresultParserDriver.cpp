/* -*- C++ -*- */

/**
 * @file   DLVresultParserDriver.cpp
 * @author Roman Schindlauer
 * @date   Wed Mar 22 14:38:53 CET 2006
 * 
 * @brief  C++ interface to the bison parser.
 * 
 * 
 */

#include <sstream>
#include <iostream>

#include "dlvhex/DLVresultParser.hpp"
#include "dlvhex/ParserDriver.h"
#include "dlvhex/DLVresultFlexLexer.h"

DLVresultParserDriver::DLVresultParserDriver()
    : lexer(new DLVresultFlexLexer(this))
{
}



DLVresultParserDriver::~DLVresultParserDriver()
{
    delete lexer;
}


DLVresultFlexLexer*
DLVresultParserDriver::getLexer()
{
    return lexer;
}



void
DLVresultParserDriver::parse(std::istream& is,
                             std::vector<AtomSet>& result,
                             std::string& dlvError) throw (SyntaxError)
{
    yy::DLVresultParser parser(this, result, dlvError);
    parser.set_debug_level(false);
    lexer->switch_streams(&is, &std::cerr);
    parser.parse();
}
