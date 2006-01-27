/* -*- C++ -*- */

/**
 * @file   Literal.cpp
 * @author Roman Schindlauer
 * @date   Sun Sep 4 12:52:05 2005
 * 
 * @brief  Literal class.
 * 
 * 
 */

#include "dlvhex/Literal.h"


Literal::Literal()
{ }


Literal::~Literal()
{
    delete atom;
}


Literal::Literal(const Literal &literal2)
    : isWeaklyNegated(literal2.isWeaklyNegated)
{
    atom = literal2.atom->clone();
}


Literal::Literal(const Atom& at, bool naf)
    : isWeaklyNegated(naf)
{
    atom = new Atom(at);
}


Literal::Literal(const ExternalAtom& at, bool naf)
    : isWeaklyNegated(naf)
{
    atom = new ExternalAtom(at);
}


const Atom*
Literal::getAtom() const
{
    return atom;
}


bool
Literal::isNAF() const
{
    return isWeaklyNegated;
}


bool
Literal::operator== (const Literal& lit2) const
{
    if (!(*atom == *(lit2.getAtom())))
        return 0;

    if (isWeaklyNegated != lit2.isNAF())
        return 0;

    return 1;
}


std::ostream&
Literal::print(std::ostream& stream, const bool ho) const
{
    if (isNAF())
        stream << "not ";
        
    getAtom()->print(stream, ho);
    
    return stream;
}

