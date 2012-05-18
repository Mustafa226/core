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
 * @file   CDNLSolver.hpp
 * @author Christoph Redl <redl@kr.tuwien.ac.at>
 * 
 * @brief  SAT solver based on conflict-driven nogood learning.
 */

#ifndef CDNLSOLVER_HPP_INCLUDED__09122011
#define CDNLSOLVER_HPP_INCLUDED__09122011

#include "dlvhex2/ID.h"
#include "dlvhex2/Interpretation.h"
#include <vector>
#include <set>
#include <map>
#include <boost/foreach.hpp>
#include "dlvhex2/Printhelpers.h"
#include <boost/foreach.hpp>
#include "dlvhex2/Set.h"
#include "dlvhex2/DynamicVector.h"
#include "dlvhex2/Nogood.h"
#include "dlvhex2/SATSolver.h"
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

DLVHEX_NAMESPACE_BEGIN

class CDNLSolver : virtual public NogoodContainer, virtual public SATSolver{
protected:
	struct SimpleHashIDAddress : public std::unary_function<IDAddress, std::size_t> {
		inline std::size_t operator()(IDAddress const& ida) const{
			return ida;
		}
	};
	struct SimpleHashID : public std::unary_function<ID, std::size_t> {
		inline std::size_t operator()(ID const& id) const{
			if (id.isNaf())	return id.address * 2 + 1;
			else		return id.address * 2;
		}
	};

	// instance information
	NogoodSet nogoodset;
	Set<IDAddress> allFacts;
	ProgramCtx& ctx;

	// solver state information
	InterpretationPtr interpretation;
	InterpretationPtr factWasSet;
	DynamicVector<IDAddress, int> decisionlevel;
//	std::vector<int> cause;
//	DynamicVector<IDAddress, int> cause;
	boost::unordered_map<IDAddress, int, SimpleHashIDAddress> cause;
	int currentDL;
	OrderedSet<IDAddress, SimpleHashIDAddress> assignmentOrder;
	DynamicVector<int, std::vector<IDAddress> > factsOnDecisionLevel;

	int exhaustedDL;	// maximum decision level such that the search space above was exhausted
	DynamicVector<int, ID> decisionLiteralOfDecisionLevel;

	// watching data structures for efficient unit propagation
//	std::vector<Set<int> > nogoodsOfPosLiteral;
//	std::vector<Set<int> > nogoodsOfNegLiteral;
//	std::vector<Set<int> > watchingNogoodsOfPosLiteral;
//	std::vector<Set<int> > watchingNogoodsOfNegLiteral;
//	DynamicVector<IDAddress, Set<int> > nogoodsOfPosLiteral;
//	DynamicVector<IDAddress, Set<int> > nogoodsOfNegLiteral;
//	DynamicVector<IDAddress, Set<int> > watchingNogoodsOfPosLiteral;
//	DynamicVector<IDAddress, Set<int> > watchingNogoodsOfNegLiteral;
	boost::unordered_map<IDAddress, Set<int>, SimpleHashIDAddress > nogoodsOfPosLiteral;
	boost::unordered_map<IDAddress, Set<int>, SimpleHashIDAddress > nogoodsOfNegLiteral;
	boost::unordered_map<IDAddress, Set<int>, SimpleHashIDAddress > watchingNogoodsOfPosLiteral;
	boost::unordered_map<IDAddress, Set<int>, SimpleHashIDAddress > watchingNogoodsOfNegLiteral;
	std::vector<Set<ID> > watchedLiteralsOfNogood;
	Set<int> unitNogoods;
	Set<int> contradictoryNogoods;

	// variable selection heuristics
	int conflicts;
//	std::vector<int> varCounterPos;
//	std::vector<int> varCounterNeg;
//	DynamicVector<IDAddress, int> varCounterPos;
//	DynamicVector<IDAddress, int> varCounterNeg;
	boost::unordered_map<IDAddress, int, SimpleHashIDAddress> varCounterPos;
	boost::unordered_map<IDAddress, int, SimpleHashIDAddress> varCounterNeg;
	std::vector<int> recentConflicts;

