.PHONY: build build-debug

build:
	cmake -DCMAKE_BUILD_TYPE=Release -B $@
	cd $@ && make -j4

build-debug:
	cmake -DCMAKE_BUILD_TYPE=Debug -B $@
	cd $@ && make -j4

test: build-debug
	./$</engine assets/*.ini
