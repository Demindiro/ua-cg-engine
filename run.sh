#!/bin/bash

if test -z "$1"
then
	echo "Usage: $0 <dir>" >&2
	exit 1
fi

(
	make -j$(nproc) build \
	&& cd assets \
	&& perf stat ../build/engine "$1".ini
) && ./feh.sh assets/"$1".bmp
