#!/bin/bash

make build

cd assets

(
	for f in *.ini
	do
		echo -n $f, >&2
		perf stat -x , -e task-clock ../build/engine "$f"
	done
) 2> /tmp/bench.csv
