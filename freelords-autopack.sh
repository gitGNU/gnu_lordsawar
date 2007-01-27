#!/bin/bash
#
# bash-script to create a CVS-Snapshot package and upload it to sf.net
# 
# Copyright 2001-2003 by Tobias Mathes, <tobi@freelords.org>
#
# ChangeLog at script end

# options
export DATE_STRING=$(date +%Y%m%d)			# Date  -> YYYYmmDD
export TAR_BIN=$(which tar)				# Where is tar?
export ZIP_BIN=$(which zip)				# where is zip?
export NCFTPPUT=$(which ncftpput)			# Where is ncftpput?

# compress options
export XPKG_BZ2=cfj
export XPKG_GZ=cfz
export XPKG_ZIP=-r9q

# filename options
export NPKG_BZ2=tar.bz2
export NPKG_GZ=tar.gz
export NPKG_ZIP=zip

# ftp options
export FTP_USER=anonymous
export FTP_PASS=release@freelords.org
export FTP_HOST=upload.sourceforge.net
export FTP_DIR=incoming

# script options
export FL_DIR=freelords_sdl
export FL_PREFIX=FreeLords
export FL_POSTFIX=CVS-SDL

export FLAP_VERSION=0.1.15


# colors
export FL_BLUEGREEN="\\033[44;33;1m"		# FG: BLUE, BG: GREEN
export FL_BLUEWHITE="\\033[44;37;1m"		# FG: BLUE, BG: WHITE
export FL_BLUEYELLOW="\\033[44;32;1m"		# FG: BLUE, BG: YELLOW

export FL_BLACKBLUE="\\033[40;34;1m"		# FG: BLACK, BG: BLUE
export FL_BLACKGREEN="\\033[40;32;1m"		# FG: BLACK, BG: GREEN
export FL_BLACKWHITE="\\033[40;37;1m"		# FG: BLACK, BG: WHITE
export FL_BLACKYELLOW="\\033[40;33;1m"		# FG: BLACK, BG: YELLOW

export FL_REDWHITE="\\033[41;37;1m"		# FG: RED, BG: WHITE
export FL_REDYELLOW="\\033[41;33;1m"		# FG: RED, BG: YELLOW


# MAIN SCRIPT ;)

# clear screen
clear
# print header 
printf "\n $FL_REDWHITE                                                                     $FL_BLACKWHITE\n$FL_BLACKWHITE"
printf " $FL_REDYELLOW -===>$FL_REDWHITE $FL_PREFIX CVS Snapshot Creation Script, Version: $FLAP_VERSION $FL_REDYELLOW<===- $FL_BLACKWHITE\n$FL_BLACKWHITE"
printf " $FL_REDWHITE                                                                     $FL_BLACKWHITE\n$FL_BLACKWHITE"
# creating copy of cvs path
printf "\n $FL_BLACKYELLOW-$FL_BLACKGREEN>$FL_BLACKBLUE Creating copy of directory.$FL_BLACKWHITE";
cp -R $FL_DIR $FL_PREFIX-$FL_POSTFIX-$DATE_STRING
printf "\t\t\t$FL_BLACKYELLOW<$FL_BLACKGREEN\aDONE$FL_BLACKYELLOW>$FL_BLACKWHITE";

# removing backup files "*~" and "CVS/" directories
printf "\n $FL_BLACKYELLOW-$FL_BLACKGREEN>$FL_BLACKBLUE Removing CVS directories and backup files.$FL_BLACKWHITE";
rm $FL_PREFIX-$FL_POSTFIX-$DATE_STRING/freelords-autopack.sh
find $FL_PREFIX-$FL_POSTFIX-$DATE_STRING -name CVS -type d | xargs rm -rf
find $FL_PREFIX-$FL_POSTFIX-$DATE_STRING -name \*~ -type f | xargs rm -rf
printf "\t$FL_BLACKYELLOW<$FL_BLACKGREEN\aDONE$FL_BLACKYELLOW>$FL_BLACKWHITE";

