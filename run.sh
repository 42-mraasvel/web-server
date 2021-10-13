if [ "$1" == "catch" ]; then
	# ./run.sh catch
	make catch && ./catch.exe
elif [ "$1" == "debug" ]; then
	make debug && ./debug_build.exe
else
	# ./run.sh
	make && ./a.out
fi
