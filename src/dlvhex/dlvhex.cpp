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
 * 02110-1301 USA
 */


/**
 * @file   dlvhex.cpp
 * @author Roman Schindlauer
 * @date   Thu Apr 28 15:00:10 2005
 * 
 * @brief  main().
 * 
 */

/** @mainpage dlvhex Source Documentation
 *
 * \section intro_sec Overview
 *
 * You will look into the documentation of dlvhex most likely to implement a
 * plugin. In this case, please continue with the \ref pluginframework "Plugin
 * Interface Module", which contains all necessary information.
 *
 * For an overview on the logical primitives and datatypes used by dlvhex, see
 * \ref dlvhextypes "dlvhex Datatypes".
 */


/**
 * \defgroup dlvhextypes dlvhex Types
 *
 */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "dlvhex/Error.h"
#include "dlvhex/Benchmarking.h"
#include "dlvhex/ProgramCtx.h"
#include "dlvhex/PluginContainer.h"
#include "dlvhex/ASPSolverManager.h"
#include "dlvhex/ASPSolver.h"

#include <boost/tokenizer.hpp>
#include <boost/foreach.hpp>

#include <iostream>
#include <sstream>
#include <cstring>

#include <getopt.h>

DLVHEX_NAMESPACE_USE

/**
 * @brief Print logo.
 */
void
printLogo()
{
	std::cout
		<< "DLVHEX "
#ifdef HAVE_CONFIG_H
		<< VERSION << " "
#endif // HAVE_CONFIG_H
		<< "[build "
		<< __DATE__ 
#ifdef __GNUC__
		<< "   gcc " << __VERSION__ 
#endif
		<< "]" << std::endl
		<< std::endl;
}


/**
 * @brief Print usage help.
 */
void
printUsage(std::ostream &out, const char* whoAmI, bool full)
{
  //      123456789-123456789-123456789-123456789-123456789-123456789-123456789-123456789-
  out << "Usage: " << whoAmI 
      << " [OPTION] FILENAME [FILENAME ...]" << std::endl
      << std::endl;
  
  out << "   or: " << whoAmI 
      << " [OPTION] --" << std::endl
      << std::endl;
  
  if (!full)
    {
      out << "Specify -h or --help for more detailed usage information." << std::endl
	  << std::endl;
      
      return;
    }
  
  //
  // As soos as we have more options, we can introduce sections here!
  //
  //      123456789-123456789-123456789-123456789-123456789-123456789-123456789-123456789-
  out << "     --               Parse from stdin." << std::endl
      << " -s, --silent         Do not display anything than the actual result." << std::endl
    //        << "--strongsafety     Check rules also for strong safety." << std::endl
      << " -p, --plugindir=DIR  Specify additional directory where to look for plugin" << std::endl
      << "                      libraries (additionally to the installation plugin-dir" << std::endl
      << "                      and $HOME/.dlvhex/plugins)." << std::endl
      << " -f, --filter=foo[,bar[,...]]" << std::endl
      << "                      Only display instances of the specified predicate(s)." << std::endl
      << " -a, --allmodels      Display all models also under weak constraints." << std::endl
//      << " -r, --reverse        Reverse weak constraint ordering." << std::endl
//      << "     --ruleml         Output in RuleML-format (v0.9)." << std::endl
      << "     --noeval         Just parse the program, don't evaluate it (only useful" << std::endl
      << "                      with --verbose)." << std::endl
      << "     --keepnsprefix   Keep specified namespace-prefixes in the result." << std::endl
      << "     --solver=S       Use S as ASP engine, where S is one of (dlv,dlvdb,libdlv,libclingo)" << std::endl
      << "     --nocache        Do not cache queries to and answers from external atoms." << std::endl
      << " -v, --verbose[=N]    Specify verbose category (default: 1):" << std::endl
      << "                      1  - program analysis information (including dot-file)" << std::endl
      << "                      2  - program modifications by plugins" << std::endl
      << "                      4  - intermediate model generation info" << std::endl
      << "                      8  - timing information (only if configured with" << std::endl
      << "                                               --enable-debug)" << std::endl
      << "                      add values for multiple categories." << std::endl
      << "     --version        Show version information." << std::endl;
}


