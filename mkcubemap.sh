#!/usr/bin/env bash

if test -z "$7"
then
	echo "Usage: <+x> <-x> <+y> <-y> <+z> <-z> <out>" >&2
	exit 1
fi

PX="$1"
NX="$2"
PY="$3"
NY="$4"
PZ="$5"
NZ="$6"
OUT="$7"

temp=$(mktemp)
temp2=$(mktemp)

convert +append "$PX" "$PY" "$NX" "$NY" "$temp"
convert -append "$PZ" "$temp" "$NZ" "$temp2"
rm "$temp"

ffmpeg -i "$temp2" -pix_fmt bgr24 "$OUT"
rm "$temp2"
