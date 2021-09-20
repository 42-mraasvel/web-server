if [ "$1" == "catch" ]; then
	# ./run.sh catch
	make catch && ./catch.exe
else
	# ./run.sh
	make && ./a.out
fi
