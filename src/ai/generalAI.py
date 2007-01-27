#!/usr/bin/env python

# The generalAI shall gather the common functions for the different AI types

def giveCommand(str):
    f = open("aicommand", "w")
    f.write(str)

def lastCommand():
    f = open("aicommand", "r")
    return f.read()

# read the map (not yet saved by lordsawar)
def readMap():
    pass

def readCityList():
    pass

def readStackList():
    pass
