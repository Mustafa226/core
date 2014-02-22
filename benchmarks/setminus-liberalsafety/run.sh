#!/bin/bash

source dlvhex_run_header.sh

# run instances
if [[ $all -eq 1 ]]; then
	# run all instances using the benchmark script run insts
	$bmscripts/runinsts.sh "{1..20}" "$mydir/run.sh" "$mydir" "$to"
else
	# run single instance
	confstr="--extlearn --flpcheck=aufs prog$instance.hex;--extlearn --flpcheck=aufs --liberalsafety prognd$instance.hex"

	inststr=`printf "%03d" ${instance}`

	# write instance files
	prog="
		nsel(X) :- domain(X), &testSetMinus[domain, sel](X)<monotonic domain,antimonotonic sel>.
		sel(X) :- domain(X), &testSetMinus[domain, nsel](X)<monotonic domain, antimonotonic nsel>.
		:- sel(X), sel(Y), sel(Z), X != Y, X != Z, Y != Z."
	for (( j = 1; j <= $instance; j++ ))
	do
		prog="domain($j). $prog"
	done
	echo $prog > prog$instance.hex

	prog="
		nsel(X) :- &testSetMinus[domain, sel](X)<monotonic domain,antimonotonic sel,relativefinitedomain 0 domain>.
		sel(X) :- &testSetMinus[domain, nsel](X)<monotonic domain, antimonotonic nsel,relativefinitedomain 0 domain>.
		:- sel(X), sel(Y), sel(Z), X != Y, X != Z, Y != Z."
	for (( j = 1; j <= $instance; j++ ))
	do
		prog="domain($j). $prog"
	done
	echo $prog > prognd$instance.hex

	$bmscripts/runconfigs.sh "dlvhex2 --plugindir=../../testsuite --verbose=8 CONF" "$confstr" "$inststr" "$to" "$bmscripts/gstimeoutputbuilder.sh"
	rm prog$instance.hex
	rm prognd$instance.hex
fi

