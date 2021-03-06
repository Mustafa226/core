#!/bin/bash

runheader=$(which dlvhex_run_header.sh)
if [[ $runheader == "" ]] || [ $(cat $runheader | grep "dlvhex_run_header.sh Version 1." | wc -l) == 0 ]; then
	echo "Could not find dlvhex_run_header.sh (version 1.x); make sure that the benchmarks/script directory is in your PATH"
	exit 1
fi
source $runheader

# run instances
if [[ $all -eq 1 ]]; then
	# run all instances using the benchmark script run insts
	$bmscripts/runinsts.sh "instances/*.hex" "$mydir/run.sh" "$mydir" "$to" "" "" "$req"
else
        dlvinst=$(mktemp "inst_${inststr}_XXXXXXXXXX.dlv")
	cat $instance | sed 's/(\"/([/' | sed 's/\")/])/' | sed 's/;/,/g' > $dlvinst

	# run single instance
	confstr="dlvhex2 --plugindir=../../testsuite --verbose=8 --extlearn --flpcheck=aufs --ufslearn=none --liberalsafety mergesort.hex -n=1 $instance;dlvhex2 --plugindir=../../testsuite --verbose=8 --extlearn --flpcheck=aufs --ufslearn=none --strongsafety mergesort_strongsafety.hex -n=1 $instance;dlv -nofinitecheck mergesort.dlv $dlvinst"

	$bmscripts/runconfigs.sh "CONF" "$confstr" "$instance" "$to" "$bmscripts/gstimeoutputbuilder.sh"

	rm $dlvinst
fi

