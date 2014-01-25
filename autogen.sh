#! /bin/sh

AUTORECONF=`which autoreconf`
if test -z $AUTORECONF; then
	echo "*** No autoreconf found, please install it ***"
	exit 1
fi

rm -rf autom4te.cache

autoreconf --install --verbose --force || exit $?

echo ""
echo "------------------------------------------------------------"
echo "autogen.sh ran successfully. Execute ./configure to proceed."
