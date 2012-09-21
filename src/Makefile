# Project: SgribthMaker

BIN  = ../sgribthmaker

all : $(BIN)

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
	sudo cp $(BIN) /usr/bin
