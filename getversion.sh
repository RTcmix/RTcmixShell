#!/bin/sh

# In main.cpp, the version number is declared on this line...
# #define APP_VERSION_STR             "1.0.9"
# We extract the line with grep, remove double quotes, strip newline, and
# store the 3rd column into $version.
version=`grep "define APP_VERSION_STR" main.cpp | sed 's/"//g' \
	| tr -d '\r\n' | awk '/define/ {print $3}'`
echo ${version}
echo "Check that version in Info.plist matches!"
