#!/usr/bin/env bash

make assets/honk.bmp assets/Intro2_Blocks.bmp assets/mountains.bmp

cd assets
valgrind \
	--tool=callgrind \
	--dump-instr=yes \
	--collect-jumps=yes \
	../build/engine "$@.ini"

