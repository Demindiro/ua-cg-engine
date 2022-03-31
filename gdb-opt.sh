#!/bin/bash

if test -z "$1"
then
	echo "Usage: $0 <ini>" >&2
	exit 1
fi

make build -j \
	&& cd assets \
	&& gdb --args ../build/engine "$1".ini \
	&& ../../feh.sh "$1".bmp
