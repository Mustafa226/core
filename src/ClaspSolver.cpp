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
 * @file ClaspSolver.cpp
 * @author Christoph Redl
 *
 * @brief Interface to genuine clasp 2.0.5-based Solver.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_LIBCLASP

#include "dlvhex2/ClaspSolver.h"

#include <iostream>
#include <sstream>
#include "dlvhex2/Logger.h"
#include "dlvhex2/GenuineSolver.h"
#include "dlvhex2/Printer.h"
#include <boost/foreach.hpp>
#include <boost/graph/strong_components.hpp>

#include "clasp/program_rule.h"
#include "clasp/constraint.h"


DLVHEX_NAMESPACE_BEGIN

/*

// callback interface for printing answer sets
struct AspOutPrinter : public Clasp::Enumerator::Report {
	void reportModel(const Clasp::Solver& s, const Clasp::Enumerator&);
	void reportSolution(const Clasp::Solver& s, const Clasp::Enumerator&, bool);
};


// Compute the stable models of the program
//    a :- not b.
//    b :- not a.
void example1() {
	// The ProgramBuilder lets you define logic programs.
	// It preprocesses logic programs and converts them
	// to the internal solver format.
	// See program_builder.h for details
	Clasp::ProgramBuilder api;
	
	// Among other things, SharedContext maintains a Solver object 
	// which hosts the data and functions for CDNL answer set solving.
	// SharedContext also contains the symbol table which stores the 
	// mapping between atoms of the logic program and the 
	// propositional literals in the solver.
	// See solver.h and solver_types.h for details
	Clasp::SharedContext  ctx;
	
	// startProgram must be called once before we can add atoms/rules
	api.startProgram(ctx);
	
	// Populate symbol table. Each atoms must have a unique id, the name is optional.
	// The symbol table then maps the ids to the propositional 
	// literals in the solver.
	api.setAtomName(1, "a");
	api.setAtomName(2, "b");
	api.setAtomName(3, "c");
	api.setAtomName(4, "d");
	
	// Define the rules of the program
	api.startRule(Clasp::BASICRULE).addHead(1).addToBody(2, false).addToBody(3, false).addToBody(4, false).endRule();
	api.startRule(Clasp::BASICRULE).addHead(2).addToBody(1, false).addToBody(3, false).addToBody(4, false).endRule();
	api.startRule(Clasp::BASICRULE).addHead(3).addToBody(1, false).addToBody(2, false).addToBody(4, false).endRule();
	api.startRule(Clasp::BASICRULE).addHead(4).addToBody(1, false).addToBody(2, false).addToBody(3, false).endRule();
	api.startRule(Clasp::BASICRULE).addHead(1).addToBody(2, true).endRule();
	api.startRule(Clasp::BASICRULE).addHead(2).addToBody(1, true).endRule();
	api.startRule(Clasp::BASICRULE).addHead(1).addToBody(3, true).endRule();
	
	// Once all rules are defined, call endProgram() to load the (simplified)
	// program into the context object
	api.endProgram();
	if (api.dependencyGraph() && api.dependencyGraph()->nodes() > 0) {
		// program is non tight - add unfounded set checker
		Clasp::DefaultUnfoundedCheck* ufs = new Clasp::DefaultUnfoundedCheck();
		ufs->attachTo(*ctx.master(), api.dependencyGraph()); // register with solver and graph & transfer ownership
	}
	
	// For printing answer sets
	AspOutPrinter         printer;

	// Since we want to compute more than one
	// answer set, we need an enumerator.
	// See enumerator.h for details
	ctx.addEnumerator(new Clasp::BacktrackEnumerator(0,&printer));
	ctx.enumerator()->enumerate(0);

	// endInit() must be called once before the search starts
	ctx.endInit();

	// Aggregates some solving parameters, e.g.
	//  - restart-strategy
	//  - ...
	// See solve_algorithms.h for details
	Clasp::SolveParams    params;

	// solve() starts the search for answer sets. It uses the 
	// given parameters to control the search.
	Clasp::solve(ctx, params);
}

void AspOutPrinter::reportModel(const Clasp::Solver& s, const Clasp::Enumerator& e) {
	std::cout << "Model " << e.enumerated << ": \n";
	// get the symbol table from the solver
	const Clasp::SymbolTable& symTab = s.sharedContext()->symTab();
	for (Clasp::SymbolTable::const_iterator it = symTab.begin(); it != symTab.end(); ++it) {
		// print each named atom that is true w.r.t the current assignment
		if (s.isTrue(it->second.lit) && !it->second.name.empty()) {
			std::cout << it->second.name.c_str() << " ";
		}
	}
	std::cout << std::endl;
}
void AspOutPrinter::reportSolution(const Clasp::Solver&, const Clasp::Enumerator&, bool complete) {
	if (complete) std::cout << "No more models!" << std::endl;
	else          std::cout << "More models possible!" << std::endl;
}
*/

