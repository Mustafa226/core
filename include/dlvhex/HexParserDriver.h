/* dlvhex -- Answer-Set Programming with external interfaces.
 * Copyright (C) 2005, 2006, 2007 Roman Schindlauer
 * 
 * This file is part of dlvhex.
 *
 * dlvhex is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * dlvhex is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with dlvhex; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */


/**
 * @file   HexParserDriver.h
 * @author Roman Schindlauer
 * @date   Wed Mar 22 14:39:32 CET 2006
 * 
 * @brief  C++ interface to the bison parser.
 * 
 * 
 */


#if !defined(_DLVHEX_HEXPARSERDRIVER_H)
#define _DLVHEX_HEXPARSERDRIVER_H

#include "dlvhex/PlatformDefinitions.h"

#include "dlvhex/ParserDriver.h"
#include "dlvhex/Program.h"
#include "dlvhex/AtomSet.h"
#include "dlvhex/Error.h"

#include <iosfwd>
#include <string>


// some forward declaration 
class HexFlexLexer;


DLVHEX_NAMESPACE_BEGIN

/**
 * @brief Parses HEX-programs.
 */
class DLVHEX_EXPORT HexParserDriver : public ParserDriver
{
public:
    HexParserDriver();

    virtual
    ~HexParserDriver();

    void
    parse(std::istream& is,
          Program& program,
          AtomSet& EDB) throw (SyntaxError);

    void
    parse(const std::string& filename,
          Program& program,
          AtomSet& EDB) throw (SyntaxError);

    HexFlexLexer*
    getLexer();

    const std::string&
    getInputFilename() const;

    void
    setOrigin(const std::string&);

private:

    /// lexer object which scans the stream
    HexFlexLexer* lexer;

    std::string source;

};

DLVHEX_NAMESPACE_END

#endif // _DLVHEX_HEXPARSERDRIVER_H



// Local Variables:
// mode: C++
// End:
