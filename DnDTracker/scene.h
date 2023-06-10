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

	CoordInt add(CoordInt other){
		return CoordInt(x+other.x, y+other.y);
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
			SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
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
		return (float)renderWidth/(float)width;
	}
	float getYScaleFactor(){
		return (float)renderHeight/(float)height;
	}


	CoordInt toCameraCoordinates(CoordInt coord){
		float Xfactor = getXScaleFactor();
		float Yfactor = getYScaleFactor();
		return CoordInt(std::round(((float)coord.x-position.x)*Xfactor),std::round(((float)coord.y-position.y)*Yfactor));
	}

	CoordInt fromCameraCoordinates(CoordInt coord)
	{
		float Xfactor = getXScaleFactor();
		float Yfactor = getYScaleFactor();
		return CoordInt(std::round(((float)coord.x/Xfactor) + position.x), std::round(((float)coord.y/Yfactor) + position.y));
	}

	CoordInt scaleToCameraCoordinates(CoordInt coord){
		float Xfactor = getXScaleFactor();
		float Yfactor = getYScaleFactor();
		return CoordInt(std::round(((float)coord.x)*Xfactor),std::round(((float)coord.y)*Yfactor));
	}

	CoordInt scaleFromCameraCoordinates(CoordInt coord)
	{
		float Xfactor = getXScaleFactor();
		float Yfactor = getYScaleFactor();
		return CoordInt(std::round(((float)coord.x/Xfactor)), std::round(((float)coord.y/Yfactor)));
	}

	cv::Mat getBackgroundImage(cv::Mat image){
		cv::Mat ret;
		cv::Mat im = image(cv::Rect(position.x, position.y, width, height));
		cv::resize(im, ret, cv::Size(renderWidth,renderHeight), 0, 0);
		return ret;
	}

};

struct Scene{
	float zoomSpeed = -0.1f;

	Window w;
	Camera camera;
	cv::Mat backgroundImage;
	CoordInt mousePosition = CoordInt(0, 0);
	CoordInt mouseLeftDownPosition = CoordInt(0, 0);
	CoordInt mouseLeftDownCameraPosition = CoordInt(0, 0);

	int toScroll = 0;
	int originalCameraWidth;
	int originalCameraHeight;

	int baseZoomCameraWidth;
	int baseZoomCameraHeight;

	bool isMouseLeftDown = false;
	bool changedWindowSize = false;

	float zoomFactor = 1.0f;

	SDL_Texture* texture;
	
	Scene(): backgroundImage(), w(), camera() {};
	Scene(cv::Mat image, Window w, Camera camera){
		this->backgroundImage = image;
		this->w = w;
		this->camera = camera;
		this->originalCameraWidth = camera.width;
		this->originalCameraHeight = camera.height;

		this->baseZoomCameraWidth = camera.width;
		this->baseZoomCameraHeight = camera.height;
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

	void alignCameraAspectRatio(){
		float factor = std::sqrt(((float)originalCameraWidth*(float)originalCameraHeight)/((float)camera.renderWidth*(float)camera.renderHeight));
		baseZoomCameraWidth = std::round((float)camera.renderWidth*factor);
		baseZoomCameraHeight = std::round((float)camera.renderHeight*factor);
		
	}

	void handleEvent(SDL_Event event){
		if(event.type == SDL_QUIT){
			w.quit = true;
		}

		if(event.type == SDL_WINDOWEVENT){
			if(event.window.event == SDL_WINDOWEVENT_RESIZED){
				changeOutputResolution(event.window.data1, event.window.data2);
				changedWindowSize = true;
			}
		}

		if(event.type == SDL_MOUSEWHEEL){
			toScroll += event.wheel.y;
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
				
			}
		}

	}

	float getMaxZoomFactor(){
		return std::min(
			(float)backgroundImage.cols/(float)baseZoomCameraWidth,
			(float)backgroundImage.rows/(float)baseZoomCameraHeight
		);
	}

	void renderBackground(SDL_Renderer* renderer){
		cv::Mat image = camera.getBackgroundImage(backgroundImage);
		SDL_UpdateTexture(texture, NULL, image.data, image.cols*3);
		SDL_RenderCopy(renderer, texture, NULL, NULL);

	}

	void zoomToFactor(float factor){
		changeCameraResolution(
			std::round((float)baseZoomCameraWidth*factor),
			std::round((float)baseZoomCameraHeight*factor)
		);

	}

	void changeCameraResolution(int width, int height){
		float xcenter = (float)camera.width/2.0f;
		float ycenter = (float)camera.height/2.0f;
		camera.width = width;
		camera.height = height;
		float newxcenter = (float)camera.width/2.0f;
		float newycenter = (float)camera.height/2.0f;

		camera.position = camera.position.add(CoordInt(newxcenter-xcenter, newycenter-ycenter));
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
		SDL_DestroyTexture(texture);
		texture = SDL_CreateTexture(w.renderer, SDL_PIXELFORMAT_BGR24, SDL_TEXTUREACCESS_STATIC, camera.renderWidth, camera.renderHeight);
	}

	void updateGUI(){

		if(changedWindowSize){
			alignCameraAspectRatio();
			changedWindowSize = false;
		}
		

		zoomFactor += ((float)toScroll)*zoomSpeed*zoomFactor;
		if(zoomFactor <= 0.01){
			zoomFactor = 0.01;
		}
		if(zoomFactor > getMaxZoomFactor()){
			zoomFactor = getMaxZoomFactor();
		}
		zoomToFactor(zoomFactor);
		toScroll = 0;
		if(isMouseLeftDown){
			camera.position.x = mouseLeftDownCameraPosition.x - (camera.scaleFromCameraCoordinates(mousePosition).x-camera.scaleFromCameraCoordinates(mouseLeftDownPosition).x);
			camera.position.y = mouseLeftDownCameraPosition.y - (camera.scaleFromCameraCoordinates(mousePosition).y-camera.scaleFromCameraCoordinates(mouseLeftDownPosition).y);
		}
		clipCamera();
	}
};
