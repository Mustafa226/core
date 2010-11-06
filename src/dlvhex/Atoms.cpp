/* dlvhex -- Answer-Set Programming with external interfaces.
 * Copyright (C) 2005, 2006, 2007 Roman Schindlauer
 * Copyright (C) 2006, 2007, 2008, 2009, 2010 Thomas Krennwallner
 * Copyright (C) 2009, 2010 Peter Schüller
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
 * @file Atoms.cpp
 * @author Peter Schüller
 *
 * @brief Implementation of Atoms.hpp
 */

#include "dlvhex/Atoms.hpp"
#include "dlvhex/Logger.hpp"

#include <map>

DLVHEX_NAMESPACE_BEGIN

bool OrdinaryAtom::unifiesWith(const OrdinaryAtom& a) const
{
  if( tuple.size() != a.tuple.size() )
    return false;

  LOG_SCOPE("unifiesWith", false);
  // unify from left to right
  Tuple result1(this->tuple);
  Tuple result2(a.tuple);
  // if both tuples have a variable, assign result1 variable to result2 for all occurences to the end
  // if one tuple has constant, assign this constant into the other tuple for all occurences to the end
  Tuple::iterator it1, it2;
  LOG("starting with result1 tuple " << printvector(result1));
  LOG("starting with result2 tuple " << printvector(result2));
  for(it1 = result1.begin(), it2 = result2.begin();
      it1 != result1.end();
      ++it1, ++it2)
  {
    LOG("at position " << static_cast<unsigned>(it1 - result1.begin()) <<
        ": checking " << *it1 << " vs " << *it2);
    if( *it1 != *it2 )
    {
      // different terms
      if( it1->isVariableTerm() )
      {
        // it1 is variable
        if( it2->isVariableTerm() )
        {
          // it2 is variable

          // assign *it1 variable to all occurances of *it2 in result2
          Tuple::iterator it3(it2); it3++;
          for(;it3 != result2.end(); ++it3)
          {
            if( *it3 == *it2 )
              *it3 = *it1;
          }
        }
        else
        {
          // it2 is nonvariable

          // assign *it2 nonvariable to all occurances of *it1 in result1
          Tuple::iterator it3(it1); it3++;
          for(;it3 != result1.end(); ++it3)
          {
            if( *it3 == *it1 )
              *it3 = *it2;
          }
        }
      }
      else
      {
        // it1 is nonvariable
        if( it2->isVariableTerm() )
        {
          // it2 is variable

          // assign *it1 nonvariable to all occurances of *it2 in result2
          Tuple::iterator it3(it2); it3++;
          for(;it3 != result2.end(); ++it3)
          {
            if( *it3 == *it2 )
              *it3 = *it1;
          }
        }
        else
        {
          // it2 is nonvariable
          return false;
        }
      }
      LOG("after propagation of difference (look only after current position!):");
      LOG("result1 tuple " << printvector(result1));
      LOG("result2 tuple " << printvector(result2));
    }
  }
  return true;
}

DLVHEX_NAMESPACE_END


// Local Variables:
// mode: C++
// End: