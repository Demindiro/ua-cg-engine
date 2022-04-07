#!/usr/bin/env bash

make build assets/honk.bmp assets/Intro2_Blocks.bmp assets/mountains.bmp -j || exit

cd assets
valgrind \
	--tool=callgrind \
	--dump-instr=yes \
	--collect-jumps=yes \
	../build/engine *.ini

