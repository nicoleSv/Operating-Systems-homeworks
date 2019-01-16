#!/bin/bash

if [ ! -f encrypted ]; then
	echo "Encrypted file does not exist!"
	exit 1
fi

cipher=$(<encrypted)

for letter in $(seq 25); do
	decode=$(echo ${cipher} | tr $(printf %${letter}s | tr ' ' '.')\a-z a-za-z)
	if [[ "${decode}" =~ fuehrer ]]; then
		echo "${decode}"
		exit 0
	fi
done
