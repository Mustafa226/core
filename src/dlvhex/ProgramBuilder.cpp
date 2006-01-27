/* -*- C++ -*- */

/**
 * @file   ProgramBuilder.cpp
 * @author Roman Schindlauer
 * @date   Sun Sep 4 12:52:05 2005
 * 
 * @brief  Builders for logic program representations.
 * 
 * 
 */

#include "dlvhex/ProgramBuilder.h"


ProgramBuilder::ProgramBuilder()
{ }

ProgramBuilder::~ProgramBuilder()
{ }


std::string
ProgramBuilder::getString()
{
    return stream.str();
}

void
ProgramBuilder::clearString()
{
    stream.str("");
    stream.clear();
}


ProgramDLVBuilder::ProgramDLVBuilder(bool ho)
  : ProgramBuilder(), higherOrder(ho)
{ }


ProgramDLVBuilder::~ProgramDLVBuilder()
{ }


void
ProgramDLVBuilder::buildRule(const Rule& rule) // throw (???Error)
{
    for (std::vector<Atom>::const_iterator hl = rule.getHead().begin();
         hl != rule.getHead().end();
         hl++)
    {
        if (hl != rule.getHead().begin())
            stream << " v ";
        
        hl->print(stream, higherOrder);
    }

    stream << " :- ";
        
    for (std::vector<Literal>::const_iterator l = rule.getBody().begin();
         l != rule.getBody().end();
         l++)
    {
        if (l != rule.getBody().begin())
            stream << ", ";
        
        l->print(stream, higherOrder);
    }

    stream << "." << std::endl;
}


void
ProgramDLVBuilder::buildFacts(const GAtomSet& facts) // throw (???Error)
{
    for (GAtomSet::const_iterator f = facts.begin();
         f != facts.end();
         f++)
    {
        (*f).print(stream, higherOrder);

        stream << "." << std::endl;
    }
}


void
ProgramDLVBuilder::buildFacts(const Interpretation& I) // throw (???Error)
{
    for (GAtomSet::const_iterator f = I.begin();
         f != I.end();
         f++)
    {
        (*f).print(stream, higherOrder);

        stream << "." << std::endl;
    }
}


void
ProgramDLVBuilder::buildProgram(const Program& program)
{
    const Rules& rules = program.getRules();

    for (Rules::const_iterator r = rules.begin();
         r != rules.end();
         r++)
    {
        buildRule(*r);
    }
}
