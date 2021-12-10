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

#build all Method directory
mkdir -p $DELETE_DIR
mkdir -p $FORBIDDEN_DIR
mkdir -p $POST_DIR

#build for Method/Delete
cp $METHOD_SAMPLE_FILE $DELETE_DIR

#(re)build forbidden file
if [ -f "$FORBIDDEN_FILE" ]; then
	chmod 755 $FORBIDDEN_FILE
	rm -f $FORBIDDEN_FILE
fi
cp $METHOD_SAMPLE_FILE $FORBIDDEN_FILE
chmod a=r $FORBIDDEN_FILE

#(re)build forbidden sub directory
if [ -d "$FORBIDDEN_SUBDIR" ]; then
	sudo chown $USER $FORBIDDEN_SUBDIR
	chmod 755 $FORBIDDEN_SUBDIR
	rm -r $FORBIDDEN_SUBDIR
fi
mkdir -p $FORBIDDEN_SUBDIR
cp $METHOD_SAMPLE_FILE $FORBIDDEN_SUBDIR'/sample.html'
chmod a=r $FORBIDDEN_SUBDIR
sudo chown nobody $FORBIDDEN_SUBDIR
