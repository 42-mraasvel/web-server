############# Settings ##########################

getRealPath() {
    [[ $1 = /* ]] && echo "$1" || echo "$PWD/${1#./}"
}

WEBSERV_DIR='..'
EXEC_NAME='a.out'
PYTHON_DIR='./PythonTester'
CONFIG_FILE='./TestConfiguration.conf' #$(getRealPath './TestConfiguration.conf')
SERVER_ROOT='./ServerRoot' #$(getRealPath './ServerRoot')

####################################################

source prepare.sh "$SERVER_ROOT"
bash background_webserver.sh "$CONFIG_FILE"

sleep 0.0001
process=$(ps -a | grep -v grep | grep $EXEC_NAME)
if [ -z "$process" ]; then
	echo "ERROR: not running: $EXEC_NAME"
	exit 1
fi

make -C $PYTHON_DIR > /dev/null && python3 $PYTHON_DIR/src/main.py "$SERVER_ROOT" $@

rm -rf $SERVER_ROOT/Method/Post
cp $SERVER_ROOT/Method/Delete/copy.html $SERVER_ROOT/Method/Delete/sample.html
pkill $EXEC_NAME
