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

WEBSERV_NAME='a.out'
EXEC_NAME='a.out'
CONFIG_FILE='./TestConfiguration.conf' #$(getRealPath '../TestConfiguration.conf')

( cd .. ; make > /dev/null && bash background_webserver.sh $CONFIG_FILE )

sleep 0.0001
process=$(ps -a | grep -v grep | grep $WEBSERV_NAME)
if [ -z "$process" ]; then
	echo "ERROR: not running: $WEBSERV_NAME"
	exit 1
fi

make > /dev/null && ./$EXEC_NAME $@

sleep 0.001
pkill $WEBSERV_NAME
