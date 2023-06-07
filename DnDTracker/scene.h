#pragma once

#include <SDL.h>
#include <iostream>
#include <string>
#include <queue>
#include <filesystem>
#include "opencv2/opencv.hpp"

#define DEFAULT_MAP_FILE_NAME "default_map.png"

struct Coord{
	float x;
	float y;
	Coord(): x(), y() {};
	Coord(float x, float y){
		this->x = x;
		this->y = y;
	}
};

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
			return 1;
		}
		return 0;
	}

	bool shouldQuit(){return this->quit;}
};

struct Camera{
	Coord position;
	int width;
	int height;

	int renderWidth;
	int renderHeight;

	Camera(): position(), width(), height(), renderWidth(), renderHeight() {};
	Camera(Coord position, int width, int height, int renderWidth, int renderHeight){
		this->position = position;
		this->width = width;
		this->height = height;
		this->renderWidth = renderWidth;
		this->renderHeight = renderHeight;
	}

	Coord toCameraCoordinates(Coord coord){
		return Coord(coord.x-this->position.x, coord.y-this->position.y);
	}

	cv::Mat getBackgroundImage(cv::Mat image){
		cv::Mat ret;
		cv::Mat im = image(cv::Rect(this->position.x, this->position.y, this->width, this->height));
		cv::resize(im, ret, cv::Size(renderWidth, renderHeight), 0, 0);
		return ret;
	}

};

struct Scene{
	Window w;
	Camera camera;
	cv::Mat backgroundImage;
	
	Scene(): backgroundImage(), w(), camera() {};
	Scene(cv::Mat image, Window w, Camera camera){
		this->backgroundImage = image;
		this->w = w;
		this->camera = camera;
	}

	static cv::Mat generateNoiseImage(int width, int height) {
		cv::Mat noiseImage(height, width, CV_8UC3); // Color image

		cv::RNG rng; // Random number generator

					 // Generate random noise for each pixel
		for (int y = 0; y < height; ++y) {
			for (int x = 0; x < width; ++x) {
				cv::Vec3b& pixel = noiseImage.at<cv::Vec3b>(y, x);
				pixel[0] = rng.uniform(0, 256); // Random blue component
				pixel[1] = rng.uniform(0, 256); // Random green component
				pixel[2] = rng.uniform(0, 256); // Random red component
			}
		}

		return noiseImage;
	}

	static cv::Mat readDefaultImage(){

		std::cout << std::filesystem::exists(DEFAULT_MAP_FILE_NAME) << std::endl;

		//cv::Mat im = cv::imread(DEFAULT_MAP_FILE_NAME);
		cv::Mat im = Scene::generateNoiseImage(1000, 1000);
		return im;
	}

	

	void handleEvent(SDL_Event event){

	}

	void renderBackground(SDL_Renderer* renderer){

		cv::Mat image = this->camera.getBackgroundImage(this->backgroundImage);
		SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(image.data, image.cols, image.rows, 24, image.cols*3, 0, 0, 0, 0);
		SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
		SDL_RenderCopy(renderer, texture, NULL, NULL);

	}

	void zoom(float factor){

	}

	void render(){
		renderBackground(this->w.renderer);
		SDL_RenderPresent(this->w.renderer);
	}

};
