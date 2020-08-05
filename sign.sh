#!/bin/sh

# Run this after running deploy-build-mac.sh.

identity='Developer ID Application: Indiana University (5J69S77A7G)'
appname=RTcmixShell
qtversion=5_15_0
projectdir=/Users/johgibso/docs/development/Qt-mine/${appname}
entitlements=${projectdir}/Entitlements.plist

builddir=${projectdir}/../build-${appname}-Desktop_Qt_${qtversion}_clang_64bit-Release
app=${builddir}/${appname}.app
tool=codesign
#tool=echo

${tool} --sign "$identity" --deep --force --verbose --options runtime --entitlements ${entitlements} ${app}
${tool} --verify --verbose --deep --strict ${app}

