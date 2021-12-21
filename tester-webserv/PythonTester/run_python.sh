SERVER_ROOT='../ServerRoot'
make > /dev/null && python3 src/main.py $SERVER_ROOT $@
