@echo off

set optimizations=
::/O2
mkdir build
pushd build
:loop
pwd
cl /Zi /FC /nologo %optimizations% /LD ..\position.cpp
cl /Zi /FC /nologo %optimizations% Dwmapi.lib User32.lib Gdi32.lib /DEBUG:FULL ..\payoff.cpp 
::&& payoff.exe
pause
clear
::GOTO loop
popd 
