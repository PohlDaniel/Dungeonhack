#
# Internal functions required by scripts to operate within Dungeonhack
# This file is automatically loaded at startup
#

import sys
sys.path.append("GameData/Scripts/Lib")

import random


# Define file name, required by SWIG (since we dont use execfile to load scripts)
__file__ = "dungeonhack.py"


# Exception used by script to exit current thread
# We use BaseException and not Exception in order to be sure it won't be catched
class ScriptExitException(BaseException):
    def __init__(self, value):
        self.value = value
    def __str__(self):
        return repr(self.value)


# Wrapper functions to raise script exit exception
def ExitScript():
    raise ScriptExitException("Script ended normally")
def AbortScript():
    raise ScriptExitException("Script was aborted")
