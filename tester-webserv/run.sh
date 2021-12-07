# Settings
IP_PORT='localhost:8080'
WEBSERV_DIR='..'
EXEC_NAME='a.out'
CONFIG_FILE='TestConfiguration.conf'

# Execution
# make -C $WEBSERV_DIR > /dev/null
# if [ "$?" != "0" ]; then
# 	echo "Makefile Error"
# 	exit 1
# fi
# $WEBSERV_DIR/$EXEC_NAME $CONFIG_FILE > /dev/null 2>&1 &
make > /dev/null && python3 src/main.py $IP_PORT $@

# pkill $EXEC_NAME
