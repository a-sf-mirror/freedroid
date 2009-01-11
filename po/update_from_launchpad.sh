#!/bin/bash

TMPDIR=`mktemp -d`

tar zxf /tmp/launchpad-export.tar.gz -C $TMPDIR

echo 'Uncompressed export in ' $TMPDIR

rm -rf code
rm -rf data

cp $TMPDIR/dialogs/freedroidrpg-dialogs-fr.po po/fr_FR/LC_MESSAGES/freedroidrpg_dialogs.po
cp $TMPDIR/dialogs/freedroidrpg-dialogs-de.po po/de_DE/LC_MESSAGES/freedroidrpg_dialogs.po
cp $TMPDIR/dialogs/freedroidrpg-dialogs-ru.po po/ru_RU/LC_MESSAGES/freedroidrpg_dialogs.po
cp $TMPDIR/freedroidrpg-dialogs-sv.po po/sv_SE/LC_MESSAGES/freedroidrpg_dialogs.po

rm -rf dialogs

cp $TMPDIR/freedroidrpg-data/freedroidrpg-data-de.po po/de_DE/LC_MESSAGES/freedroidrpg_data.po
cp $TMPDIR/freedroidrpg-data/freedroidrpg-data-fr.po po/fr_FR/LC_MESSAGES/freedroidrpg_data.po
cp $TMPDIR/freedroidrpg-data/freedroidrpg-data-sv.po po/sv_SE/LC_MESSAGES/freedroidrpg_data.po
cp $TMPDIR/freedroidrpg-data/freedroidrpg-data-ru.po po/ru_RU/LC_MESSAGES/freedroidrpg_data.po

rm -rf freedroidrpg-data

cp $TMPDIR/freedroidrpg-fr.po po/fr_FR/LC_MESSAGES/freedroidrpg.po
cp $TMPDIR/freedroidrpg-de.po po/de_DE/LC_MESSAGES/freedroidrpg.po
cp $TMPDIR/freedroidrpg-sv.po po/sv_SE/LC_MESSAGES/freedroidrpg.po
rm -rf $TMPDIR
