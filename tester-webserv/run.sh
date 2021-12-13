############# Settings ##########################

getRealPath() {
    [[ $1 = /* ]] && echo "$1" || echo "$PWD/${1#./}"
}

WEBSERV_DIR='..'
PYTHON_DIR='./PythonTester'
EXEC_NAME='a.out'
CONFIG_FILE=$(getRealPath './TestConfiguration.conf')
SERVER_ROOT=$(getRealPath './ServerRoot')

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

source prepare.sh $SERVER_ROOT

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

make -C $PYTHON_DIR > /dev/null && python3 $PYTHON_DIR/src/main.py $SERVER_ROOT $@

pkill $EXEC_NAME
