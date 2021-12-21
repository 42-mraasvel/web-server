############# Settings ##########################
#directory
SERVER_ROOT=$1
METHOD_DIR="$SERVER_ROOT/Method"
FORBIDDEN_DIR=$METHOD_DIR'/forbidden'
GET_DIR=$METHOD_DIR'/Get'
POST_DIR=$METHOD_DIR'/Post'
DELETE_DIR=$METHOD_DIR'/Delete'

#files
METHOD_SAMPLE_FILE=$GET_DIR'/sample.html'
FORBIDDEN_FILE=$FORBIDDEN_DIR'/forbidden.html'
FORBIDDEN_SUBDIR=$FORBIDDEN_DIR'/forbidden_dir'

#special (DO NOT CHANGE!)
AC_MAC="0"

# Make CGI
make > /dev/null

####################################################

#build all Method directory
mkdir -p $DELETE_DIR
mkdir -p $FORBIDDEN_DIR
mkdir -p $POST_DIR

#build for Method/Delete
cp $METHOD_SAMPLE_FILE $DELETE_DIR

#build forbidden file
if [ ! -f "$FORBIDDEN_FILE" ]; then
	cp $METHOD_SAMPLE_FILE $FORBIDDEN_FILE
	chmod a=r $FORBIDDEN_FILE
fi

#build forbidden sub directory
if [ ! -d "$FORBIDDEN_SUBDIR" ]; then
	mkdir -p $FORBIDDEN_SUBDIR
	cp $METHOD_SAMPLE_FILE $FORBIDDEN_SUBDIR'/sample.html'
	chmod a=r $FORBIDDEN_SUBDIR
	if [ "$AC_MAC" = "1" ]; then
		sudo chown nobody $FORBIDDEN_SUBDIR
	fi
fi
