CPUS  := $(shell nproc)
INI   := $(patsubst assets/%,%,$(wildcard assets/*.ini))
# Use perf list for more hardware events
PERF_STAT := perf stat \
	-e branch-instructions \
	-e branch-misses \
	-e cache-references \
	-e cache-misses \
	-e cpu-cycles \
	-e instructions \
	-e stalled-cycles-backend \
	-e stalled-cycles-frontend \
	-e context-switches \
	-e cpu-clock \
	-e cpu-migrations \
	-e page-faults

.PHONY: build build-debug $(INI) test

ARCHIVE := s0215648

build:
	cmake -DCMAKE_BUILD_TYPE=Release -B $@
	+make -C $@ engine

build/libcgengine.a::
	cmake -DCMAKE_BUILD_TYPE=Release -B build
	+make -C build cgengine

build-debug:
	cmake -DCMAKE_BUILD_TYPE=Debug -B $@
	+make -C $@ engine

build-debug/libcgengine.a::
	cmake -DCMAKE_BUILD_TYPE=Debug -B build-debug
	+make -C build-debug cgengine

#test: build-debug
#	cd assets && for f in *.ini; do ../$</engine "$$f" || exit; done

test-intro: build-debug
	cd assets && for f in Intro*.ini; do ../$</engine "$$f" || exit; done

test-line-drawings: build-debug
	cd assets && for f in line_*.ini; do ../$</engine "$$f" || exit; done

test-zbuffering: build-debug
	cd assets && for f in z_buffering*.ini; do ../$</engine "$$f" || exit; done

test-fractals: build-debug
	cd assets && for f in 3d_fractals*.ini; do ../$</engine "$$f" || exit; done

test-clipping: build-debug
	cd assets && for f in clipping*.ini; do ../$</engine "$$f" || exit; done

test-specular: build-debug
	cd assets && for f in specular_light*.ini; do ../$</engine "$$f" || exit; done

test-texture: build-debug test-intro | assets/honk.bmp
	cd assets && ../$</engine texture*.ini

test-thicken: build-debug
	cd assets && ../$</engine spheres_and_cylinders*.ini

test-cubemap: build-debug | assets/mountains.bmp
	cd assets && ../$</engine cubemap*.ini

assets/%.bmp: assets/%.ini build
	cd assets && ../build/engine $(patsubst assets/%,%,$<)

assets/honk.bmp: %.bmp: %.webp
	ffmpeg -y -i $< -pix_fmt bgr24 $@

assets/ambulance.bmp: %.bmp: %.png
	ffmpeg -y -i $< -pix_fmt bgr24 $@

mountain_images := posz.jpg negz.jpg posx.jpg negx.jpg posy.jpg negy.jpg
assets/mountains.bmp: $(patsubst %,assets/cubemap/mountain-skyboxes/Maskonaive/%,$(mountain_images))
	./mkcubemap.sh $^ $@

test: $(INI)

$(INI): build-debug
	@echo $@
	@cd assets && ../$</engine $@ > /dev/null || echo $@ failed with code $$? || exit 1

bench-sep: $(patsubst %.ini,bench-sep-%,$(INI))
	$(PERF_STAT) make -C . $^

bench-sep-%: build
	cd assets && ../$</engine "$(patsubst bench-sep-%,%.ini,$@)"

bench-batch: build | assets/honk.bmp assets/Intro2_Blocks.bmp assets/ambulance.bmp assets/mountains.bmp
	cd assets && $(PERF_STAT) ../$</engine *.ini

bench-batch-%: build | assets/honk.bmp
	cd assets && $(PERF_STAT) ../$</engine $(patsubst bench-batch-%,%*.ini,$@)

cachegrind-batch: build | assets/honk.bmp assets/Intro2_Blocks.bmp
	cd assets && valgrind --tool=cachegrind ../$</engine *.ini

cachegrind-%: build | assets/honk.bmp assets/Intro2_Blocks.bmp
	cd assets && valgrind --tool=cachegrind ../$</engine $(patsubst cachegrind-%,%.ini,$@)

callgrind-%: build | assets/honk.bmp assets/Intro2_Blocks.bmp
	cd assets && valgrind --tool=callgrind \
		--dump-instr=yes \
		--collect-jumps=yes \
		../$</engine $(patsubst callgrind-%,%.ini,$@)

gen: build | assets/honk.bmp
	make -C . $(patsubst %,_gen-%,$(INI))

gen-%: build | assets/honk.bmp
	make -C . $(patsubst _%,%,$@)

_gen-%:
	cd assets && ../build/engine $(patsubst _gen-%,%,$@)

$(ARCHIVE).tar.gz: $(wildcard src/*) $(wildcard include/*) \
	$(wildcard assets/stochastic_*.L2D) $(wildcard assets/stochastic_*.ini) \
		CMakeLists.txt LICENSE README.md
	tar czvf $@ $^

clean:: clean-images clean-bench
	rm -rf $(ARCHIVE) build/ build-debug/

clean-images::
	rm -rf assets/*.bmp assets/extreme/*.bmp

clean-bench::
	rm -rf assets/cachegrind.out.* assets/callgrind.out.*

loop::
	while true; do clear; make -C . build-debug; inotifywait -e CREATE CMakeLists.txt \
		src/ \
		src/shapes/ \
		src/render/ \
		src/render/fragment/ \
		include/ \
		include/shapes/ \
		include/math/ \
		include/render/\
	; done
