############# Settings ##########################

IP_PORT='localhost:8080'
WEBSERV_DIR='..'
EXEC_NAME='a.out'
CONFIG_FILE='TestConfiguration.conf'
SERVER_ROOT='./ServerRoot'

#Server program log
SERVER_LOG_FILE='./server.log'
SERVER_LOG=off #toggle on/off

####################################################

# Execution
make -C $WEBSERV_DIR > /dev/null
if [ "$?" != "0" ]; then
	echo "Makefile Error"
	exit 1
fi

source prepare.sh

rm -f $SERVER_LOG_FILE

if [ "$SERVER_LOG" == "on" ]; then
	$WEBSERV_DIR/$EXEC_NAME $CONFIG_FILE > $SERVER_LOG_FILE 2>&1 &
else
	$WEBSERV_DIR/$EXEC_NAME $CONFIG_FILE > /dev/null 2>&1 &
fi

sleep 0.0001
process=$(ps -a | grep -v grep | grep $EXEC_NAME)
if [ -z "$process" ]; then
	echo "ERROR: not running: $EXEC_NAME"
	exit 1
fi

make > /dev/null && python3 src/main.py $IP_PORT $@

pkill $EXEC_NAME
