#!/bin/sh

VICEVERSION=2.4.2

# see if we are in the top of the tree
if [ ! -f configure.in ]; then
  cd ../..
  if [ ! -f configure.in ]; then
    echo "please run this script from the base of the VICE directory"
    exit 1
  fi
fi

curdir=`pwd`

# set all cpu builds to no
armbuild=no
arm7abuild=no
mipsbuild=no
x86build=no

showusage=no

buildrelease=no
builddevrelease=no
builddebug=no

# check options
for i in $*
do
  if test x"$i" = "xarmeabi"; then
    armbuild=yes
  fi
  if test x"$i" = "xarmeabi-v7a"; then
    arm7abuild=yes
  fi
  if test x"$i" = "xmips"; then
    mipsbuild=yes
  fi
  if test x"$i" = "xx86"; then
    x86build=yes
  fi
  if test x"$i" = "xall-cpu"; then
    armbuild=yes
    arm7abuild=yes
    mipsbuild=yes
    x86build=yes
  fi
  if test x"$i" = "xhelp"; then
    showusage=yes
  fi
  if test x"$i" = "xrelease"; then
    buildrelease=yes
  fi
done

if test x"$showusage" = "xyes"; then
  echo "Usage: $0 [release] [<cpu types>] [help]"
  echo "release indicates that the binary needs to be build as a official release as opposed to a developent release."
  echo "cpu-types: armeabi, armeabi-v7a, mips, x86 (or all-cpu for all)."
  echo "if no cpu-type is given armeabi will be built by default."
  exit 1
fi

CPUS=""

if test x"$armbuild" = "xyes"; then
  CPUS="armeabi"
fi

if test x"$arm7abuild" = "xyes"; then
  if test x"$CPUS" = "x"; then
    CPUS="armeabi-v7a"
  else
    CPUS="$CPUS armeabi-v7a"
  fi
fi

if test x"$mipsbuild" = "xyes"; then
  if test x"$CPUS" = "x"; then
    CPUS="mips"
  else
    CPUS="$CPUS mips"
  fi
fi

if test x"$x86build" = "xyes"; then
  if test x"$CPUS" = "x"; then
    CPUS="x86"
  else
    CPUS="$CPUS x86"
  fi
fi

if test x"$CPUS" = "x"; then
  CPUS="armeabi"
fi

if test x"$CPUS" = "xarmeabi armeabi-v7a mips x86"; then
  CPULABEL="all"
else
  CPULABEL=$CPUS
fi

if test x"$buildrelease" = "xyes"; then
  if [ ! -f vice-release.keystore ]; then
    echo "vice-release.keystore not found, will fallback on a debug build"
    buildrelease=no
    builddebug=yes
  fi
else
  if [ ! -f vice-dev.keystore ]; then
    echo "vice-dev.keystore not found, will use a debug key instead"
    builddebug=yes
  else
    builddebug=no
    builddevrelease=yes
  fi
fi

cd src
echo generating src/translate_table.h
. ./gentranslatetable.sh <translate.txt >translate_table.h
echo generating src/translate.h
. ./gentranslate_h.sh <translate.txt >translate.h
echo generating src/infocontrib.h
. ./geninfocontrib_h.sh <../doc/vice.texi | sed -f infocontrib.sed >infocontrib.h
cd arch/android/AnVICE/jni
echo generating Application.mk
cp Application.mk.proto Application.mk
echo >>Application.mk "APP_ABI := $CPUS"
cd ..
echo building libvice.so
ndk-build

echo generating apk

if test x"$buildrelease" = "xyes"; then
  cp $curdir/vice-release.keystore ./
  echo >ant.properties "key.store=vice-release.keystore"
  echo >>ant.properties "key.alias=vice_release"
fi

if test x"$builddevrelease" = "xyes"; then
  cp $curdir/vice-dev.keystore ./
  echo >ant.properties "key.store=vice-dev.keystore"
  echo >>ant.properties "key.alias=vice_dev"
fi

if test x"$builddebug" = "xyes"; then
  rm -f ant.properties
  ant debug
  cd ../../../..
  mv src/arch/android/AnVICE/bin/PreConfig-debug.apk ./AnVICE-\($CPULABEL\)-$VICEVERSION.apk
else
  ant release
  rm -f vice-*.keystore
  rm -f ant.properties
  cd ../../../..
  mv src/arch/android/AnVICE/bin/PreConfig-release.apk ./AnVICE-\($CPULABEL\)-$VICEVERSION.apk
fi

if [ ! -f AnVICE-\($CPULABEL\)-$VICEVERSION.apk ]; then
  echo build not completed, check for errors in the output
else
  echo Android port binary generated as AnVICE-\($CPULABEL\)-$VICEVERSION.apk
fi
