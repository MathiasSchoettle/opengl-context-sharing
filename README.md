A minimal example of two opengl contexts sharing state.

A compute shader in the first opengl context updates a bound buffer. A second opengl context renders this buffer to the screen.

run it -> g++ main.cpp -lOpenGL -lglfw -lGLEW -lpthread -o main