void ClaspSolver::ModelEnumerator::reportModel(const Clasp::Solver& s, const Clasp::Enumerator&){

	DBGLOG(DBG, "ClaspThread: Start producing a model");

	// create a model
	InterpretationPtr model = InterpretationPtr(new Interpretation(cs.reg));

	// get the symbol table from the solver
	const Clasp::SymbolTable& symTab = s.sharedContext()->symTab();
	for (Clasp::SymbolTable::const_iterator it = symTab.begin(); it != symTab.end(); ++it) {
		// translate each named atom that is true w.r.t the current assignment into our dlvhex ID
		if (s.isTrue(it->second.lit) && !it->second.name.empty()) {
			IDAddress adr = ClaspSolver::stringToIDAddress(it->second.name.c_str());
			// set it in the model
			model->setFact(adr);
		}
	}

	// remember the model
	DBGLOG(DBG, "Model is: " << *model);
	cs.nextModel = model;
	cs.modelRequest = false;
	DBGLOG(DBG, "ClaspThread: Notifying MainThread");
	cs.sem_answer.post();	// continue with execution of main thread

	DBGLOG(DBG, "ClaspThread: Waiting for further requests");
	cs.sem_request.wait();

	// @TODO: find a breakout possibility to terminate only the current thread!
	//        the following throw kills the whole application, which is not what we want
	if (cs.terminationRequest) throw std::runtime_error("ClaspThread was requested to terminate");
}

void ClaspSolver::ModelEnumerator::reportSolution(const Clasp::Solver& s, const Clasp::Enumerator&, bool complete){
}

bool ClaspSolver::ExternalPropagator::propagate(Clasp::Solver& s){

/*
	Nogood ng;
	ng.insert(ID(ID::MAINKIND_LITERAL | ID::SUBKIND_ATOM_ORDINARYG, 0));
	ng.insert(ID(ID::MAINKIND_LITERAL | ID::SUBKIND_ATOM_ORDINARYG, 2));
	cs.addNogoodToClasp(ng);
*/

	if (cs.learner.size() != 0){

		DBGLOG(DBG, "Translating clasp assignment to HEX-interpretation");
		// create an interpretation and a bitset of assigned values
		InterpretationPtr interpretation = InterpretationPtr(new Interpretation(cs.reg));
		InterpretationPtr factWasSet = InterpretationPtr(new Interpretation(cs.reg));

		// translate clasp assignment to hex assignment
		// get the symbol table from the solver
		const Clasp::SymbolTable& symTab = s.sharedContext()->symTab();
		for (Clasp::SymbolTable::const_iterator it = symTab.begin(); it != symTab.end(); ++it) {
			// bitset of all assigned values
			if (s.isTrue(it->second.lit) || s.isFalse(it->second.lit)) {
				IDAddress adr = ClaspSolver::stringToIDAddress(it->second.name.c_str());
				factWasSet->setFact(adr);
			}
			// bitset of true values (partial interpretation)
			if (s.isTrue(it->second.lit)) {
				IDAddress adr = ClaspSolver::stringToIDAddress(it->second.name.c_str());
				interpretation->setFact(adr);
			}
		}

		DBGLOG(DBG, "Calling external learners with interpretation: " << *interpretation);
		bool learned = false;
		BOOST_FOREACH (LearningCallback* cb, cs.learner){
			// we are currently not able to check what changed inside clasp, so assume that all facts changed
			learned |= cb->learn(interpretation, factWasSet->getStorage(), factWasSet->getStorage());
		}
	}

	// add the produced nogoods to clasp
	bool inconsistent = false;
	DBGLOG(DBG, "External learners have produced " << cs.nogoods.size() << " nogoods; transferring to clasp");
	for (int i = cs.translatedNogoods; i < cs.nogoods.size(); ++i){
		inconsistent |= cs.addNogoodToClasp(cs.nogoods[i]);
	}
	cs.translatedNogoods = cs.nogoods.size();
	DBGLOG(DBG, "Result: " << (inconsistent ? "" : "not ") << "inconsistent");

	return !inconsistent;
}

