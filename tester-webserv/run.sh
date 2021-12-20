############# Settings ##########################

getRealPath() {
    [[ $1 = /* ]] && echo "$1" || echo "$PWD/${1#./}"
}

WEBSERV_DIR='..'
EXEC_NAME='a.out'
PYTHON_DIR='./PythonTester'
CPP_DIR='./CppTester'
CONFIG_FILE='./TestConfiguration.conf' #$(getRealPath './TestConfiguration.conf')
SERVER_ROOT='./ServerRoot' #$(getRealPath './ServerRoot')

RUN_WEBSERV="off"

####################################################

source prepare.sh "$SERVER_ROOT"
if [ "$RUN_WEBSERV" == "on" ]; then
bash background_webserver.sh "$CONFIG_FILE"
fi

sleep 0.0001
process=$(ps -a | grep -v grep | grep $EXEC_NAME)
if [ -z "$process" ]; then
	echo "ERROR: not running: $EXEC_NAME"
	exit 1
fi

echo Python Tester...
( cd $PYTHON_DIR ; bash run_python.sh $@ )
echo CPP tester...
( cd $CPP_DIR ; bash run_cpp.sh $@ )

rm -rf $SERVER_ROOT/Method/Post
cp $SERVER_ROOT/Method/Delete/copy.html $SERVER_ROOT/Method/Delete/sample.html
if [ "$RUN_WEBSERV" == "on" ]; then
pkill $EXEC_NAME
fi
