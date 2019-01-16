#!/bin/bash

if [ $# -ne 2 ]; then
	echo "Two arguments needed!"
	exit 1
fi

if ! [[ "${2}" =~ http://lzdx.bfra.bg/logs ]]; then
	echo "Second argument must be link to log files!"
	exit 2
fi

if ! which lftp &>/dev/null; then
	echo "Command 'lftp' for downloading not found. Run 'sudo apt install lftp'"
	exit 3
fi

if [ ! -d "${1}" ]; then
	mkdir -p "${1}"
fi

dir="${1}"
url="${2}"

lftp -c mirror "${url}" "${dir}/" &>/dev/null
if [ $? -eq 0 ]; then
    echo "Download successful!"
else
    echo "Download failed!"
fi
