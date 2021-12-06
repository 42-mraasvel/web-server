IP_PORT='localhost:8080'

make > /dev/null && python3 src/main.py $IP_PORT $@
