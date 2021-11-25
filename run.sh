#!/bin/bash
OTHER_ARGS=${@:2}

if [ "$#" -eq 0 ]; then
	make && ./a.out $@
elif [ "$1" == "catch" ]; then
	# ./run.sh catch
	make catch && ./catch.out $OTHER_ARGS
elif [ "$1" == "debug" ]; then
	make debug && ./debug.out $OTHER_ARGS
elif [ "$1" == "fsanitize" ]; then
	make fsanitize && ./fsanitize.out $OTHER_ARGS
else
	echo "run.sh: error: unknown argument: \"$1\""
fi
