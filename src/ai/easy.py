#!/usr/bin/env python

from generalAI import *

if lastCommand() != "MOVE":
    giveCommand("MOVE")
else:
    giveCommand("ENDTURN")