bool ClaspSolver::addNogoodToClasp(Nogood& ng){

#ifndef NDEBUG
	std::stringstream ss;
	ss << "{ ";
	bool first = true;
#endif
	// only nogoods are relevant where all variables occur in this clasp instance
	BOOST_FOREACH (ID lit, ng){
		if (hexToClasp.find(lit.address) == hexToClasp.end()) return false;
	}

	// translate dlvhex::Nogood to clasp clause
	clauseCreator->start(Clasp::Constraint_t::learnt_other);
	BOOST_FOREACH (ID lit, ng){
		// 1. cs.hexToClasp maps hex-atoms to clasp-literals
		// 2. the sign must be changed if the hex-atom was default-negated (xor ^)
		// 3. the overall sign must be changed (negation !) because we work with nogoods and clasp works with clauses
		Clasp::Literal clit = Clasp::Literal(hexToClasp[lit.address].var(), !(hexToClasp[lit.address].sign() ^ lit.isNaf()));
		clauseCreator->add(clit);
#ifndef NDEBUG
		if (!first) ss << ", ";
		first = false;
		ss << (clit.sign() ? "" : "!") << clit.var();
#endif
	}

#ifndef NDEBUG
	ss << " }";
#endif
	clauseCreator->end();

	//std::cout << claspInstance.numTernary() << ", " << claspInstance.numBinary() << ", " << claspInstance.numLearntShort() << std::endl;
	/*
	if (claspInstance.master()->numLearntConstraints() > 0){
		Clasp::LitVec lv;
		Clasp::LearntConstraint& lc = claspInstance.master()->getLearnt(claspInstance.master()->numLearntConstraints() - 1);

		std::vector<std::vector<ID> > ngg = convertClaspNogood(lc);

		BOOST_FOREACH (std::vector<ID> ng, ngg){
			std::cout << "{";
			BOOST_FOREACH (ID id, ng){
				std::cout << (id.isNaf() ? "-" : "") << id.address << ", ";
			}
			std::cout << "} x ";
		}
		std::cout << std::endl;

		std::vector<Nogood> nogoods = convertClaspNogood(ngg);
		BOOST_FOREACH (Nogood nogood, nogoods){
			std::cout << "NG: " << nogood << std::endl;
		}
	}
	*/

	DBGLOG(DBG, "Adding nogood " << ng << " as clasp-clause " << ss.str());

	return false;
}

std::vector<std::vector<ID> > ClaspSolver::convertClaspNogood(Clasp::LearntConstraint& learnedConstraint){

	if (learnedConstraint.clause()){
		Clasp::LitVec lv;
		learnedConstraint.clause()->toLits(lv);
		return convertClaspNogood(lv);
	}
}

std::vector<std::vector<ID> > ClaspSolver::convertClaspNogood(const Clasp::LitVec& litvec){

	// A clasp literal possibly maps to multiple dlvhex literals
	// (due to optimization, equivalent and antivalent variables are represented by the same clasp literal).
	// Therefore a clasp clause can represent several dlvhex nogoods.
	// The result of this function is a vector of vectors of IDs.
	// The outer vector has one element for each clasp literal. The inner vector enumerates all possible back-translations of the clasp literal to dlvhex.

	std::vector<std::vector<ID> > ret;

	BOOST_FOREACH (Clasp::Literal l, litvec){
		// create for each clasp literal a vector of all possible back-translations to dlvhex
		std::vector<ID> translations;
		for (int i = 0; i < claspToHex[l].size(); ++i) translations.push_back(ID(ID::MAINKIND_LITERAL | ID::SUBKIND_ATOM_ORDINARYG | ID::NAF_MASK, claspToHex[l][i]));
		Clasp::Literal ln(l.var(), !l.sign());
		for (int i = 0; i < claspToHex[ln].size(); ++i) translations.push_back(ID(ID::MAINKIND_LITERAL | ID::SUBKIND_ATOM_ORDINARYG | ID::NAF_MASK, claspToHex[ln][i]));
		ret.push_back(translations);
	}
	return ret;
}

std::vector<Nogood> ClaspSolver::convertClaspNogood(std::vector<std::vector<ID> >& nogoods){

	// The method "unfolds" a set of nogoods, represented as
	// { l1[1], ..., l1[n1] } x { l2[1], ..., l2[n2] } x ... x { lk[1], ..., lk[nk] }
	// into a set of nogoods

	std::vector<Nogood> ret;
	if (nogoods.size() == 0) return ret;

	std::vector<int> ind(nogoods.size());
	for (int i = 0; i < nogoods.size(); ++i) ind[i] = 0;
	while (true){
		if (ind[0] >= nogoods[0].size()) break;

		// translate
		Nogood ng;
		for (int i = 0; i < nogoods.size(); ++i){
			ng.insert(nogoods[i][ind[i]]);
		}
		ret.push_back(ng);

		int k = nogoods.size() - 1;
		ind[k]++;
		while (ind[k] >= nogoods[k].size()){
			ind[k] = 0;
			k--;
			if (k < 0) break;
			ind[k]++;
			if (ind[0] >= nogoods[0].size()) break;
		}
	}

	return ret;
}

void ClaspSolver::buildInitialSymbolTable(OrdinaryASPProgram& p, Clasp::ProgramBuilder& pb){
//	pb.newAtom();

	DBGLOG(DBG, "Building atom index");
	// edb
	bm::bvector<>::enumerator en = p.edb->getStorage().first();
	bm::bvector<>::enumerator en_end = p.edb->getStorage().end();
	while (en < en_end){
		if (hexToClasp.find(*en) == hexToClasp.end()){
			uint32_t c = *en + 2; // pb.newAtom();
			DBGLOG(DBG, "Clasp index of atom " << *en << " is " << c);
			hexToClasp[*en] = Clasp::Literal(c, true);

			std::string str = idAddressToString(*en);
			claspInstance.symTab().addUnique(c, str.c_str());
		}
		en++;
	}

	// idb
	BOOST_FOREACH (ID ruleId, p.idb){
		const Rule& rule = reg->rules.getByID(ruleId);
		BOOST_FOREACH (ID h, rule.head){
			if (hexToClasp.find(h.address) == hexToClasp.end()){
				uint32_t c = h.address + 2; // pb.newAtom();
				DBGLOG(DBG, "Clasp index of atom " << h.address << " is " << c);
				hexToClasp[h.address] = Clasp::Literal(c, true);

				std::string str = idAddressToString(h.address);
				claspInstance.symTab().addUnique(c, str.c_str());
			}
		}
		BOOST_FOREACH (ID b, rule.body){
			if (hexToClasp.find(b.address) == hexToClasp.end()){
				uint32_t c = b.address + 2; // pb.newAtom();
				DBGLOG(DBG, "Clasp index of atom " << b.address << " is " << c);
				hexToClasp[b.address] = Clasp::Literal(c, true);

				std::string str = idAddressToString(b.address);
				claspInstance.symTab().addUnique(c, str.c_str());
			}
		}
	}
}

void ClaspSolver::buildOptimizedSymbolTable(){

	hexToClasp.clear();

#ifndef NDEBUG
	std::stringstream ss;
#endif

	// go through symbol table
	const Clasp::SymbolTable& symTab = claspInstance.symTab();
	for (Clasp::SymbolTable::const_iterator it = symTab.begin(); it != symTab.end(); ++it) {
		IDAddress hexAdr = stringToIDAddress(it->second.name.c_str());
		hexToClasp[hexAdr] = it->second.lit;
		claspToHex[it->second.lit].push_back(hexAdr);
#ifndef NDEBUG
		ss << "Hex " << hexAdr << " <--> " << (it->second.lit.sign() ? "" : "!") << it->second.lit.var() << std::endl;
#endif
	}
	DBGLOG(DBG, "Symbol table of optimized program: " << std::endl << ss.str());
}

std::string ClaspSolver::idAddressToString(IDAddress adr){
	std::stringstream ss;
	ss << adr;
	return ss.str();
}

IDAddress ClaspSolver::stringToIDAddress(std::string str){
	return atoi(str.c_str());
}

void ClaspSolver::runClasp(){

	DBGLOG(DBG, "ClaspThread: Initialization");
	DBGLOG(DBG, "ClaspThread: Waiting for requests");
	sem_request.wait();	// continue with execution of main thread

	Clasp::solve(claspInstance, params);

	DBGLOG(DBG, "Clasp terminated -> Exit thread");
	endOfModels = true;
	sem_answer.post();
}

bool ClaspSolver::sendProgramToClasp(OrdinaryASPProgram& p){

	const int false_ = 1;	// 1 is our constant "false"

//	eqOptions.iters = 0;	// disable program optimization
	pb.startProgram(claspInstance, eqOptions);
	pb.setCompute(false_, false);

	buildInitialSymbolTable(p, pb);

#ifndef NDEBUG
	std::stringstream programstring;
	RawPrinter printer(programstring, reg);
#endif

	// transfer edb
	DBGLOG(DBG, "Sending EDB to clasp");
	bm::bvector<>::enumerator en = p.edb->getStorage().first();
	bm::bvector<>::enumerator en_end = p.edb->getStorage().end();
	while (en < en_end){
DBGLOG(DBG, "Fact " << hexToClasp[*en].var());
		// add fact
		pb.startRule();
		pb.addHead(hexToClasp[*en].var());
		pb.endRule();

		en++;
	}
#ifndef NDEBUG
	programstring << *p.edb << std::endl;
#endif

	// transfer idb
	DBGLOG(DBG, "Sending IDB to clasp");
	BOOST_FOREACH (ID ruleId, p.idb){
		const Rule& rule = reg->rules.getByID(ruleId);
#ifndef NDEBUG
		programstring << (rule.head.size() == 0 ? "(constraint)" : "(rule)") << " ";
		printer.print(ruleId);
		programstring << std::endl;
#endif
		if (rule.head.size() > 1){
			// shifting
			DBGLOG(DBG, "Shifting disjunctive rule " << ruleId);
			for (int keep = 0; keep < rule.head.size(); ++keep){
				pb.startRule(Clasp::BASICRULE);
				int hi = 0;
				BOOST_FOREACH (ID h, rule.head){
					if (hi == keep){
						// add literal to head
						pb.addHead(hexToClasp[h.address].var());
					}
					hi++;
				}
				BOOST_FOREACH (ID b, rule.body){
					// add literal to body
					pb.addToBody(hexToClasp[b.address].var(), !b.isNaf());
				}
				// shifted head atoms
				hi = 0;
				BOOST_FOREACH (ID h, rule.head){
					if (hi != keep){
						// add literal to head
						pb.addToBody(hexToClasp[h.address].var(), false);
					}
					hi++;
				}
				pb.endRule();
			}
		}else{
			pb.startRule(Clasp::BASICRULE);
			if (rule.head.size() == 0){
				pb.addHead(false_);
			}
			BOOST_FOREACH (ID h, rule.head){
				// add literal to head
				pb.addHead(hexToClasp[h.address].var());
			}
			BOOST_FOREACH (ID b, rule.body){
				// add literal to body
				pb.addToBody(hexToClasp[b.address].var(), !b.isNaf());
			}
			pb.endRule();
		}
	}

#ifndef NDEBUG
	DBGLOG(DBG, "Program is: " << std::endl << programstring.str());
#endif
	// Once all rules are defined, call endProgram() to load the (simplified)
	bool initiallyInconsistent = !pb.endProgram();

	// rebuild the symbol table as it might have changed due to optimization
	buildOptimizedSymbolTable();

	return initiallyInconsistent;
}

/*
class MyDistributor : public Clasp::Distributor{
public:
	uint32 receive(const Clasp::Solver& in, Clasp::SharedLiterals** out, uint32 maxOut){
		std::cout << "Distributing learned knowledge" << std::endl;
	}

	MyDistributor() : Clasp::Distributor(2, 3, 32){
	}

protected:
	void doPublish(const Clasp::Solver& source, Clasp::SharedLiterals* lits){
		std::cout << "Distributing learned knowledge" << std::endl;
	}
};
*/

ClaspSolver::ClaspSolver(ProgramCtx& c, OrdinaryASPProgram& p) : ctx(c), program(p), sem_request(0), sem_answer(0), modelRequest(false), terminationRequest(false), endOfModels(false), translatedNogoods(0){

	reg = ctx.registry();

//claspInstance.enableUpdateShortImplications(false);
//claspInstance.enableConstraintSharing();
//claspInstance.enableLearntSharing(new MyDistributor());

	clauseCreator = new Clasp::ClauseCreator(claspInstance.master());
	bool initiallyInconsistent = sendProgramToClasp(p);
	DBGLOG(DBG, "Initially inconsistent: " << initiallyInconsistent);

	// if the program is initially inconsistent we do not need to do a search at all
	modelCount = 0;
	if (initiallyInconsistent){
		endOfModels = true;
		claspThread = NULL;
	}else{
		if (pb.dependencyGraph() && pb.dependencyGraph()->nodes() > 0) {
			DBGLOG(DBG, "Adding unfounded set checker");
			Clasp::DefaultUnfoundedCheck* ufs = new Clasp::DefaultUnfoundedCheck();
			ufs->attachTo(*claspInstance.master(), pb.dependencyGraph()); // register with solver and graph & transfer ownership
		}

		std::stringstream prog;
		pb.writeProgram(prog);
		DBGLOG(DBG, "Program in LParse format: " << prog.str());

		// add enumerator
		DBGLOG(DBG, "Adding enumerator");
		claspInstance.addEnumerator(new Clasp::BacktrackEnumerator(0, new ModelEnumerator(*this)));
		claspInstance.enumerator()->enumerate(0);

		// add propagator
		DBGLOG(DBG, "Adding external propagator");
		ExternalPropagator* ep = new ExternalPropagator(*this);
		claspInstance.addPost(ep);

		// endInit() must be called once before the search starts
		DBGLOG(DBG, "Finalizing clasp initialization");
		claspInstance.endInit();

		DBGLOG(DBG, "Starting clasp thread");

		claspThread = new boost::thread(boost::bind(&ClaspSolver::runClasp, this));
	}
}

ClaspSolver::~ClaspSolver(){

	// is clasp still active?
	if (!endOfModels){
		DBGLOG(DBG, "ClaspSolver destructor: Clasp is still running - Requesting all outstanding models");
		while (getNextModel() != InterpretationConstPtr());

/*		// activate this code if there is a breakout possibility in ModelEnumerator::reportModel in case of termination requests
		DBGLOG(DBG, "MainThread destructor: clasp is still active");
		DBGLOG(DBG, "MainThread destructor: sending termination request");
		terminationRequest = true;
		sem_request.post();
		DBGLOG(DBG, "MainThread destructor: waiting for answer");
		sem_answer.wait();
		claspThread->join();
		DBGLOG(DBG, "MainThread destructor: ClaspThread terminated");
*/






		DBGLOG(DBG, "Joining ClaspThread");
		claspThread->join();
	}else{
		DBGLOG(DBG, "ClaspSolver destructor: Clasp has already terminated");
	}
	DBGLOG(DBG, "Deleting ClauseCreator");
	delete clauseCreator;

	DBGLOG(DBG, "Deleting ClaspThread");
	if (claspThread) delete claspThread;
}

