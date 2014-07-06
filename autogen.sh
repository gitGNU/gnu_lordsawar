#!/bin/sh
#This script sets up the codebase so that it can be configured.
autoreconf --verbose --install --force --symlink
gettextize -f
#echo "Running intltoolize"
intltoolize --copy --force --automake
echo "Now type './configure' to prepare LordsAWar! for compilation."
