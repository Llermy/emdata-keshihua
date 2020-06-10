run: main.cpp glad.c shader.cpp camera.cpp ComputeShaderManager.cpp
	g++ -g -o run -std=c++11 -I /usr/local/include -I include -L /usr/local/lib main.cpp glad.c shader.cpp camera.cpp ComputeShaderManager.cpp -lXrandr -lXrender -lXi -lXfixes -lXxf86vm -lXext -lglfw3 -lrt -lm -ldl -lX11 -lpthread -lxcb -lXau -lXdmcp
