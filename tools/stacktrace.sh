#!/bin/bash


ADDR2LINE=${PLATFORMIO_CORE_DIR}/packages/toolchain-xtensa-esp-elf/bin/xtensa-esp32-elf-addr2line
FIRMWARE=build/esp32dev/firmware.elf
BEFORE=5
AFTER=5
ADDRS=""


while [ "x$1" != "x" ]
do

	case "$1" in

		-a) shift; AFTER="$1";;
		-b) shift; BEFORE="$1";;
		-c) shift; BEFORE="$1"; AFTER="$1";;

		*)  ADDRS="${ADDRS} $1";;

	esac

	shift

done


${ADDR2LINE} -e ${FIRMWARE} -- ${ADDRS} | while read info
	do

		file=$(echo $info | cut -d ':' -f 1)
		line=$(echo $info | cut -d ':' -f 2 | cut -d ' ' -f 1)

		echo
		echo $info

		if [ -f "$file" ]
		then

			cat -n "$file" | sed -n "$((line-BEFORE)),$((line+AFTER))p"

		elif [ "x$file" != "x??" ]
		then

			echo "Unable to open: $file"

		fi

	done


