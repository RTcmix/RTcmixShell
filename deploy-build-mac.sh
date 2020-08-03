#!/bin/sh

# NB: if you get lots of error messages, it's probably because there is
# already an app bundle with some of these files in it. It's best to delete
# the app bundle, then build the project, then run this script.

appname=RTcmixShell
qtversion=5_15_0
projectdir=/Users/johgibso/docs/development/Qt-mine/${appname}

librtcmix=librtcmix_embedded.dylib
libportaudio=libportaudio.2.dylib
libsndfile=libsndfile.1.dylib

builddir=${projectdir}/../build-${appname}-Desktop_Qt_${qtversion}_clang_64bit-Release
mylibdir=${projectdir}/lib/mac
frameworksdir=${builddir}/${appname}.app/Contents/Frameworks
executable=${builddir}/${appname}.app/Contents/MacOS/${appname}

#echo ${builddir}
#echo ${mylibdir}
#echo ${frameworksdir}
#echo ${executable}
#exit 0

cd ${builddir}
macdeployqt ${appname}.app

# Copy our libs into the bundle.
cp ${mylibdir}/${librtcmix} ${frameworksdir}
cp ${mylibdir}/${libportaudio} ${frameworksdir}
cp ${mylibdir}/${libsndfile} ${frameworksdir}

# Fix references within the shared libs to their bundle-relative locations.
install_name_tool -id @executable_path/../Frameworks/${librtcmix} ${frameworksdir}/${librtcmix}
install_name_tool -id @executable_path/../Frameworks/${libportaudio} ${frameworksdir}/${libportaudio}
install_name_tool -id @executable_path/../Frameworks/${libsndfile} ${frameworksdir}/${libsndfile}

# Fix references to the shared libs from the app executable.
# (Yes, no change for portaudio, but could be different in future.)
install_name_tool -change /Users/johgibso/rtcmix/lib/${librtcmix} @executable_path/../Frameworks/${librtcmix} ${executable}
install_name_tool -change @executable_path/../Frameworks/${libportaudio} @executable_path/../Frameworks/${libportaudio} ${executable}
install_name_tool -change /usr/local/lib/${libsndfile} @executable_path/../Frameworks/${libsndfile} ${executable}

# Replace the Info.plist with our own.
cp -f ../${appname}/Info.plist ${appname}.app/Contents/
