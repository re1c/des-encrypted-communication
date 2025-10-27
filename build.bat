@echo off
echo Compiling for Windows...

echo Compiling server.cpp...
g++ -std=c++11 -Wall -Isrc -o server.exe src/server.cpp src/des.cpp -lws2_32

echo Compiling client.cpp...
g++ -std=c++11 -Wall -Isrc -o client.exe src/client.cpp src/des.cpp -lws2_32

echo Done.
