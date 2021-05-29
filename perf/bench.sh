#!/bin/sh

runs=${1:-10}
max_t=${2:-$(nproc)}
far=0.9

if [ "$(hostname)" = "boron" ]; then
	steps=500000
else
	steps=60000
fi

bodies="120 180 240"

doruns () {
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

testprog () {
	progname=$1
	thebodies=$2
	args="$thebodies $steps"
	if [ "$progname" = "barneshut" ]; then
		args="$args $far"
	fi
	prefix="single $thebodies"
	doruns "./single-$progname $args" "$prefix"
	for threads in $(seq $max_t); do
		prefix="$threads $thebodies"
		doruns "./multi-$progname $args $threads" "$prefix"
	done
}

for prog in "quadratic" "barneshut"; do
	outfile="perf/data/$(hostname)-${prog}"
	echo "doing $prog, will do bodies = ($bodies)" >&2
	thebodies=$bodies
	for b in $thebodies; do
		echo $prog with $b >&2
		testprog $prog $b >> $outfile
	done
done

