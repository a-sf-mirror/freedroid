#! /bin/sh

missing=0
for tool in autoreconf autopoint aclocal autoconf autoheader automake ; do
	file=`which $tool`
	if test -z $file ; then
		echo "*** No $tool found, please install it ***"
		missing=1
	fi
done
[ $missing -eq 1 ] && exit 1

rm -rf autom4te.cache

autoreconf --install --verbose --force || exit $?

echo ""
echo "------------------------------------------------------------"
echo "autogen.sh ran successfully. Execute ./configure to proceed."
