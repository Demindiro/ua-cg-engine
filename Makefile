CPUS  := $(shell nproc)
INI   := $(patsubst assets/%,%,$(wildcard assets/*.ini))

.PHONY: build build-debug $(INI) test

ARCHIVE := s0215648

build:
	cmake -DCMAKE_BUILD_TYPE=Release -B $@
	+make -C $@

build-debug:
	cmake -DCMAKE_BUILD_TYPE=Debug -B $@
	+make -C $@

#test: build-debug
#	cd assets && for f in *.ini; do echo "$$f"; ../$</engine "$$f" || exit; done

test-intro: build-debug
	cd assets && for f in Intro*.ini; do echo "$$f"; ../$</engine "$$f" || exit; done

test-line-drawings: build-debug
	cd assets && for f in line_*.ini; do echo "$$f"; ../$</engine "$$f" || exit; done

test-zbuffering: build-debug
	cd assets && for f in z_buffering*.ini; do echo "$$f"; ../$</engine "$$f" || exit; done

test-fractals: build-debug
	cd assets && for f in 3d_fractals*.ini; do echo "$$f"; ../$</engine "$$f" || exit; done

test-clipping: build-debug
	cd assets && for f in clipping*.ini; do echo "$$f"; ../$</engine "$$f" || exit; done

test-specular: build-debug
	cd assets && for f in specular_light*.ini; do echo "$$f"; ../$</engine "$$f" || exit; done

test-texture: build-debug test-intro assets/honk.bmp
	cd assets && ../$</engine texture*.ini

assets/honk.bmp: assets/honk.webp
	ffmpeg -y -i $< $@

test: $(INI)

$(INI): build-debug
	@echo $@
	@cd assets && ../$</engine $@ > /dev/null || echo $@ failed with code $$? || exit 1

bench-sep: $(patsubst %.ini,bench-sep-%,$(INI))
	perf stat make -C . $^

bench-sep-%: build
	cd assets && ../$</engine "$(patsubst bench-sep-%,%.ini,$@)"

bench-batch: build
	cd assets && perf stat ../$</engine *.ini

bench-batch-%: build
	cd assets && perf stat ../$</engine $(patsubst bench-batch-%,%*.ini,$@)

gen: build
	make -C . $(patsubst %,_gen-%,$(INI))

gen-%: build
	make -C . $(patsubst _%,%,$@)

_gen-%:
	cd assets && ../build/engine $(patsubst _gen-%,%,$@)

$(ARCHIVE).tar.gz: $(wildcard src/*) $(wildcard include/*) \
	$(wildcard assets/stochastic_*.L2D) $(wildcard assets/stochastic_*.ini) \
		CMakeLists.txt LICENSE README.md
	tar czvf $@ $^

clean:: clean-images
	rm -rf $(ARCHIVE) build/ build-debug/

clean-images::
	rm -rf assets/*.bmp

loop::
	while true; do clear; make -C . build-debug; inotifywait -e CREATE CMakeLists.txt src/ src/shapes/ src/render/ include/ include/shapes/ include/math/ include/render/; done
