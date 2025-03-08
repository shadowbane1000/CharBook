#!/usr/bin/env bash

rm -f ../src/artwork/artwork.h

for i in *.png; do
	xxd -i $i >"../src/artwork/${i%.*}.cpp"
	echo "extern unsigned char ${i%.*}_png[];" >>../src/artwork/artwork.h
	echo "extern unsigned int ${i%.*}_png_len;" >>../src/artwork/artwork.h
done

