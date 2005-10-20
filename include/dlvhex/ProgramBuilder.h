/* -*- C++ -*- */

/**
 * @file   ProgramBuilder.h
 * @author Roman Schindlauer
 * @date   Sun Sep 4 12:39:40 2005
 * 
 * @brief  Builders for logic program representations.
 * 
 * 
 */

#ifndef _PROGRAMBUILDER_H
#define _PROGRAMBUILDER_H

#include <string>
#include <sstream>

#include "dlvhex/Rule.h"

/**
* @brief Base Builder for building logic programs.
*/
class ProgramBuilder
{
protected:
    std::ostringstream stream;

    /// Ctor
    ProgramBuilder();

    /// Dtor
    virtual
    ~ProgramBuilder();

public:
    /**
     * Building method implemented by the children of ProgramBuilder.
     *
     * @param rule
     */
    virtual void
    buildRule(const Rule &rule) = 0;

    std::string
    getString();

    void
    clearString();

};


/**
* @brief A Builder for programs to be evaluated with DLV.
*/
class ProgramDLVBuilder : public ProgramBuilder
{
public:
    /// Ctor
    ProgramDLVBuilder(bool ho);

    /// Dtor
    virtual
    ~ProgramDLVBuilder();

    /**
     * @brief Build rule for DLV.
     */
    virtual void
    buildRule(const Rule &);

    /**
     * @brief Build facts for DLV.
     */
    virtual void
    buildFacts(const GAtomSet &);

    /**
     * @brief Build facts for DLV from an interpretation.
     */
    virtual void
    buildFacts(const Interpretation &);

private:
    /**
     * @brief Flag to indicate if program should be build in higher-order notation.
     */
    bool
    higherOrder;
};


#endif /* _PROGRAMBUILDER_H */
