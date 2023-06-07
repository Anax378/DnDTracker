#include <iostream>;
#include "scene.h"

int main(int argc, char* argv[]){
	std::cout << "hello world!" << std::endl;

	if(SDL_Init(SDL_INIT_VIDEO) < 0){
		std::cout << "failed to init sdl video" << std::endl;
		return 1;
	}
	Window window = Window(500, 500, "TEST");
	SDL_Event event;

	while(!window.shouldQuit()){
		while(SDL_PollEvent(&event)){
			window.handleEvent(event);
		}
		window.render();
	}

	

	return 0;
}



