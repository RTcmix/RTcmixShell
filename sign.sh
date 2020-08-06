#!/bin/sh

# Run this after running deploy-build-mac.sh.

identity='Developer ID Application: Indiana University (5J69S77A7G)'
appname=RTcmixShell
qtversion=5_15_0
projectdir=/Users/johgibso/docs/development/Qt-mine/${appname}
entitlements=${projectdir}/Entitlements.plist
builddir=${projectdir}/../build-${appname}-Desktop_Qt_${qtversion}_clang_64bit-Release
version=`grep "define APP_VERSION_STR" main.cpp | sed 's/"//g' \
	| tr -d '\r\n' | awk '/define/ {print $3}'`
exportdir=${appname}-${version}-macOS
app=${exportdir}/${appname}.app
if [ -e ${exportdir} ]
then
	echo "Export dir `${exportdir}` already exists. Delete before running this."
	exit -1
fi
/bin/mkdir ${exportdir}
/usr/bin/ditto ${builddir}/${appname}.app ${app}
/usr/bin/ditto ChangeLog.txt ${exportdir}/ChangeLog.txt

tool=/usr/bin/codesign
#tool=echo
${tool} --sign "$identity" --deep --force --verbose --options runtime --entitlements ${entitlements} ${app}
${tool} --verify --verbose --deep --strict ${app}

# finally, make a zip
/usr/bin/ditto -c -k --keepParent ${exportdir} ${exportdir}.zip
