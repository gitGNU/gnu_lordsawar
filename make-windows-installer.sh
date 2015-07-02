#!/bin/bash
# Copyright (C) 2015 Ben Asselstine
# this script is licensed under the terms of the GNU GPL version 3, or later.
#
# make a windows installer using NSIS and the zip file that gets generated 
# by running make-lordsawar.zip.sh

#let's make sure the lordsawar-windows.zip file is in place.
if [ !  -x ./make-windows-zip.sh ]; then
  if [ !  -x ./make-windows-zip.sh.in ]; then
    echo "Error: You need to run ./configure first."
  else
    echo "Error: run this script from the top level of the lordsawar src tree."
  fi
  exit 1
fi

if [ ! -f ./lordsawar-windows.zip ]; then
  ./make-lordsawar-zip.sh
fi
if [ ! -f ./lordsawar-windows.zip ]; then
  echo "Error: can't make the zip file."
  exit 1
fi

#okay, do we have NSIS?
makensis=`which makensis 2>/dev/null`
if [ "x$makensis" == "x" ]; then
  echo "Error: We need the makensis command.  sudo yum install mingw32-nsis"
  exit 1
fi
mktemp=`which mktemp 2>/dev/null`
if [ "x$mktemp" == "x" ]; then
  echo "Error: We need the mktemp command.  sudo yum install coreutils"
  exit 1
fi
find=`which find 2>/dev/null`
if [ "x$find" == "x" ]; then
  echo "Error: We need the mktemp command.  sudo yum install findutils"
  exit 1
fi
unzip=`which unzip 2>/dev/null`
if [ "x$unzip" == "x" ]; then
  echo "Error: We need the mktemp command.  sudo yum install unzip"
  exit 1
fi


#unzip the zip file and enumerate the contents
tmpdir=`mktemp -d /tmp/lordsawar-windows.XXXXX`
unzip -q -d $tmpdir lordsawar-windows.zip
if [ "x$?" != "x0" ]; then
  echo "Error: we had trouble extracting the zip file."
  if [ -d $tmpdir ]; then
    rm -rf $tmpdir
  fi
  exit 1
fi

orig=`pwd`
cd $tmpdir
dirlist=`mktemp /tmp/lordsawar-windows.XXXXXX`
find -maxdepth 2 -type d  > $dirlist
filelist=`mktemp /tmp/lordsawar-windows.XXXXXX`
find -maxdepth 2 -type f  > $filelist
cd $orig

nsiscript=`mktemp /tmp/lordsawar-windows.XXXXXX`
 
echo "Name \"LordsAWar!\"" >> $nsiscript
echo "OutFile \"lordsawar-setup.exe\"" >> $nsiscript
echo "InstallDir \"\$PROGRAMFILES32\\LordsAWar\"" >> $nsiscript
echo "Section \"install\"" >> $nsiscript
echo "  SetOutPath \"\$INSTDIR\"" >> $nsiscript

while IFS='' read -r line || [[ -n $line ]]; do
  echo -n "  File " >> $nsiscript
  echo $line | sed -e 's/^.\/lordsawar-windows\///g' >> $nsiscript
done < "$filelist"
echo "  CreateShortcut \"\$DESKTOP\\lordsawar.lnk\" \"\$INSTDIR\\lordsawar.exe\" \"\" \"\$INSTDIR\\various\\castle_icon.ico\"" >> $nsiscript
echo "  CreateShortcut \"\$DESKTOP\\lordsawar-editor.lnk\" \"\$INSTDIR\\lordsawar-editor.exe\" \"\" \"\$INSTDIR\\various\\tileset_icon.ico\"" >> $nsiscript

while IFS='' read -r line || [[ -n $line ]]; do
  if [ "x$line" == "x." ]; then
    continue
  fi
  if [ "x$line" == "x./lordsawar-windows" ]; then
    continue
  fi
  dir=`echo $line | sed -e 's/^.\/lordsawar-windows\///g'`
  echo "  File /r $dir" >> $nsiscript
done < "$dirlist"
echo "  WriteUninstaller \"\$INSTDIR\\Uninstall.exe\"" >> $nsiscript
echo "  ExecShell \"open\" \"\$Desktop\\lordsawar.lnk\"" >> $nsiscript
echo "SectionEnd" >> $nsiscript

echo "Section \"Uninstall\"" >> $nsiscript
echo "  RMDir /r \"\$INSTDIR\*.*\"" >> $nsiscript
echo "  RMDir \"\$INSTDIR\"" >> $nsiscript
echo "  delete \"\$DESKTOP\\lordsawar.lnk\"" >> $nsiscript
echo "  delete \"\$DESKTOP\\lordsawar-editor.lnk\"" >> $nsiscript
echo "  delete \"\$INSTDIR\\Uninstall.exe\"" >> $nsiscript
echo "SectionEnd" >> $nsiscript

echo "Generated this .nsi script:"
cat $nsiscript
echo "------"
echo "Please wait while we generate lordsawar-setup.exe..."

cd $tmpdir/lordsawar-windows
makensis -V0 -NOCD $nsiscript
cp $tmpdir/lordsawar-windows/lordsawar-setup.exe $orig
cd $orig
# cleanup
rm $dirlist
rm $filelist
if [ -d $tmpdir ]; then
  rm -rf $tmpdir
fi
rm $nsiscript
echo "Done."