# building tarball ($NPKG_GZ)
printf "\n $FL_BLACKYELLOW-$FL_BLACKGREEN>$FL_BLACKBLUE Building tarball.$FL_BLACKWHITE";
$TAR_BIN $XPKG_GZ $FL_PREFIX-$FL_POSTFIX-$DATE_STRING.$NPKG_GZ $FL_PREFIX-$FL_POSTFIX-$DATE_STRING/
printf "\t\t\t\t$FL_BLACKYELLOW<$FL_BLACKGREEN\aDONE$FL_BLACKYELLOW>$FL_BLACKWHITE";

# building bzip2-ball ($NPKG_BZ2)
printf "\n $FL_BLACKYELLOW-$FL_BLACKGREEN>$FL_BLACKBLUE Building bzip2-ball.$FL_BLACKWHITE";
$TAR_BIN $XPKG_BZ2 $FL_PREFIX-$FL_POSTFIX-$DATE_STRING.$NPKG_BZ2 $FL_PREFIX-$FL_POSTFIX-$DATE_STRING/
printf "\t\t\t$FL_BLACKYELLOW<$FL_BLACKGREEN\aDONE$FL_BLACKYELLOW>$FL_BLACKWHITE";

# building zip-ball ($NPKG_ZIP)
printf "\n $FL_BLACKYELLOW-$FL_BLACKGREEN>$FL_BLACKBLUE Building zip-ball.$FL_BLACKWHITE";
$ZIP_BIN $XPKG_ZIP $FL_PREFIX-$FL_POSTFIX-$DATE_STRING.$NPKG_ZIP $FL_PREFIX-$FL_POSTFIX-$DATE_STRING/
printf "\t\t\t\t$FL_BLACKYELLOW<$FL_BLACKGREEN\aDONE$FL_BLACKYELLOW>$FL_BLACKWHITE";

# release @ sourceforge.net
printf "\n\n $FL_BLACKYELLOW-$FL_BLACKGREEN>$FL_BLACKBLUE Uploading packages to $FTP_HOST.$FL_BLACKWHITE\n\n$FL_REDWHITE===============================================================================\n$FL_REDYELLOW";
$NCFTPPUT -u $FTP_USER -p $FTP_PASS $FTP_HOST $FTP_DIR $FL_PREFIX-$FL_POSTFIX-$DATE_STRING.$NPKG_BZ2
$NCFTPPUT -u $FTP_USER -p $FTP_PASS $FTP_HOST $FTP_DIR $FL_PREFIX-$FL_POSTFIX-$DATE_STRING.$NPKG_GZ
$NCFTPPUT -u $FTP_USER -p $FTP_PASS $FTP_HOST $FTP_DIR $FL_PREFIX-$FL_POSTFIX-$DATE_STRING.$NPKG_ZIP
printf "$FL_REDWHITE===============================================================================\n$FL_BLACKWHITE";

# removing created, "temporary" release-files and directories
printf "\n $FL_BLACKYELLOW-$FL_BLACKGREEN>$FL_BLACKBLUE Removing directory and files.";
rm -Rf $FL_PREFIX-$FL_POSTFIX-$DATE_STRING/ $FL_PREFIX-$FL_POSTFIX-$DATE_STRING.tar.* $FL_PREFIX-$FL_POSTFIX-$DATE_STRING.zip
printf "\t\t$FL_BLACKYELLOW<$FL_BLACKGREEN\aDONE$FL_BLACKYELLOW>$FL_BLACKWHITE\n";
# print footer
printf "\n\n$FL_BLUEYELLOW -===$FL_BLUEGREEN#]$FL_BLUEYELLOW>$FL_BLUEWHITE Copyright & Code 2001-2003 by Tobias Mathes, <tobi@freelords.org> $FL_BLUEYELLOW<$FL_BLUEGREEN[#$FL_BLUEYELLOW===- \n$FL_BLACKWHITE";

# remove variables
unset DATE_STRING TARBIN ZIPBIN NCFTPPUT FL_DIR FL_PREFIX FL_POSTFIX FLAP_VERSION FTP_HOST FTP_USER FTP_PASS FTP_DIR NPKG_BZ2 NPKG_GZ NPKG_ZIP XPKG_BZ2 XPKG_GZ XPKG_ZIP FL_BLUEGREEN FL_BLUEWHITE FL_BLUEYELLOW FL_BLACKBLUE FL_BLACKGREEN FL_BLACKWHITE FL_BLACKYELLOE FL_REDWHITE FL_REDYELLOW
