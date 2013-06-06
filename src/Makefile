# Project: SgribthMaker

BIN  = ../sgribthmaker
MAINSOURCE = SgribthMaker.cpp
all : $(BIN)
MODULES = \
 lib/script/script.a \
 lib/file/file.a \
 lib/math/math.a \
 lib/nix/nix.a \
 lib/image/image.a \
 lib/net/net.a \
 lib/hui/hui.a \
 lib/any/any.a \
 lib/algebra/algebra.a \
 lib/base/base.a
OBJ = lib/base/strings.o lib/file/msg.o lib/script/script.o \
 History.o CommandDialog.o SettingsDialog.o SourceView.o HighlightSchema.o \
 Parser/Parser.o Parser/ParserKaba.o Parser/ParserText.o \
 Parser/ParserC.o Parser/ParserShader.o
CPP  = g++
#GLOBALFLAGS = -O3
#GLOBALFLAGS = -g -O3
GLOBALFLAGS = -g
LIB_DIR = lib
CXXFLAGS = `pkg-config --cflags gtk+-3.0` $(GLOBALFLAGS)
CXXFLAGSLINKING =  `pkg-config --cflags --libs gtk+-3.0` -lGL -lGLU -lX11 -I/usr/include/AL -lopenal -lvorbis -lvorbisfile $(GLOBALFLAGS)


$(BIN) : $(MAINSOURCE) $(OBJ) $(MODULES)
	$(CPP) $(OBJ) $(MODULES) $(MAINSOURCE) -o $(BIN) $(CXXFLAGSLINKING)


History.o : History.cpp
	$(CPP) -c History.cpp -o $@ $(CXXFLAGS)

SettingsDialog.o : SettingsDialog.cpp
	$(CPP) -c SettingsDialog.cpp -o $@ $(CXXFLAGS)

CommandDialog.o : CommandDialog.cpp
	$(CPP) -c CommandDialog.cpp -o $@ $(CXXFLAGS)

HighlightSchema.o : HighlightSchema.cpp
	$(CPP) -c HighlightSchema.cpp -o $@ $(CXXFLAGS)

SourceView.o : SourceView.cpp
	$(CPP) -c SourceView.cpp -o $@ $(CXXFLAGS)

Parser/Parser.o : Parser/Parser.cpp
	$(CPP) -c Parser/Parser.cpp -o $@ $(CXXFLAGS)

Parser/ParserKaba.o : Parser/ParserKaba.cpp
	$(CPP) -c Parser/ParserKaba.cpp -o $@ $(CXXFLAGS)

Parser/ParserText.o : Parser/ParserText.cpp
	$(CPP) -c Parser/ParserText.cpp -o $@ $(CXXFLAGS)

Parser/ParserC.o : Parser/ParserC.cpp
	$(CPP) -c Parser/ParserC.cpp -o $@ $(CXXFLAGS)

Parser/ParserShader.o : Parser/ParserShader.cpp
	$(CPP) -c Parser/ParserShader.cpp -o $@ $(CXXFLAGS)

install : all
	mkdir -p /usr/share/sgribthmaker
	mkdir -p /usr/share/sgribthmaker/Data
	cp -f ../Data/hui_resources.txt /usr/share/sgribthmaker/Data
	cp -f ../Data/icon.ico             /usr/share/sgribthmaker/Data
	cp -f ../Data/license.txt          /usr/share/sgribthmaker/Data
	cp -f ../Data/license_small.txt    /usr/share/sgribthmaker/Data
	cp -f ../Data/SgribthMaker.png     /usr/share/sgribthmaker/Data
	xdg-mime install ../Data/michisoft-sgribthmaker.xml
	xdg-desktop-menu install ../Data/michisoft-sgribthmaker.desktop
	cp $(BIN) /usr/bin


include lib/base/base.mk
include lib/file/file.mk
include lib/hui/hui.mk
include lib/nix/nix.mk
include lib/image/image.mk
include lib/script/script.mk
include lib/net/net.mk
include lib/math/math.mk
include lib/any/any.mk
include lib/algebra/algebra.mk

clean :
	#rm -f $(OBJ) $(BIN)
	#rm -f temp/* $(BIN)
	rm -f $(BIN)
	find . -name "*.o" -exec rm -f {} \;
	find . -name "*.a" -exec rm -f {} \;
