run:
	g++ src/*.cpp main.cpp -lglfw -lGLX -lGL -lGLESv2 -lglapi -lGLU -o build/main && build/main
