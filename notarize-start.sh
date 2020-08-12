#!/bin/sh

# Run this after running deploy-build-mac.sh and sign.sh.
# Be sure to run notarize-finish.sh after successful completion.
# NB: some of this modeled on a similar script in Sonic
# Visualiser (sonicvisualiser.org)

user="johgibso@gmail.com"
appname=RTcmixShell
bundleid="org.rtcmix.RTcmixShell"
version=`grep "define APP_VERSION_STR" main.cpp | sed 's/"//g' \
	| tr -d '\r\n' | awk '/define/ {print $3}'`
signeddir=${appname}-${version}-macOS-signed
if [ ! -e ${signeddir} ]; then
	echo "The dir for the signed version (`${signeddir}`) should already exist."
	echo "If it doesn't, maybe you haven't run 'sign.sh' yet."
	exit 1
fi

/usr/bin/ditto -c -k --keepParent ${signeddir} ${signeddir}.zip

uuidfile=.notarize-uuid

# This will block until the upload is complete.
/usr/bin/xcrun altool --notarize-app \
	--primary-bundle-id "$bundleid" \
	--username "$user" \
	--password "@keychain:notary" \
	--f ${signeddir}.zip 2>&1 | tee "$uuidfile"

# If successful, this will return something like...
# 2020-08-05 20:06:43.775 altool[8571:13018300] No errors uploading 'RTcmixShell-1.0.9-macOS-signed.zip'.
# RequestUUID = 3b13db45-5171-48f3-9ecd-da34f4ecced3
# Not sure exactly what happens if this fails, but I assume the
# output file has no match for "RequestUUID".

# extract UUID from output of last command
uuid=$(cat "$uuidfile" | grep RequestUUID | awk '{ print $3; }')

if [ -z "$uuid" ]; then
	echo "Upload to Apple notarization service failed."
else
	echo "Upload to Apple notarization service was successful (uuid: $uuid)."
fi

