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
 * @file   SymbolTable.h
 * @author Thomas Krennwallner <tkren@kr.tuwien.ac.at>
 * @date   Mon Jul 12 08:17:38 2010
 * 
 * @brief  Indexed table of symbols.
 * 
 * @todo Check efficiency and conversions.
 * 
 */


#if !defined(_DLVHEX_SYMBOLTABLE_H)
#define _DLVHEX_SYMBOLTABLE_H

#include "dlvhex/PlatformDefinitions.h"

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/random_access_index.hpp>

DLVHEX_NAMESPACE_BEGIN

/// the SymbolTable is used to get unique IDs for every constant/variable symbol
typedef boost::multi_index_container<
  std::string,
  boost::multi_index::indexed_by<
    boost::multi_index::random_access<>,
    boost::multi_index::hashed_unique<boost::multi_index::identity<std::string> >
    >
  > SymbolTable;

DLVHEX_NAMESPACE_END

#endif /* _DLVHEX_SYMBOLTABLE_H */


// Local Variables:
// mode: C++
// End:
