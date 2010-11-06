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
 * @file   TestDependencyGraphFull.cpp
 * @author Peter Schueller <ps@kr.tuwien.ac.at>
 * 
 * @brief  Test the dependency graph builder (and the graph)
 */

#include <boost/cstdint.hpp>
#include "dlvhex/DependencyGraph.hpp"
#include "dlvhex/DependencyGraphFull.hpp"
#include "dlvhex/HexParser.hpp"
#include "dlvhex/ProgramCtx.h"
#include "dlvhex/PluginInterface.h"

#define BOOST_TEST_MODULE "TestDependencyGraph"
#include <boost/test/unit_test.hpp>

#include <iostream>
#include <fstream>

#define LOG_REGISTRY_PROGRAM(ctx) \
  ctx.registry->logContents(); \
	RawPrinter printer(std::cerr, ctx.registry); \
	LOG("edb"); \
	printer.printmany(ctx.edb,"\n"); \
	std::cerr << std::endl; \
	LOG("edb end"); \
	LOG("idb"); \
	printer.printmany(ctx.idb,"\n"); \
	std::cerr << std::endl; \
	LOG("idb end");

DLVHEX_NAMESPACE_USE

inline void makeGraphVizPdf(const char* fname)
{
  std::ostringstream ss;
  ss << "dot " << fname << " -Tpdf -o " << fname << ".pdf";
  system(ss.str().c_str());
}

class TestPluginAtomCount:
	public PluginAtom
{
public:
	TestPluginAtomCount(): PluginAtom()
	{
		monotonic = false;
		inputSize = 1;
		outputSize = 1;
		inputType.push_back(PREDICATE);
	}

	// won't be used
	virtual void retrieve(const Query&, Answer&) throw (PluginError)
		{ assert(false); }
};

class TestPluginAtomReach:
	public PluginAtom
{
public:
	TestPluginAtomReach(): PluginAtom()
	{
		monotonic = true;
		inputSize = 2;
		outputSize = 1;
		inputType.push_back(CONSTANT);
		inputType.push_back(PREDICATE);
	}

	// won't be used
	virtual void retrieve(const Query&, Answer&) throw (PluginError)
		{ assert(false); }
};

class TestPluginAspCtxAcc:
	public PluginAtom
{
public:
	TestPluginAspCtxAcc(): PluginAtom()
	{
		monotonic = false;
		inputSize = 5;
		outputSize = 0;
		inputType.push_back(CONSTANT);
		inputType.push_back(PREDICATE);
		inputType.push_back(PREDICATE);
		inputType.push_back(PREDICATE);
		inputType.push_back(CONSTANT);
	}

	// won't be used
	virtual void retrieve(const Query&, Answer&) throw (PluginError)
		{ assert(false); }
};

BOOST_AUTO_TEST_CASE(testNonext) 
{
  ProgramCtx ctx;
  ctx.registry = RegistryPtr(new Registry);

  std::stringstream ss;
  ss <<
		// a <-(+)-> f(X) (head/head = disjunctive)
    // 2x head -> rule
    "a v f(X)." << std::endl <<
	  // X(a) -(+)-> f(X) (unifying+?)
	  // f(b) -(+)-> f(X) (unifying+?)
	  // b -> rule (head/rule = positive)
    // rule -(+)-> X(a) (rule/body = positive)
	  // rule -(-)-> f(b) (rule/nafbody = negative)
    "b :- X(a), not f(b)." << std::endl <<
	  // X(b) -(+c)-> f(X) (unifying pos_constraint)
	  // f(a) -(-c)-> f(X) (unifying neg_constraint)
    // rule -> body (pos_constraint)
    // rule -> nafbody (neg_constraint)
    ":- X(b), not f(a)." << std::endl;
  HexParser parser(ctx);
  BOOST_REQUIRE_NO_THROW(parser.parse(ss));

	LOG_REGISTRY_PROGRAM(ctx);

  ID ida = ctx.registry->ogatoms.getIDByString("a");
  ID idb = ctx.registry->ogatoms.getIDByString("b");
  ID idfb = ctx.registry->ogatoms.getIDByString("f(b)");
  ID idfa = ctx.registry->ogatoms.getIDByString("f(a)");
  BOOST_REQUIRE((ida | idb | idfb | idfa) != ID_FAIL);

  ID idfX = ctx.registry->onatoms.getIDByString("f(X)");
  ID idXa = ctx.registry->onatoms.getIDByString("X(a)");
  ID idXb = ctx.registry->onatoms.getIDByString("X(b)");
  BOOST_REQUIRE((idfX | idXa | idXb) != ID_FAIL);

  // full dependency graph
  {
    DependencyGraphFull depgraph(ctx.registry);
    depgraph.createNodesAndBasicDependencies(ctx.idb);
    depgraph.createUnifyingDependencies();

    BOOST_CHECK_EQUAL(depgraph.countNodes(), 10);
    BOOST_CHECK_EQUAL(depgraph.countDependencies(), 13);

    const char* fnamev = "testDependencyGraphNonextFullVerbose.dot";
    LOG("dumping verbose graph to " << fnamev);
    std::ofstream filev(fnamev);
    depgraph.writeGraphViz(filev, true);
    makeGraphVizPdf(fnamev);

    const char* fnamet = "testDependencyGraphNonextFullTerse.dot";
    LOG("dumping terse graph to " << fnamet);
    std::ofstream filet(fnamet);
    depgraph.writeGraphViz(filet, false);
    makeGraphVizPdf(fnamet);
  }

  // smaller more efficient dependency graph
  {
    DependencyGraph depgraph(ctx.registry);
    std::vector<ID> auxRules;
    depgraph.createDependencies(ctx.idb, auxRules);

    // TODO
    //BOOST_CHECK_EQUAL(depgraph.countNodes(), 10);
    //BOOST_CHECK_EQUAL(depgraph.countDependencies(), 13);

    // TODO test dependencies (will do manually with graphviz at the moment)

    const char* fnamev = "testDependencyGraphNonextVerbose.dot";
    LOG("dumping verbose graph to " << fnamev);
    std::ofstream filev(fnamev);
    depgraph.writeGraphViz(filev, true);
    makeGraphVizPdf(fnamev);

    const char* fnamet = "testDependencyGraphNonextTerse.dot";
    LOG("dumping terse graph to " << fnamet);
    std::ofstream filet(fnamet);
    depgraph.writeGraphViz(filet, false);
    makeGraphVizPdf(fnamet);
  }
}

BOOST_AUTO_TEST_CASE(testExtCountReach) 
{
  ProgramCtx ctx;
  ctx.registry = RegistryPtr(new Registry);

  std::stringstream ss;
  ss <<
		// head -> rule
		// rule -> body (pos)
    "item(X) :- part(X)." << std::endl <<
		// head -> rule
		// rule -> body (pos)
		"edge(Y) :- foo(Y)." << std::endl <<
    // head -> rule
		// rule -> body (pos + neg, as count is nonmonotonic)
		// extatom -> item(X) (pos external)
    "num(N) :- &count[item](N)." << std::endl <<
		// head -> rule
		// rule -> body &reach... (pos, reach is monotonic)
		// rule -> body startnode(N) (pos)
		// extatom -> edge(Y) (pos external)
		// extatom -> startnode(N) (pos external)
    "reached(X) :- &reach[N,edge](X), startnode(N)." << std::endl;
  HexParser parser(ctx);
  BOOST_REQUIRE_NO_THROW(parser.parse(ss));

	LOG_REGISTRY_PROGRAM(ctx);

	// create dummy plugin atoms and register them into external atoms
	PluginAtomPtr papCount(new TestPluginAtomCount);
	PluginAtomPtr papReach(new TestPluginAtomReach);
  ID idreach = ctx.registry->terms.getIDByString("reach");
  ID idcount = ctx.registry->terms.getIDByString("count");
  BOOST_REQUIRE((idreach | idcount) != ID_FAIL);
	{
		ExternalAtomTable::PredicateIterator it, it_end;
		for(boost::tie(it, it_end) = ctx.registry->eatoms.getRangeByPredicateID(idreach);
				it != it_end; ++it)
		{
			ExternalAtom ea(*it);
			ea.pluginAtom = papReach;
			ctx.registry->eatoms.update(*it, ea);
		}
	}
	{
		ExternalAtomTable::PredicateIterator it, it_end;
		for(boost::tie(it, it_end) = ctx.registry->eatoms.getRangeByPredicateID(idcount);
				it != it_end; ++it)
		{
			ExternalAtom ea(*it);
			ea.pluginAtom = papCount;
			ctx.registry->eatoms.update(*it, ea);
		}
	}

  // full dependency graph
  {
    // clone registry, because full depgraph will modify it for auxiliary rules
    RegistryPtr cloneRegistry(new Registry(*ctx.registry));
    DependencyGraphFull depgraph(cloneRegistry);
    depgraph.createNodesAndBasicDependencies(ctx.idb);
    depgraph.createUnifyingDependencies();
    std::vector<ID> auxRules;
    depgraph.createExternalDependencies(auxRules);

    BOOST_CHECK_EQUAL(auxRules.size(), 1);
    BOOST_CHECK_EQUAL(depgraph.countNodes(), 13+2); // 1 aux rule + 1 aux predicate
    BOOST_CHECK_EQUAL(depgraph.countDependencies(), 12+3); // 3 aux dependencies

    const char* fnamev = "testDependencyGraphExtCountReachFullVerbose.dot";
    LOG("dumping verbose graph to " << fnamev);
    std::ofstream filev(fnamev);
    depgraph.writeGraphViz(filev, true);
    makeGraphVizPdf(fnamev);

    const char* fnamet = "testDependencyGraphExtCountReachFullTerse.dot";
    LOG("dumping terse graph to " << fnamet);
    std::ofstream filet(fnamet);
    depgraph.writeGraphViz(filet, false);
    makeGraphVizPdf(fnamet);
  }

  // smaller more efficient dependency graph
  {
    DependencyGraph depgraph(ctx.registry);
    std::vector<ID> auxRules;
    depgraph.createDependencies(ctx.idb, auxRules);

    // TODO
    //BOOST_CHECK_EQUAL(depgraph.countNodes(), 10);
    //BOOST_CHECK_EQUAL(depgraph.countDependencies(), 13);

    // TODO test dependencies (will do manually with graphviz at the moment)

    const char* fnamev = "testDependencyGraphExtCountReachVerbose.dot";
    LOG("dumping verbose graph to " << fnamev);
    std::ofstream filev(fnamev);
    depgraph.writeGraphViz(filev, true);
    makeGraphVizPdf(fnamev);

    const char* fnamet = "testDependencyGraphExtCountReachTerse.dot";
    LOG("dumping terse graph to " << fnamet);
    std::ofstream filet(fnamet);
    depgraph.writeGraphViz(filet, false);
    makeGraphVizPdf(fnamet);
  }
}

