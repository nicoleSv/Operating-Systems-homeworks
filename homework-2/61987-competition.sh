#!/bin/bash

function participants()
{
    dirname=${1}
    sort < <(find ${dirname} -type f -printf "%f\n")
}

function outliers()
{
    dirname=${1}
	all=$(find "${dirname}" -maxdepth 1 -mindepth 1 -type f -exec cat {} \; 2>/dev/null | grep -Fa "QSO:" | awk '{ print $9 }' | tr -d "[[:punct:]\r]" | tr [:lower:] [:upper:] | sort | uniq)

    for name in ${all}; do
	    if [ !  -f "${dirname}/${name}" ]; then
            echo "${name}"
		fi
	done
}

function unique()
{
    dirname=${1}
	all=$(find "${dirname}" -maxdepth 1 -mindepth 1 -type f -exec cat {} \; 2>/dev/null | grep -Fa "QSO:" | awk '{ print $9 }' | tr -d "[[:punct:]\r]" | tr [:lower:] [:upper:] | sort | uniq)
        
    for name in ${all}; do
        if [ $(egrep -l "\<${name}\>" ${dirname}/* | wc -l) -le 3 ]; then
            echo "${name}"
        fi
    done
}

function cross_check()
{
    dirname=${1}
    while read file; do
        while read line; do
                name=$(echo "${line}" | awk '{ print $9 }' | tr -d "[[:punct:]\r]" | tr [:lower:] [:upper:])
                if [ -f "${dirname}/${name}" ]; then
                    time=$(echo "${line}" | awk '{ print $5 }')
                    date=$(echo "${line}" | awk '{ print $4 }')
                    callsign=$(basename ${file})
                    awk -v d=${date} -v t=${time} -v n=${callsign} -v l="${line}" '$9==n && $4==d && $5==t {cnt++} END { if(cnt==0) print l }' "${dirname}/${name}"
                else
                    echo "${line}"
                fi
       done < <(grep -Fa "QSO:" ${file})
    done < <(find ${dirname} -type f | sort)
}

function bonus()
{
    dirname=${1}
    while read file; do
        while read line; do
                name=$(echo "${line}" | awk '{ print $9 }' | tr -d "[[:punct:]\r" | tr [:lower:] [:upper:])
                if [ -f "${dirname}/${name}" ]; then
                    time=$(echo "${line}" | awk '{ print $5 }')
                    date=$(echo "${line}" | awk '{ print $4 }')
                    callsign=$(basename ${file})
                    awk -v d=${date} -v t=${time} -v n=${callsign} -v l="${line}" 'function abs(v) { return v<0 ? -v : v }  {
                    if( $9==n && $4==d && abs((int(t/100)*60+int(t%100))-(int($5/100)*60+int($5%100)))<=3 )
                        cnt++ } END { if(cnt==0) print l }' "${dirname}/${name}"
                else
                    echo "${line}"
                fi
       done < <(grep -Fa "QSO:" ${file})
    done < <(find ${dirname} -type f | sort)
}

if [ $# -ne 2 ]; then
	echo "Two arguments needed!"
	exit 1
fi

if [ ! -d "${1}" ]; then
	echo "First argument must be an existing directory!"
	exit 2
fi

dir="${1}"
func_name="${2}"

if [ "${func_name}" == "participants" ]; then
	participants "${dir}"
elif [ "${func_name}" == "outliers" ]; then
	outliers "${dir}"
elif [ "${func_name}" == "unique" ]; then
	unique "${dir}"
elif [ "${func_name}" == "cross_check" ]; then
    cross_check "${dir}"
elif [ "${func_name}" == "bonus" ]; then
    bonus "${dir}"
else
	echo "Second argument must be a valid function!"
    exit 3
fi
