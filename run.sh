if [ "$1" == "catch" ]; then
	# ./run.sh catch
	make > /dev/null catch && ./catch.exe
elif [ "$1" == "debug" ]; then
	make debug && ./debug_build.exe
else
	# ./run.sh
	make > /dev/null && ./a.out
fi
