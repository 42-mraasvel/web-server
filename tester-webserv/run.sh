IP_PORT='localhost:8080'

if [ "$1" == "all" ]; then
	REQUEST_DIR='./requests'
	FILES="$(find $REQUEST_DIR -type f)"
	python3 src/main.py $IP_PORT $FILES
else
	python3 src/main.py $IP_PORT $@
fi
