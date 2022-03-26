CPUS  := $(shell nproc)
INI   := $(patsubst assets/%,%,$(wildcard assets/*.ini))

.PHONY: build build-debug $(INI) test

ARCHIVE := s0215648

CPUS := $(shell nproc)

build:
	cmake -DCMAKE_BUILD_TYPE=Release -B $@
	cd $@ && make -j$(CPUS)

build-debug:
	cmake -DCMAKE_BUILD_TYPE=Debug -B $@
	cd $@ && make -j$(CPUS)

#test: build-debug
#	cd assets && for f in *.ini; do echo "$$f"; ../$</engine "$$f" || exit; done

test-line-drawings: build-debug
	cd assets && for f in line_*.ini; do echo "$$f"; ../$</engine "$$f" || exit; done

test-zbuffering: build-debug
	cd assets && for f in z_buffering*.ini; do echo "$$f"; ../$</engine "$$f" || exit; done

test: $(INI)

$(INI): build-debug
	@echo $@
	@cd assets && ../$</engine $@ > /dev/null || echo $@ failed with code $$? || exit 1

bench-sep: build
	cd assets && for f in *.ini; do echo "$$f"; perf stat ../$</engine "$$f"; done

bench-batch: build
	cd assets && perf stat ../$</engine *.ini

gen: build
	make -j$(CPUS) $(patsubst %,_gen-%,$(INI))

gen-%: build
	make $(patsubst _%,%,$@)

_gen-%:
	cd assets && ../build/engine $(patsubst _gen-%,%,$@)

$(ARCHIVE).tar.gz: $(wildcard src/*) $(wildcard include/*) \
	$(wildcard assets/stochastic_*.L2D) $(wildcard assets/stochastic_*.ini) \
		CMakeLists.txt LICENSE README.md
	tar czvf $@ $^

clean::
	rm -f $(ARCHIVE) assets/*.bmp

loop::
	while true; do make build-debug; inotifywait -e CREATE CMakeLists.txt src/ src/shapes/ include/ include/shapes/; done
