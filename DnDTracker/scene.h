#pragma once

#include <SDL.h>
#include <iostream>
#include <string>
#include <queue>

struct Window{
	SDL_Window* window;
	SDL_Renderer* renderer;

	int width;
	int height;

	std::string label;

	bool quit = false;

	Window(): window(NULL), width(), height() {};
	Window(int width, int height, std::string label){
		this->width = width;
		this->height = height;
		this->label = label;

	};

	int init(){
		this->window = SDL_CreateWindow(
			label.c_str(),
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			this->width,
			this->height,
			SDL_WINDOW_SHOWN
		);

		if(this->window == NULL){
			std::cout << "failed to create window" << std::endl;
			return 1;
		}

		this->renderer = SDL_CreateRenderer(this->window, -1, SDL_RENDERER_ACCELERATED);

		if(this->renderer == NULL){
			std::cout << "failed to create renderer" << std::endl;
		}

	}

	void handleEvent(SDL_Event event){

	}

	void render(){
		SDL_RenderPresent(this->renderer);
	}

	bool shouldQuit(){return this->quit;}

	~Window(){
		SDL_DestroyWindow(window);
	}
};

struct Scene{


};




