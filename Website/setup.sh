DEFAULT=default_config.conf
TARGET=website_config.conf
PYTHON_PATH=$(which python3)
PHP_CGI_PATH=$(which php-cgi)

cp $DEFAULT $TARGET
if [ ! -z "$PYTHON_PATH" ]; then
	sed "s~PYTHON_PATH~$PYTHON_PATH~g" -i $TARGET
fi

if [ ! -z "$PHP_CGI_PATH" ]; then
	sed "s~PHP_CGI_PATH~$PHP_CGI_PATH~g" -i $TARGET
fi