void
printVersion()
{
  std::cout << PACKAGE_TARNAME << " " << VERSION << std::endl;

  std::cout << "Copyright (C) 2011 Roman Schindlauer, Thomas Krennwallner, Peter Schüller" << std::endl
	    << "License LGPLv2+: GNU GPL version 2 or later <http://gnu.org/licenses/lgpl.html>" << std::endl
	    << "This is free software: you are free to change and redistribute it." << std::endl
	    << "There is NO WARRANTY, to the extent permitted by law." << std::endl;

  std::cout << std::endl;

  std::cout << "Homepage: http://www.kr.tuwien.ac.at/research/systems/dlvhex/" << std::endl
	    << "Support: dlvhex-devel@lists.sourceforge.net" << std::endl
	    << "Bug reports: http://sourceforge.net/apps/trac/dlvhex/" << std::endl;

  exit(0);
}


/**
 * @brief Print a fatal error message and terminate.
 */
void
InternalError (const char *msg)
{
  std::cerr << std::endl
	    << "An internal error occurred (" << msg << ")."
	    << std::endl
	    << "Please contact <" PACKAGE_BUGREPORT ">." << std::endl;
  exit (99);
}

// config and defaults of dlvhex main
struct Config
{
  bool checkStrongSafety;
  bool optionNoEval;
  bool helpRequested;
  std::string optionPlugindir;
  #if defined(HAVE_DLVDB)
	// dlvdb speciality
	std::string typFile;
  #endif
	// those options unhandled by dlvhex main
	std::list<const char*> pluginOptions;

	Config():
		checkStrongSafety(true),
  	optionNoEval(false),
  	helpRequested(false),
		optionPlugindir(""),
		#if defined(HAVE_DLVDB)
		typFile(),
		#endif
		pluginOptions() {}
};

void processOptionsPrePlugin(int argc, char** argv, Config& config, ProgramCtx& pctx);

