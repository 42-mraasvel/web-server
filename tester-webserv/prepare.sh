############# Settings ##########################
#directory
METHOD_DIR='./ServerRoot/Method'
FORBIDDEN_DIR=$METHOD_DIR'/forbidden'
GET_DIR=$METHOD_DIR'/Get'
POST_DIR=$METHOD_DIR'/Post'
DELETE_DIR=$METHOD_DIR'/Delete'

#content to be removed each run
TEMP_DIR=$POST_DIR'/upload'
TEMP_DIR+=' '$POST_DIR'/root'

#files
METHOD_SAMPLE_FILE=$GET_DIR'/sample.html'
FORBIDDEN_FILE=$FORBIDDEN_DIR'/forbidden.html'
FORBIDDEN_SUBDIR=$FORBIDDEN_DIR'/forbidden_dir'

####################################################

#remove temp
rm -rf $TEMP_DIR

#build for Method/Delete
cp $METHOD_SAMPLE_FILE $DELETE_DIR

#remove forbidden main directory
if [ -f "$FORBIDDEN_FILE" ]; then
	chmod 0755 $FORBIDDEN_FILE
fi
if [ -d "$FORBIDDEN_SUBDIR" ]; then
	chmod 0755 $FORBIDDEN_SUBDIR
fi
rm -rf $FORBIDEEN_DIR

#build forbidden main directory
mkdir $FORBIDDEN_DIR
#	build forbidden file
cp $METHOD_SAMPLE_FILE $FORBIDDEN_FILE
chmod 0111 $FORBIDDEN_FILE
#	build forbidden dir
mkdir $FORBIDDEN_SUBDIR
cp $METHOD_SAMPLE_FILE $FORBIDDEN_SUBDIR'/sample.html'
chmod 0444 $FORBIDDEN_SUBDIR
