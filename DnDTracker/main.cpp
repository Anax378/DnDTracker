#include <iostream>
#include "scene.h"
#include <chrono>

int main(int argc, char* argv[]){

	if(SDL_Init(SDL_INIT_VIDEO) < 0){
		std::cout << "failed to init sdl video" << std::endl;
		return 1;
	}

	std::filesystem::path execDir = std::filesystem::absolute(std::filesystem::path(argv[0])).remove_filename();

	Scene scene;

	bool foundScene = false;

	std::cout << "argc: " << argc << std::endl;

	if(argc >= 2){
		std::cout << "here" << std::endl;
		std::string path(argv[1]);
		std::cout << path << std::endl;
		if(std::filesystem::exists(path)){
			scene = Scene::sceneFromFile(path);
			foundScene = true;
		}
	}

	if(!foundScene){
		scene = Scene::getDefaultScene();	
	}

	LOG("Here 12312");


	LOG("Here 44444");
	scene.init(execDir.string());
	LOG("Here 3333");
	SDL_Event event;

	int fpsc = 0;

	std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
	LOG("Here 44");
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



