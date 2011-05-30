#/bin/sh
#
# Convert an animations facing right to animations facing left.
#

DIR=$1
for APATH in $DIR/right/*; do
    ANIM=`basename $APATH`

    test -e $DIR/left/$ANIM && {
	echo Skipping $DIR/left/$ANIM
	continue
    }

    echo Creating $DIR/left/$ANIM
    mkdir -p $DIR/left/$ANIM
    cp $DIR/right/$ANIM/*.png $DIR/left/$ANIM/

    for PNG in $DIR/left/$ANIM/*.png; do
	convert $PNG -flop $PNG~
	mv $PNG~ $PNG
	cat $DIR/right/$ANIM/anim | sed 's/right/left'/g > $DIR/left/$ANIM/anim
    done

done