std::string ClaspSolver::getStatistics(){
	std::stringstream ss;
	ss <<	"Guesses: " << claspInstance.master()->stats.choices << std::endl <<
		"Conflicts: " << claspInstance.master()->stats.conflicts << std::endl <<
		"Models: " << claspInstance.master()->stats.models;
	return ss.str();
}

void ClaspSolver::addExternalLearner(LearningCallback* lb){
	learner.insert(lb);
}

void ClaspSolver::removeExternalLearner(LearningCallback* lb){
	learner.erase(lb);
}

int ClaspSolver::addNogood(Nogood ng){
//	addNogoodToClasp(ng);

	nogoods.push_back(ng);
	return nogoods.size() - 1;
}

void ClaspSolver::removeNogood(int index){
//	nogoods.erase(nogoods.begin() + index);
	throw std::runtime_error("ClaspSolver::removeNogood not implemented");
}

int ClaspSolver::getNogoodCount(){
	return nogoods.size();
}

InterpretationConstPtr ClaspSolver::getNextModel(){

	if (endOfModels){
/*
		if (modelCount == 0){
			std::vector<std::vector<ID> > ngt = convertClaspNogood(claspInstance.master()->conflict());
			std::cout << "hasConflict: " << claspInstance.master()->hasConflict() << ", hasStopConflict: " << claspInstance.master()->hasStopConflict() << " CS: " << ngt.size() << std::endl;
			std::vector<Nogood> ngg = convertClaspNogood(ngt);
			std::cout << "Learned conflicts:" << std::endl;
			BOOST_FOREACH (Nogood ng, ngg){
				std::cout << ng << std::endl;
				std::cout << std::endl;
			}
		}
*/
/*
std::cout << "Component has no models for input interpretation " << *program.edb << ", contraditory nogoods are:" << std::endl;
InternalGroundDASPSolver igdas(ctx, program);
assert(igdas.getNextModel() == InterpretationConstPtr());
std::vector<Nogood> ngg = igdas.getContradictoryNogoods();
BOOST_FOREACH (Nogood ng, ngg){
	std::cout << ng << std::endl;

	BOOST_FOREACH (ID id, ng){
		std::cout << "Cause of " << id.address << " is " << igdas.getCause(id.address) << std::endl;
	}

}
std::cout << "---" << std::endl;
*/
/*
DBGLOG(DBG, "Analyzing inconsistency");
InconsistencyAnalyzer ia(ctx);
ia.explainInconsistency(program, program.edb);
*/

		return InterpretationConstPtr();
	}

	DBGLOG(DBG, "MainThread: Sending nextModel request");
	modelRequest = true;
	sem_request.post();
	DBGLOG(DBG, "MainThread: Waiting for answer to nextModel request");
	sem_answer.wait();

	if (endOfModels){
		DBGLOG(DBG, "MainThread: endOfModels");
		return InterpretationConstPtr();
	}else{
		DBGLOG(DBG, "MainThread: Got a model: " << *nextModel);
		modelCount++;
		return nextModel;
	}
}

int ClaspSolver::getModelCount(){
	return modelCount;
}

InterpretationPtr ClaspSolver::projectToOrdinaryAtoms(InterpretationConstPtr intr){
	if (intr == InterpretationConstPtr()){
		return InterpretationPtr();
	}else{
		InterpretationPtr answer = InterpretationPtr(new Interpretation(reg));
		answer->add(*intr);

		if (program.mask != InterpretationConstPtr()){
			answer->getStorage() -= program.mask->getStorage();
		}

		DBGLOG(DBG, "Projected " << *intr << " to " << *answer);
		return answer;
	}
}

DLVHEX_NAMESPACE_END

#endif