int main(int argc, char *argv[])
{
  const char* whoAmI = argv[0];

	// program context
  ProgramCtx pctx;

  // defaults of dlvhex API
  pctx.setASPSoftware(
		ASPSolverManager::SoftwareConfigurationPtr(new DLVSoftware::Configuration));
  pctx.config.setOption("Silent", 0);
  pctx.config.setOption("Verbose", 0);
  pctx.config.setOption("WeakAllModels", 0);
  // TODO was/is not implemented: pctx.config.setOption("WeakReverseAllModels", 0);
  pctx.config.setOption("UseExtAtomCache",1);
  pctx.config.setOption("KeepNamespacePrefix",0);

	// defaults of main
	Config config;

	// manage options we can already manage
	// TODO use boost::program_options
	processOptionsPrePlugin(argc, argv, config, pctx);

  // before anything else we dump the logo
  if (!pctx.config.getOption("Silent"))
		printLogo();

  // no arguments at all: shorthelp with no specific error message
  if (argc == 1)
	{
		printUsage(std::cerr, whoAmI, false);
		return 1;
	}

  // initialize benchmarking (--verbose=8) with scope exit
	// (this cannot be outsourced due to the scope)
  benchmark::BenchmarkController& ctr =
    benchmark::BenchmarkController::Instance();
  if( pctx.config.doVerbose(Configuration::PROFILING) )
  {
    ctr.setOutput(&pctx.config.getVerboseStream());
    // for continuous statistics output, display every 1000'th output
    ctr.setPrintInterval(999);
  }
  else
    ctr.setOutput(0);
  // deconstruct benchmarking (= output results) at scope exit 
  int dummy; // this is needed, as SCOPE_EXIT is not defined for no arguments
  BOOST_SCOPE_EXIT( (dummy) ) {
	  (void)dummy;
	  benchmark::BenchmarkController::finish();
  }
  BOOST_SCOPE_EXIT_END

	// if we throw UsageError inside this, error and usage will be displayed, otherwise only error
	try
	{
		if( !pctx.inputProvider.hasContent() )
			throw UsageError("no input specified!");

		// load plugins
		{
			DLVHEX_BENCHMARK_REGISTER_AND_SCOPE(sid,"loading plugins");
			pctx.pluginContainer.loadPlugins(config.optionPlugindir);
			pctx.showPlugins();
		}

		// now we may offer help, including plugin help
    if( config.helpRequested )
		{
			printUsage(std::cerr, whoAmI, true);
			pctx.pluginContainer.printUsage(std::cerr);
			return 1;
		}

		// process plugin options using plugins
		// (this deletes processed options from config.pluginOptions)
		// TODO use boost::program_options
		pctx.pluginContainer.processOptions(config.pluginOptions);
      
		// handle options not recognized by dlvhex and not by plugins
		if( !config.pluginOptions.empty() )
		{
			std::stringstream bad;
			bad << "Unknown option(s):";
			BOOST_FOREACH(const char* opt, config.pluginOptions)
			{
				bad << " " << opt;
			}
			throw UsageError(bad.str());
		}
      
		// convert input (only done if at least one plugin provides a converter)
		pctx.convert();
      
		// parse input (coming directly from inputprovider or from inputprovider provided by the convert() step)
		pctx.parse();
      
		// rewrite program
		pctx.rewriteEDBIDB();
      
		// check weak safety
		pctx.safetyCheck();

		// associate PluginAtom instances (coming from pctx.pluginContainer) with
		// ExternalAtom instances (in the IDB)
		pctx.associatePluginAtomsWithExtAtoms();

		// create dependency graph (we need the previous step for this)
		pctx.createDependencyGraph();

		// optimize dependency graph (some plugin might want to do this, e.g. partial grounding)
		pctx.optimizeEDBDependencyGraph();
		// everything in the following will be done using the dependency graph and EDB
		#warning IDB and dependencygraph could get out of sync!
      
		// create graph of strongly connected components of dependency graph
		pctx.createComponentGraph();

		// use SCCs to do strong safety check
		pctx.strongSafetyCheck();
		
		// select heuristics and create eval graph
		pctx.createEvalGraph();

		// stop here if no evaluation was requested
		if( config.optionNoEval )
			return 0;

		// setup model builder and configure plugin/dlvhex model processing hooks
		// allow plugins to setup pctx
		pctx.configureModelBuilder();
      
		// evaluate (generally done in streaming mode, may exit early if indicated by hooks)
		// (individual model output should happen here)
		pctx.evaluate();

		// finalization plugin/dlvhex hooks (for accumulating model processing)
		// (accumulated model output/query answering should happen here)
		pctx.postProcess();
	}
  catch(const UsageError &ue)
	{
		std::cerr << "GeneralError: " << ue.getErrorMsg() << std::endl;
		printUsage(std::cerr, whoAmI, true);
		pctx.pluginContainer.printUsage(std::cerr);
		return 1;
	}
  catch(const GeneralError &ge)
	{
		std::cerr << "GeneralError: " << ge.getErrorMsg() << std::endl << std::endl;
		return 1;
	}
	catch(const std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << std::endl << std::endl;
		return 1;
	}

	// regular exit
	return 0;
}

void configurePluginPath(std::string& userPlugindir);

