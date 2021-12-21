#!/bin/bash
DEFAULT_CONFIG='./Website/website_config.conf'

OTHER_ARGS=${@:2}
if [ "$#" -eq 0 ]; then
	make -C ./Website
	make && ./a.out $DEFAULT_CONFIG
elif [ "$1" == "catch" ]; then
	make catch && ./catch.out $OTHER_ARGS
elif [ "$1" == "debug" ]; then
	make debug && ./debug.out $OTHER_ARGS
elif [ "$1" == "fsanitize" ]; then
	make fsanitize && ./fsanitize.out $OTHER_ARGS
else
	make && ./a.out $@
fi