// example using MCS-IE encoding from KR 2010 for calculation of equilibria in medical example
BOOST_AUTO_TEST_CASE(testMCSMedEQ) 
{
  ProgramCtx ctx;
  ctx.registry = RegistryPtr(new Registry);

  std::stringstream ss;
  // program was obtained from trunk of mcs-ie via 'dlvhex --verbose=15 --plugindir=`pwd`/../build/src medExample/master.hex --ieenable --ieuseKR2010rewriting'
  ss <<
    "foo(X,c) :- bar. foo(c,Y) :- baz." << std::endl << // this is not from MCS, but required to test scc dependencies!
    "o2(xray_pneumonia)." << std::endl <<
    "b3(pneumonia) :- a2(xray_pneumonia)." << std::endl <<
    "o2(blood_marker)." << std::endl <<
    "b3(marker) :- a2(blood_marker)." << std::endl <<
    "o3(pneumonia)." << std::endl <<
    "b4(need_ab) :- a3(pneumonia)." << std::endl <<
    "o3(atyppneumonia)." << std::endl <<
    "b4(need_strong) :- a3(atyppneumonia)." << std::endl <<
    "o1(allergy_strong_ab)." << std::endl <<
    "b4(allow_strong_ab) :- na1(allergy_strong_ab)." << std::endl <<
    "a1(X) v na1(X) :- o1(X)." << std::endl <<
    ":- not &dlv_asp_context_acc[1,a1,b1,o1,\"./medExample/kb1.dlv\"]()." << std::endl <<
    "ctx(1)." << std::endl <<
    "a2(X) v na2(X) :- o2(X)." << std::endl <<
    ":- not &dlv_asp_context_acc[2,a2,b2,o2,\"./medExample/kb2.dlv\"]()." << std::endl <<
    "ctx(2)." << std::endl <<
    "a3(X) v na3(X) :- o3(X)." << std::endl <<
    ":- not &dlv_asp_context_acc[3,a3,b3,o3,\"./medExample/kb3.dlv\"]()." << std::endl <<
    "ctx(3)." << std::endl <<
    "a4(X) v na4(X) :- o4(X)." << std::endl <<
    ":- not &dlv_asp_context_acc[4,a4,b4,o4,\"./medExample/kb4.dlv\"]()." << std::endl <<
    "ctx(4)." << std::endl;
  HexParser parser(ctx);
  BOOST_REQUIRE_NO_THROW(parser.parse(ss));

	//LOG_REGISTRY_PROGRAM(ctx);

	// create dummy plugin atoms and register them into external atoms
	PluginAtomPtr papAspCtxAcc(new TestPluginAspCtxAcc);
  ID idAspCtxAcc = ctx.registry->terms.getIDByString("dlv_asp_context_acc");
  BOOST_REQUIRE(idAspCtxAcc != ID_FAIL);
	{
		ExternalAtomTable::PredicateIterator it, it_end;
		for(boost::tie(it, it_end) = ctx.registry->eatoms.getRangeByPredicateID(idAspCtxAcc);
				it != it_end; ++it)
		{
			ExternalAtom ea(*it);
			ea.pluginAtom = papAspCtxAcc;
			ctx.registry->eatoms.update(*it, ea);
		}
	}

	// full dependency graph
  {
    // clone registry, because full depgraph will modify it for auxiliary rules
    RegistryPtr cloneRegistry(new Registry(*ctx.registry));
    DependencyGraphFull depgraph(cloneRegistry);
    depgraph.createNodesAndBasicDependencies(ctx.idb);
    depgraph.createUnifyingDependencies();
    std::vector<ID> auxRules;
    depgraph.createExternalDependencies(auxRules);

    const char* fnamev = "testDependencyGraphMCSMedEqFullVerbose.dot";
    LOG("dumping verbose graph to " << fnamev);
    std::ofstream filev(fnamev);
    depgraph.writeGraphViz(filev, true);
    makeGraphVizPdf(fnamev);

    const char* fnamet = "testDependencyGraphMCSMedEqFullTerse.dot";
    LOG("dumping terse graph to " << fnamet);
    std::ofstream filet(fnamet);
    depgraph.writeGraphViz(filet, false);
    makeGraphVizPdf(fnamet);
  }

  // smaller more efficient dependency graph
  {
    DependencyGraph depgraph(ctx.registry);
    std::vector<ID> auxRules;
    depgraph.createDependencies(ctx.idb, auxRules);

    // TODO
    //BOOST_CHECK_EQUAL(depgraph.countNodes(), 10);
    //BOOST_CHECK_EQUAL(depgraph.countDependencies(), 13);

    // TODO test dependencies (will do manually with graphviz at the moment)

    const char* fnamev = "testDependencyGraphMCSMedEqVerbose.dot";
    LOG("dumping verbose graph to " << fnamev);
    std::ofstream filev(fnamev);
    depgraph.writeGraphViz(filev, true);
    makeGraphVizPdf(fnamev);

    const char* fnamet = "testDependencyGraphMCSMedEqTerse.dot";
    LOG("dumping terse graph to " << fnamet);
    std::ofstream filet(fnamet);
    depgraph.writeGraphViz(filet, false);
    makeGraphVizPdf(fnamet);
  }
}

// TODO test aggregate dependencies