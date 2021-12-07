# Settings
IP_PORT='localhost:8080'
WEBSERV_DIR='..'
EXEC_NAME='a.out'

# Execution
make -C $WEBSERV_DIR
$WEBSERV_DIR/$EXEC_NAME TestConfiguration.conf > /dev/null 2>&1 &
make > /dev/null && python3 src/main.py $IP_PORT $@

pkill $EXEC_NAME
