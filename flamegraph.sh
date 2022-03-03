#!/bin/bash

perf record -g ../build/engine "$@"
perf script > /tmp/out.perf
stackcollapse-perf /tmp/out.perf > /tmp/out.folded
flamegraph --width 4800 /tmp/out.folded > /tmp/out.svg
inkscape /tmp/out.svg
