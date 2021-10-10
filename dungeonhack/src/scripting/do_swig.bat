@echo off

del dungeonhack.py
del PyPrototypes_wrap.cxx

swig -python -c++ PyPrototypes.i
move /y dungeonhack.py ..\..\GameData\Scripts\

pause