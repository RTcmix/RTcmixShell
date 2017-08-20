# NB: if you get lots of error messages, it's probably because there is
# already an app bundle with some of these files in it. It's best to delete
# the app bundle, then build the project, then run this script.

appname=RTcmixShell
# for some reason, Qt Creator keeps wanting to write the location
# of rtcmix_embedded.dylib as the one in my RTcmix tree, instead
# of the one in RTcmixShell/lib.
exepath=/Users/johgibso/docs/development/Qt-mine/build-${appname}-Desktop_Qt_5_9_1_clang_64bit-Release/${appname}.app/Contents/MacOS/${appname}
install_name_tool -change /Users/johgibso/rtcmix/lib/librtcmix_embedded.dylib /Users/johgibso/docs/development/Qt-mine/${appname}/lib/librtcmix_embedded.dylib $exepath
cd ../build-${appname}-Desktop_Qt_5_9_1_clang_64bit-Release
macdeployqt ${appname}.app

# now dylibs are copied into the app bundle, but the reference to
# the rtcmix lib in the app is still wrong.
install_name_tool -change /Users/johgibso/docs/development/Qt-mine/${appname}/lib/librtcmix_embedded.dylib @executable_path/../Frameworks/librtcmix_embedded.dylib $exepath

# replace the Info.plist with our own
cp -f ../${appname}/Info.plist ${appname}.app/Contents/
