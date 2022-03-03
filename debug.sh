#!/bin/bash

if test -z "$1"
then
	echo "Usage: $0 <ini>" >&2
	exit 1
fi

make build-debug \
	&& cd assets \
	&& time ../build-debug/engine "$1".ini \
	&& ../feh.sh "$1".bmp \
	&& ../feh.sh "$1".png \
