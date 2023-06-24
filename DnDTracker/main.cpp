#include <iostream>
#include "scene.h"
#include <chrono>

int main(int argc, char* argv[]){
	std::cout << "hello world!" << std::endl;

	if(SDL_Init(SDL_INIT_VIDEO) < 0){
		std::cout << "failed to init sdl video" << std::endl;
		return 1;
	}
	
	Scene scene = Scene::getDefaultScene();
	
	scene.init();

	SDL_Event event;

	int fpsc = 0;

	std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());

	while(!scene.w.shouldQuit()){
		while(SDL_PollEvent(&event)){
			scene.handleEvent(event);
		}
		scene.updateGUI();
		scene.render();

		if(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()-ms.count() > 1000){
			ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
			std::cout << "fps: " << fpsc << std::endl;
			fpsc = 0;
		}
		fpsc++;
	}

	SDL_DestroyWindow(scene.w.window);
	SDL_Quit();

	

	

	return 0;
}



