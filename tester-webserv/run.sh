############# Settings ##########################

getRealPath() {
    [[ $1 = /* ]] && echo "$1" || echo "$PWD/${1#./}"
}

WEBSERV_DIR='..'
EXEC_NAME='webserv'
PYTHON_DIR='./PythonTester'
CPP_DIR='./CppTester'
CONFIG_FILE='./TestConfiguration.conf' #$(getRealPath './TestConfiguration.conf')
SERVER_ROOT='./ServerRoot' #$(getRealPath './ServerRoot')

RUN_WEBSERV="on"

####################################################

source prepare.sh "$SERVER_ROOT"
if [ "$RUN_WEBSERV" == "on" ]; then
bash background_webserver.sh "$CONFIG_FILE"
fi

sleep 0.0001
# process=$(ps -a | grep -v grep | grep $EXEC_NAME)
# if [ -z "$process" ]; then
# 	echo "ERROR: not running: $EXEC_NAME"
# 	exit 1
# fi

echo Python Tester...
( cd $PYTHON_DIR ; bash run_python.sh $@ )
echo CPP tester...
( cd $CPP_DIR ; bash run_cpp.sh $@ )


##### remove #####
#rm forbidden file
if [ -f "$FORBIDDEN_FILE" ]; then
	chmod 755 $FORBIDDEN_FILE
	rm -f $FORBIDDEN_FILE
fi
#rm forbidden sub directory
if [ -d "$FORBIDDEN_SUBDIR" ]; then
	if [ "$AC_MAC" = "1" ]; then
		sudo chown $USER $FORBIDDEN_SUBDIR
	fi
	chmod 755 $FORBIDDEN_SUBDIR
	rm -r $FORBIDDEN_SUBDIR
fi

rm -rf $DELETE_DIR
rm -rf $FORBIDDEN_DIR
rm -rf $POST_DIR

if [ "$RUN_WEBSERV" == "on" ]; then
pkill $EXEC_NAME
fi
