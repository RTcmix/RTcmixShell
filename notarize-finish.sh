#!/bin/sh

# Run this after running notarize-start.sh.
# NB: some of this modeled on a similar script in Sonic
# Visualiser (sonicvisualiser.org)

user="johgibso@gmail.com"
appname=RTcmixShell
version=`grep "define APP_VERSION_STR" main.cpp | sed 's/"//g' \
	| tr -d '\r\n' | awk '/define/ {print $3}'`
signeddir=${appname}-${version}-macOS-signed
app=${signeddir}/${appname}.app
uuidfile=.notarize-uuid

# If successful, this will return something like...
# 2020-08-05 20:06:43.775 altool[8571:13018300] No errors uploading 'RTcmixShell-1.0.9-macOS-signed.zip'.
# RequestUUID = 3b13db45-5171-48f3-9ecd-da34f4ecced3
# Not sure exactly what happens if this fails, but I assume the
# output file has no match for "RequestUUID".

# extract UUID from output of last command
uuid=$(cat "$uuidfile" | grep RequestUUID | awk '{ print $3; }')

if [ -z "$uuid" ]; then
	echo "Upload to Apple notarization service failed."
	exit 1
fi

# Once notarization is complete, I receive an email message about it.
# But here we check for this using altool.

statusfile=.notarize-status

while true ; do
	/usr/bin/xcrun altool --notarization-info \
		"$uuid" \
		--username "$user" \
		--password "@keychain:notary" 2>&1 | tee "$statusfile"

	if grep -q 'Package Approved' "$statusfile"; then
		echo
		echo "Notarization approved! Status output is:"
		cat "$statusfile"
		break
	elif grep -q 'in progress' "$statusfile"; then
		echo
		echo "Still in progress... Status output is:"
		cat "$statusfile"
		echo "Waiting..."
	else 
		echo
		echo "Failure or unknown status in output:"
		cat "$statusfile"
		exit 2
	fi
	sleep 30
done

# Output from a failed or successful --notarization-info run
# includes a URL # that provides detailed information about
# the approval.

# This "staples" the approval "ticket" to the zipped package.
# Unfortunately, this can't work with a zip or a regular folder.
# So we run it on the app bundle inside the folder, delete
# the zip we made earlier, and recreate it..

echo
echo "Stapling to app bundle and re-zipping..."

/usr/bin/xcrun stapler staple ${app}

/bin/rm -f ${signeddir}.zip
/usr/bin/ditto -c -k --keepParent ${signeddir} ${signeddir}.zip