	// statistics
	long cntAssignments;
	long cntGuesses;
	long cntBacktracks;
	long cntResSteps;
	long cntDetectedConflicts;

	// temporary objects (they are just class members in order to make them reuseable without reallocation)
	Set<ID> tmpWatched;

	// members
	inline bool assigned(IDAddress litadr){
		return factWasSet->getFact(litadr);
	}

	inline bool satisfied(ID lit){
		// fact must have been set
		if (!assigned(lit.address)) return false;
		// truth value must be the same
		return interpretation->getFact(lit.address) == !lit.isNaf();
	}

	inline bool falsified(ID lit){
		// fact must have been set
		if (!assigned(lit.address)) return false;
		// truth value must be negated
		return interpretation->getFact(lit.address) != !lit.isNaf();
	}

	inline ID negation(ID lit){
		return ID(lit.kind ^ ID::NAF_MASK, lit.address);
	}

	inline bool complete(){
		return factWasSet->getStorage().count() == allFacts.size();
	}

	// reasoning members
	bool unitPropagation(Nogood& violatedNogood);
	void analysis(Nogood& violatedNogood, Nogood& learnedNogood, int& backtrackDL);
	Nogood resolve(Nogood& ng1, Nogood& ng2, IDAddress litadr);
	virtual void setFact(ID fact, int dl, int cause);
	virtual void clearFact(IDAddress litadr);
	void backtrack(int dl);
	ID getGuess();
	bool handlePreviousModel();
	void flipDecisionLiteral();

	// members for maintaining the watching data structures
	void initWatchingStructures();
	void updateWatchingStructuresAfterAddNogood(int index);
	void updateWatchingStructuresAfterRemoveNogood(int index);
	void updateWatchingStructuresAfterSetFact(ID lit);
	void updateWatchingStructuresAfterClearFact(ID lit);
	void inactivateNogood(int nogoodNr);
	void stopWatching(int nogoodNr, ID lit);
	void startWatching(int nogoodNr, ID lit);

	// members for variable selection heuristics
	void touchVarsInNogood(Nogood& ng);

	// initialization members
	void initListOfAllFacts();
	virtual void resizeVectors();

	// helper members
	static std::string litToString(ID lit);
	template <typename T>
	inline bool contains(const std::vector<T>& s, T el){
		return std::find(s.begin(), s.end(), el) != s.end();
	}
	template <typename T>
	inline std::vector<T> intersect(const std::vector<T>& a, const std::vector<T>& b){
		std::vector<T> i;
		BOOST_FOREACH (T el, a){
			if (contains(b, el)) i.push_back(el);
		}
		return i;
	}

	long getAssignmentOrderIndex(IDAddress adr){
		if (!assigned(adr)) return -1;
		return assignmentOrder.getInsertionIndex(adr);
	}

public:
	virtual std::string getStatistics();

	CDNLSolver(ProgramCtx& ctx, NogoodSet ns);
	ProgramCtx& getProgramContext();

	virtual InterpretationConstPtr getNextModel();
	virtual int addNogood(Nogood ng);
	void removeNogood(int nogoodIndex);
	Nogood getNogood(int index);
	int getNogoodCount();

	std::vector<Nogood> getContradictoryNogoods();
	inline bool isDecisionLiteral(IDAddress litaddr){
		if (cause[litaddr] == -1){
			return true;
		}else{
			return false;
		}
	}
	Nogood getCause(IDAddress adr);

	typedef boost::shared_ptr<CDNLSolver> Ptr;
	typedef boost::shared_ptr<const CDNLSolver> ConstPtr;
};

typedef CDNLSolver::Ptr CDNLSolverPtr;
typedef CDNLSolver::ConstPtr CDNLSolverConstPtr;

DLVHEX_NAMESPACE_END

#endif