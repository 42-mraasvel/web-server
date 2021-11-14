OTHER_ARGS=${@:2}

if [ "$1" == "catch" ]; then
	# ./run.sh catch
	make > /dev/null catch && ./catch.exe $OTHER_ARGS
elif [ "$1" == "debug" ]; then
	make debug && ./debug_build.exe $OTHER_ARGS
else
	# ./run.sh
	make > /dev/null && ./a.out $@
fi
