#!/bin/bash
#
# upload.sh
# Upload translations to launchpad.
#
# Copyright (c) 2008 Pierre "delroth" Bourdon <root@delroth.is-a-geek.org>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

MODULES="freedroidrpg freedroidrpg-dialogs freedroidrpg-data"
LANGUAGES="fr de ru"

CURL=curl
CURLOPTS="-s -k"

read -p "Launchpad email: " login
read -s -p "Password: " password

echo ''

cookiefile=""
while [ -e /tmp/$cookiefile ]; do
    cookiefile=/tmp/$RANDOM.upcookies
done

$CURL $CURLOPTS "https://translations.launchpad.net/+login" \
    --data-urlencode "loginpage_email=$login" \
    --data-urlencode "loginpage_password=$password" \
    --data-urlencode "loginpage_submit_login=Log+In" \
    -c $cookiefile  >/dev/null

cookie="`tail -n 1 $cookiefile | awk '{ print $7; }'`"
rm $cookiefile

for module in $MODULES; do
    url="https://translations.launchpad.net/freedroid/trunk/+pots/$module/+upload"
    pot="`echo $module | tr - _`.pot"

    echo -n Uploading $pot...
    $CURL $CURLOPTS -b "lp=$cookie" "$url" \
        -F "file=@$pot;filename=$pot" \
        -F "UPLOAD=Upload" >/dev/null
    
    echo done

    if [ "$1" = --pot-only ]; then
	continue;
    fi

    for lang in $LANGUAGES; do
 	pofile="$lang"_`echo $lang | tr a-z A-Z`/LC_MESSAGES/`echo $module | tr - _`.po

	echo -n Uploading $pofile...
        $CURL $CURLOPTS -b "lp=$cookie" "$url" \
            -F "file=@$pofile;filename=$lang.po" \
            -F "UPLOAD=Upload" > /dev/null
	echo done
    done
done
