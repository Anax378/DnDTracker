#pragma once

#include <SDL.h>
#include <iostream>
#include <string>
#include <queue>
#include <filesystem>
#include "opencv2/opencv.hpp"

#define DEFAULT_MAP_FILE_NAME "default_map.png"
#define LOG(x) std::cout << x << std::endl;

struct Coord{
	float x;
	float y;
	Coord(): x(), y() {};
	Coord(float x, float y){
		this->x = x;
		this->y = y;
	}
};

struct CoordInt{
	int x;
	int y;
	CoordInt(): x(), y() {};
	CoordInt(int x, int y){
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
		window = SDL_CreateWindow(
			label.c_str(),
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			width,
			height,
			SDL_WINDOW_SHOWN
		);

		if(window == NULL){
			std::cout << "failed to create window" << std::endl;
			return 1;
		}

		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

		if(renderer == NULL){
			std::cout << "failed to create renderer" << std::endl;
			return 1;
		}
		return 0;
	}

	bool shouldQuit(){return quit;}
};

struct Camera{
	CoordInt position;
	int width;
	int height;

	int renderWidth;
	int renderHeight;

	Camera(): position(), width(), height(), renderWidth(), renderHeight() {};
	Camera(CoordInt position, int width, int height, int renderWidth, int renderHeight){
		this->position = position;
		this->width = width;
		this->height = height;
		this->renderWidth = renderWidth;
		this->renderHeight = renderHeight;
	}

	float getXScaleFactor(){
		return renderWidth/width;
	}
	float getYScaleFactor(){
		return renderHeight/height;
	}

	CoordInt toCameraCoordinates(CoordInt coord){
		float Xfactor = getXScaleFactor();
		float Yfactor = getYScaleFactor();
		return CoordInt(std::round((coord.x-position.x)*Xfactor),std::round((coord.y-position.y)*Yfactor));
	}

	CoordInt fromWindowCoordinates(CoordInt coord)
	{
		float Xfactor = getXScaleFactor();
		float Yfactor = getYScaleFactor();
		return CoordInt(std::round((coord.x/Xfactor) + position.x), std::round((coord.y/Yfactor) + position.y));
	}


	CoordInt fromCameraCordinates(CoordInt coord){
		return CoordInt(coord.x+position.x, coord.y+position.y);

	}

	cv::Mat getBackgroundImage(cv::Mat image){
		cv::Mat ret;
		cv::Mat im = image(cv::Rect(position.x, position.y, width, height));
		cv::resize(im, ret, cv::Size(renderWidth,renderHeight), 0, 0);
		return ret;
	}

};

struct Scene{
	Window w;
	Camera camera;
	cv::Mat backgroundImage;
	CoordInt mousePosition = CoordInt(0, 0);
	CoordInt mouseLeftDownPosition = CoordInt(0, 0);
	CoordInt mouseLeftDownCameraPosition = CoordInt(0, 0);

	bool isMouseLeftDown = false;

	SDL_Texture* texture;
	
	Scene(): backgroundImage(), w(), camera() {};
	Scene(cv::Mat image, Window w, Camera camera){
		this->backgroundImage = image;
		this->w = w;
		this->camera = camera;
	}

	static cv::Mat generateNoiseImage(int width, int height) {
		cv::Mat noiseImage(height, width, CV_8UC3);

		cv::RNG rng;

		for (int y = 0; y < height; ++y) {
			for (int x = 0; x < width; ++x) {
				cv::Vec3b& pixel = noiseImage.at<cv::Vec3b>(y, x);
				pixel[0] = rng.uniform(0, 256);
				pixel[1] = rng.uniform(0, 256);
				pixel[2] = rng.uniform(0, 256);
			}
		}

		return noiseImage;
	}

	static cv::Mat readDefaultImage(){

		std::cout << std::filesystem::exists(DEFAULT_MAP_FILE_NAME) << std::endl;
		cv::Mat im = cv::imread(DEFAULT_MAP_FILE_NAME);
		//cv::Mat im = Scene::generateNoiseImage(1000, 1000);
		return im;
	}

	int init(){
		int res = w.init();
		texture = SDL_CreateTexture(w.renderer, SDL_PIXELFORMAT_BGR24, SDL_TEXTUREACCESS_STATIC, camera.renderWidth, camera.renderHeight);
		return res;
	}

	void handleEvent(SDL_Event event){
		if(event.type == SDL_QUIT){
			w.quit = true;
		}

		if(event.type == SDL_MOUSEBUTTONDOWN){
			if(event.button.button == SDL_BUTTON_LEFT){
				isMouseLeftDown = true;
				SDL_GetMouseState(
					&(mouseLeftDownPosition.x),
					&(mouseLeftDownPosition.y));
				mouseLeftDownCameraPosition = camera.position;
				SDL_GetMouseState(
					&(mousePosition.x),
					&(mousePosition.y));
			}
		}
		if(event.type == SDL_MOUSEBUTTONUP){
			if(event.button.button == SDL_BUTTON_LEFT){isMouseLeftDown = false;}
		}
		if(isMouseLeftDown){
			if(event.type == SDL_MOUSEMOTION){
				SDL_GetMouseState(
					&(mousePosition.x),
					&(mousePosition.y));
//				std::cout << "x: " << mousePosition.x << " y: " << mousePosition.y << std::endl;
				
			}
		}

	}

	void renderBackground(SDL_Renderer* renderer){
		cv::Mat image = camera.getBackgroundImage(backgroundImage);
		SDL_UpdateTexture(texture, NULL, image.data, image.cols*3);
		SDL_RenderCopy(renderer, texture, NULL, NULL);

	}

	void zoom(float factor){
		changeCameraResolution(
			std::round((float)camera.width*factor),
			std::round((float)camera.height*factor)
		);

	}

	void changeCameraResolution(int width, int height){
		int widthChange = width-camera.width;
		int heightChange = height-camera.height;
		camera.width = width;
		camera.height = height;
		camera.position.x += std::round((float)widthChange/2.0f);
		camera.position.y += std::round((float)heightChange/2.0f);
	}

	void clipCamera(){
		if(camera.position.x < 0){camera.position.x = 0;}
		if(camera.position.y < 0){camera.position.y = 0;}

		if(camera.width > backgroundImage.cols){camera.width = backgroundImage.cols;}
		if(camera.height > backgroundImage.rows){camera.height = backgroundImage.rows;}

		if(camera.position.x+camera.width > backgroundImage.cols){
			camera.position.x -= camera.position.x+camera.width-backgroundImage.cols;
		}
		if(camera.position.y+camera.height > backgroundImage.rows){
			camera.position.y -= camera.position.y+camera.height-backgroundImage.rows;
		}
	}

	void render(){
		renderBackground(w.renderer);
		SDL_RenderPresent(w.renderer);
	}

	void changeOutputResolution(int width, int height){
		camera.renderWidth = width;
		camera.renderHeight = height;
		delete texture;
		texture = SDL_CreateTexture(w.renderer, SDL_PIXELFORMAT_BGR24, SDL_TEXTUREACCESS_STATIC, camera.renderWidth, camera.renderHeight);
	}

	void updateGUI(){
		if(isMouseLeftDown){

			camera.position.x = mouseLeftDownCameraPosition.x - (camera.toCameraCoordinates( mousePosition).x-camera.toCameraCoordinates(mouseLeftDownPosition).x);
			camera.position.y = mouseLeftDownCameraPosition.y - (camera.toCameraCoordinates( mousePosition).y-camera.toCameraCoordinates(mouseLeftDownPosition).y);
		}
		clipCamera();
	}
};
