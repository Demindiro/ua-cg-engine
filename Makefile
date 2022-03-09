.PHONY: build build-debug

CPUS := $(shell nproc)

build:
	cmake -DCMAKE_BUILD_TYPE=Release -B $@
	cd $@ && make -j$(CPUS)

build-debug:
	cmake -DCMAKE_BUILD_TYPE=Debug -B $@
	cd $@ && make -j$(CPUS)

test: build-debug
	cd assets && for f in *.ini; do echo "$$f"; ../$</engine "$$f" || exit; done

test-line-drawings: build-debug
	cd assets && for f in line_*.ini; do echo "$$f"; ../$</engine "$$f" || exit; done

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

archive.tar.gz: $(wildcard src/*) $(wildcard include/*) \
	$(wildcard assets/stochastic_*.L2D) $(wildcard assets/stochastic_*.ini) \
	CMakeLists.txt LICENSE
	tar czvf $@ $^
