.PHONY: build build-debug

build:
	cmake -DCMAKE_BUILD_TYPE=Release -B $@
	cd $@ && make -j4

build-debug:
	cmake -DCMAKE_BUILD_TYPE=Debug -B $@
	cd $@ && make -j4

test: build-debug
	cd assets && for f in *.ini; do echo "$$f"; ../$</engine "$$f" || exit; done

bench-sep: build
	@echo Warmup...
	cd assets && for f in *.ini; do echo "$$f"; ../$</engine "$$f"; done
	@echo Measure...
	cd assets && for f in *.ini; do echo "$$f"; perf stat ../$</engine "$$f"; done

bench-batch: build
	@echo Warmup...
	cd assets && ../$</engine *.ini
	@echo Measure...
	cd assets && perf stat ../$</engine *.ini
