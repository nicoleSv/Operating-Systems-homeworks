#!/bin/bash

echo -n "" > encrypted

for letter in $(<secret_message); do
	awk -v l="${letter}" -F' ' '$2==l {printf $1}' morse | tr [:upper:] [:lower:] >> encrypted
done

echo "" >> encrypted
