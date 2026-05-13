#!/bin/bash


cat /etc/services                                           |\
	egrep "/tcp|/udp"                                   |\
	tr '\t/' '  '                                       |\
	tr -s ' '                                           |\
	cut -d ' ' -f -3                                    |\
	sed -e 's@ tcp@ SOCK_STREAM@g;s@ udp@ SOCK_DGRAM@g' |\
	xargs printf "{ \"%s\", %5d, %s },\n"               |\
	column -t                                           |\
	sed -e 's@^@\t@'


