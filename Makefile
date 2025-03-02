all:	build
	g++ src/main.cpp -o build/spectrograph -lSDL2
build:
	mkdir build
clean:
	rm -rf build