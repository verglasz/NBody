#!/bin/sh

runs=${1:-10}
max_t=${2:-$(nproc)}
far=0.9

if [ "$(hostname)" = "boron"]; then
	steps=500000
else
	steps=60000
fi

bodies="120 180 240"

function doruns {
	cmd=$1
	prefix=$2
	echo "starting $runs runs of \`$cmd\`" >&2
	for i in $(seq $runs); do
		result=$($cmd)
		result=${result%"seconds"}
		result=${result#" Total elapsed time: "}
		printf "$prefix\t$result\n"
		printf "%7.3f " $result >&2
	done
	echo "" >&2
}

function testprog {
	progname=$1
	bodies=$2
	outfile="perf/data/$(hostname)-${progname}"
	args="$bodies $steps"
	if [ "$progname" = "barneshut" ]; then
		args="$args $far"
	fi
	prefix="single $bodies"
	doruns "./single-$progname $args" "$prefix" >> $outfile
	for threads in $(seq $max_t); do
		prefix="$threads $bodies"
		doruns "./multi-$progname $args $threads" "$prefix" >> $outfile
	done
}

for prog in {quadratic,barneshut}; do
	for b in $bodies; do
		echo $prog with $b
		# testprog $prog $b
	done
done

