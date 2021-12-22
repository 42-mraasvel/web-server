#!/bin/bash
DEFAULT_CONFIG='./Website/website_config.conf'
DEFAULT_EXEC='./webserv.out'

OTHER_ARGS=${@:2}
if [ -z "$OTHER_ARGS" ]; then
	OTHER_ARGS=$DEFAULT_CONFIG
fi

if [ "$#" -eq 0 ]; then
	make -C ./Website
	make && $DEFAULT_EXEC $DEFAULT_CONFIG
elif [ "$1" == "catch" ]; then
	make catch && ./catch.out
elif [ "$1" == "debug" ]; then
	make debug && ./debug.out $OTHER_ARGS
elif [ "$1" == "fsanitize" ]; then
	make fsanitize && ./fsanitize.out $OTHER_ARGS
else
	make $1 && $DEFAULT_EXEC $OTHER_ARGS
fi
