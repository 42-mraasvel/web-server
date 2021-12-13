#Server program log
SERVER_LOG_FILE='./server.log'
SERVER_LOG=on #toggle on/off

WEBSERV_DIR='..'
EXEC_NAME='a.out'

# Execution
make -C $WEBSERV_DIR > /dev/null
if [ "$?" != "0" ]; then
	echo "Makefile Error"
	exit 1
fi

rm -f $SERVER_LOG_FILE

if [ "$SERVER_LOG" == "on" ]; then
	$WEBSERV_DIR/$EXEC_NAME $1 > $SERVER_LOG_FILE 2>&1 &
else
	$WEBSERV_DIR/$EXEC_NAME $1 > /dev/null 2>&1 &
fi
