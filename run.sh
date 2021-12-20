#!/bin/bash
OTHER_ARGS=${@:2}

if [ "$#" -eq 0 ]; then
	DEFAULT_ARG="./Website/website_config.conf"
	make -C ./Website
	make && ./a.out $DEFAULT_ARG
elif [ "$1" == "catch" ]; then
	# ./run.sh catch
	make catch && ./catch.out $OTHER_ARGS
elif [ "$1" == "debug" ]; then
	make debug && ./debug.out $OTHER_ARGS
elif [ "$1" == "fsanitize" ]; then
	make fsanitize && ./fsanitize.out $OTHER_ARGS
else
	make && ./a.out $@
fi
