# if [ "$1" == "debug" ]; then
# 	make debug > /dev/null && ./debug.out
# elif [ "$1" == "fsanitize" ]; then
# 	make fsanitize > /dev/null && ./fsanitize.out
# else
# 	make > /dev/null && ./a.out $@
# fi


############# Settings ##########################

getRealPath() {
    [[ $1 = /* ]] && echo "$1" || echo "$PWD/${1#./}"
}

EXEC_NAME='a.out'
CONFIG_FILE=$(getRealPath '../TestConfiguration.conf')
SERVER_ROOT=$(getRealPath '../ServerRoot')

( cd .. ; bash background_webserver.sh $CONFIG_FILE )

sleep 0.0001
process=$(ps -a | grep -v grep | grep $EXEC_NAME)
if [ -z "$process" ]; then
	echo "ERROR: not running: $EXEC_NAME"
	exit 1
fi

make > /dev/null && ./a.out $@

sleep 0.001
pkill $EXEC_NAME
