@echo off
setlocal

set "PROJ_DIR=%~dp0"

echo Compiling for Windows in %PROJ_DIR%...

set "SRC_DIR=%PROJ_DIR%src"
set "OUT_DIR=%PROJ_DIR%"

echo Compiling server.cpp...
g++ -std=c++11 -Wall -I"%SRC_DIR%" -o "%OUT_DIR%server.exe" "%SRC_DIR%\server.cpp" "%SRC_DIR%\des.cpp" -lws2_32 -lpthread

echo Compiling client.cpp...
g++ -std=c++11 -Wall -I"%SRC_DIR%" -o "%OUT_DIR%client.exe" "%SRC_DIR%\client.cpp" "%SRC_DIR%\des.cpp" -lws2_32 -lpthread

echo Done.
endlocal
