# Settings
IP_PORT='localhost:8080'
WEBSERV_DIR='..'
EXEC_NAME='a.out'
CONFIG_FILE='TestConfiguration.conf'
UPLOAD_DIR='./ServerRoot/Method/Post/upload'

# Execution
make -C $WEBSERV_DIR > /dev/null
if [ "$?" != "0" ]; then
	echo "Makefile Error"
	exit 1
fi

rm -r $UPLOAD_DIR

$WEBSERV_DIR/$EXEC_NAME $CONFIG_FILE > /dev/null 2>&1 &

#process=$(ps -C ${EXEC_NAME} | grep a.out)
#if [ -z "$process" ]; then
#	echo "ERROR: not running: $EXEC_NAME"
#	exit 1
#fi

make > /dev/null && python3 src/main.py $IP_PORT $@

pkill $EXEC_NAME