// process whole commandline:
// * recognized arguments are stored into some config
// * non-option arguments are interpreted as input files
//   and used to configure config.inputProvider (exception: .typ files)
// * non-recognized option arguments are stored into config.pluginOptions
void processOptionsPrePlugin(
		int argc, char** argv,
		Config& config, ProgramCtx& pctx)
{
  extern char* optarg;
  extern int optind;
  extern int opterr;

  //
  // prevent error message for unknown options - they might be known to
  // plugins later!
  //
  opterr = 0;

  int ch;
  int longid;
  
  static const char* shortopts = "f:hsvp:ar";
  static struct option longopts[] =
	{
		{ "help", no_argument, 0, 'h' },
		{ "silent", no_argument, 0, 's' },
		{ "verbose", optional_argument, 0, 'v' },
		{ "filter", required_argument, 0, 'f' },
		{ "plugindir", required_argument, 0, 'p' },
		{ "allmodels", no_argument, 0, 'a' },
		{ "reverse", no_argument, 0, 'r' },
		//{ "firstorder", no_argument, &longid, 1 },
		{ "weaksafety", no_argument, &longid, 2 },
		//{ "ruleml",     no_argument, &longid, 3 },
		//{ "dlt",        no_argument, &longid, 4 },
		{ "noeval",     no_argument, &longid, 5 },
		{ "keepnsprefix", no_argument, &longid, 6 },
		{ "solver", required_argument, &longid, 7 },
		{ "nocache",    no_argument, &longid, 8 },
		{ "version",    no_argument, &longid, 9 },
		{ NULL, 0, NULL, 0 }
	};

  while ((ch = getopt_long(argc, argv, shortopts, longopts, NULL)) != -1)
	{
		switch (ch)
		{
		case 'h':
			config.helpRequested = 1;
			break;

		case 's':
			pctx.config.setOption("Silent", 1);
			break;

		case 'v':
			if (optarg)
				pctx.config.setOption("Verbose", atoi(optarg));
			else
				pctx.config.setOption("Verbose", 1);
			break;

		case 'f':
			{
				boost::char_separator<char> sep(",");
				std::string oa(optarg); // g++ 3.3 is unable to pass that at the ctor line below
				boost::tokenizer<boost::char_separator<char> > tok(oa, sep);
				
				for(boost::tokenizer<boost::char_separator<char> >::const_iterator f = tok.begin();
						f != tok.end(); ++f)
					pctx.config.addFilter(*f);
			}
			break;
			
		case 'p':
			config.optionPlugindir = std::string(optarg);
			break;

		case 'a':
			pctx.config.setOption("AllModels", 1);
			break;

		case 'r':
			pctx.config.setOption("ReverseOrder", 1);
			break;

		case 0:
			switch (longid)
				{
				case 2:
					pctx.config.setOption("StrongSafety", 0);
					break;

				//case 3:
				 // pctx.setOutputBuilder(new RuleMLOutputBuilder);
					// XML output makes only sense with silent:
				 // pctx.config.setOption("Silent", 1);
				 // break;

				//case 4:
				//  optiondlt = true;
				//  break;

				case 5:
					config.optionNoEval = true;
					break;

				case 6:
					pctx.config.setOption("KeepNamespacePrefix",1);
					break;

				case 7:
					{
						std::string solver(optarg);
						if (solver == "dlvdb")
						{
							#if defined(HAVE_DLVDB)
							// use DLVDB as ASP solver software
							pctx.setASPSoftware(
							ASPSolverManager::SoftwareConfigurationPtr(new DLVDBSoftware::Configuration));
							#else
							printLogo();
							std::cerr << "The command line option ``--solver=dlvdb´´ "
									<< "requires that dlvhex has compiled-in dlvdb support. "
									<< "Please reconfigure the dlvhex source." 
									<< std::endl;
							exit(1);
							#endif // HAVE_DLVDB
						}
						else
						{
							#warning add clingo support
						}
					}
					break;

				case 8:
					pctx.config.setOption("UseExtAtomCache",0);
					break;

				case 9:
					printVersion();
					break;
				}
			break;

		case '?':
			pctx.addOption(argv[optind - 1]);
			break;
		}
	}

	// configure plugin path
	configurePluginPath(config.optionPlugindir);

	// check input files (stdin, file, or URI)

	// stdin requested, append it first
	if( std::string(argv[optind - 1]) == "--" )
		pctx.inputProvider.addStreamInput(std::cin, "<stdin>");

	// collect further filenames/URIs
	// if we use dlvdb, manage .typ files
	for (int i = optind; i < argc; ++i)
	{
		std::string arg(argv[i]);
		if( arg.substr(arg.size()-4) == ".typ" )
		{
			#if defined(HAVE_DLVDB)
			boost::shared_ptr<ASPSolver::DLVDBSoftware::Configuration> ptr =
				boost::dynamic_pointer_cast<ASPSolver::DLVDBSoftware::Configuration>(pctx.getASPSoftware());
			if( ptr == 0 )
				throw GeneralError(".typ files can only be used if dlvdb backend is used");

			if( !ptr->options.typFile.empty() )
				throw GeneralError("cannot use more than one .typ file with dlvdb");
			
			ptr->options.typFile = arg;
			#endif
		}
		else if( arg.find("http://") == 0 )
		{
			pctx.inputProvider.addURLInput(arg);
		}
		else
		{
			pctx.inputProvider.addFileInput(arg);
		}
	}
}

void configurePluginPath(std::string& userPlugindir)
{
  std::stringstream searchpath;
  
  const char* homedir = ::getpwuid(::geteuid())->pw_dir;
	if( !userPlugindir.empty() )
		searchpath << userPlugindir << ':';
	searchpath << homedir << "/" USER_PLUGIN_DIR << ':' << SYS_PLUGIN_DIR;
	userPlugindir = searchpath.str();
}

/* vim: set noet sw=2 ts=2 tw=80: */

// Local Variables:
// mode: C++
// End:
