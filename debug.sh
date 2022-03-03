#!/bin/bash

if test -z "$1" || test -z "$2"
then
	echo "Usage: $0 <dir> <ini>" >&2
	exit 1
fi

make build-debug \
	&& cd assets/"$1" \
	&& time ../../build-debug/engine "$2".ini \
	&& ../../feh.sh "$2".bmp \
	&& ../../feh.sh "$2".png \
