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

size=$(identify -format "%[w]" "$1")
temp=$(mktemp)

echo $size

convert -background white \
	\( -size "$size"x xc:none "$PZ" +append \) \
	\( "$NY" "$PX" "$PY" "$NX" +append \) \
	\( -size "$size"x xc:none "$NZ" +append \) \
	-append "$temp".jpg || exit

ffmpeg -i "$temp".jpg -pix_fmt bgr24 "$OUT"
rm "$temp"
