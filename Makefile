CXX		  := g++
CFLAGS := -Wall -lmingw32 -lSDL2main -std=c++17 -lSDL2

BIN		:= bin
SRC 		:= src

INCLUDE	:= include
INCLUDESDL := C:\sdl2_i686-w64-mingw32\include
LIBSDL := C:\sdl2_i686-w64-mingw32\lib

INCLUDESDL-ttf := C:\sdl2-ttf_i686-w64-mingw32\include
LIBSDL-ttf := C:\sdl2-ttf_i686-w64-mingw32\lib


EXECUTABLE	:= tetris


$(BIN)/$(EXECUTABLE): $(SRC)/*.cpp
	$(CXX) $^ -I$(INCLUDE) -I$(INCLUDESDL) -L$(LIBSDL) -I$(INCLUDESDL-ttf) -L$(LIBSDL-ttf) $(CFLAGS) -o $@

clean:
	del $(BIN)\$(EXECUTABLE).exe