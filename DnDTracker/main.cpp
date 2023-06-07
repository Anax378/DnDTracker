#include <iostream>;
#include "scene.h"

int main(int argc, char* argv[]){
	std::cout << "hello world!" << std::endl;

	if(SDL_Init(SDL_INIT_VIDEO) < 0){
		std::cout << "failed to init sdl video" << std::endl;
		return 1;
	}
	Scene scene = Scene(Scene::readDefaultImage(), Window(500, 500, "TEST"), Camera(Coord(0, 0), 100, 100, 50, 50));

	int res = 0;
	res = scene.w.init();
	if(res != 0){
		return res;
	}
	SDL_Event event;

	while(!scene.w.shouldQuit()){
		while(SDL_PollEvent(&event)){
			scene.handleEvent(event);
			scene.render();
		}
	}

	

	return 0;
}



