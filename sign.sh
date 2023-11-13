#!/bin/sh

# Run this after running deploy-build-mac.sh.

identity='Developer ID Application: Indiana University (5J69S77A7G)'
appname=RTcmixShell
qtversion=6_5_1
projectdir=/Users/johgibso/docs/development/Qt-mine/${appname}
entitlements=${projectdir}/Entitlements.plist
builddir=${projectdir}/../build-${appname}-Qt_${qtversion}_for_macOS-Release
version=`grep "define APP_VERSION_STR" main.cpp | sed 's/"//g' \
	| tr -d '\r\n' | awk '/define/ {print $3}'`
unsigneddir=${appname}-${version}-macOS
signeddir=${appname}-${version}-macOS-signed
if [ -e ${unsigneddir} ]; then
	echo "The dir for the unsigned version (`${unsigneddir}`) already exists."
	echo "Delete it before running this script."
	exit 1
fi
if [ -e ${signeddir} ]; then
	echo "The dir for the signed version (`${signeddir}`) already exists."
	echo "Delete it before running this script."
	exit 1
fi
if [ ! -e ${builddir}/${appname}.app/Contents/Frameworks ]; then
	echo "The Frameworks dir in the built app doesn't exist."
	echo "That means you haven't run 'deploy-build-mac.sh' yet."
	echo "Do that first, then try 'sign.sh' again."
	exit 1
fi

# Copy app from build dir into unsigned export dir, along with ChangeLog.txt,
# and zip the export dir.
/bin/mkdir ${unsigneddir}
/usr/bin/ditto ${builddir}/${appname}.app ${unsigneddir}/${appname}.app
/usr/bin/ditto ChangeLog.txt ${unsigneddir}/ChangeLog.txt
/usr/bin/ditto -c -k --keepParent ${unsigneddir} ${unsigneddir}.zip

# Copy app from build dir into the signed export dir, along with ChangeLog.txt.
/bin/mkdir ${signeddir}
/usr/bin/ditto ${builddir}/${appname}.app ${signeddir}/${appname}.app
/usr/bin/ditto ChangeLog.txt ${signeddir}/ChangeLog.txt

# Sign the app and its embedded libs, including the Qt-supplied ones.
tool=/usr/bin/codesign
#tool=echo
${tool} --sign "$identity" --deep --force --verbose --options runtime --entitlements ${entitlements} ${signeddir}/${appname}.app
${tool} --verify --verbose --deep --strict ${signeddir}/${appname}.app

