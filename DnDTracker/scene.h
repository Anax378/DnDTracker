#pragma once

#include <SDL.h>
#include <iostream>
#include <string>
#include <queue>
#include <filesystem>
#include "opencv2/opencv.hpp"

#define DEFAULT_MAP_FILE_NAME "Assets\\default_map.png"
#define LOG(x) std::cout << x << std::endl;
#define ICON_RES 50

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

struct Color{
	unsigned char r;
	unsigned char g;
	unsigned char b;
	Color(): r(), g(), b() {};
	Color(unsigned char r, unsigned char g, unsigned char b){
		this->r = r;
		this->g = g;
		this->b = b;
	}
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

struct GuiToggleComponent{
	CoordInt position;

	Color color;
	Color hoveringColor;
	Color selectedColor;
	cv::Mat* image;
	bool isToggled = false;

	GuiToggleComponent(): position(), color(), hoveringColor(), selectedColor(), image() {}
	GuiToggleComponent(CoordInt position, Color color, Color hoveringColor, Color selectedColor, cv::Mat* image){
		this->position = position;
		this->color = color;
		this->hoveringColor = hoveringColor;
		this->selectedColor = selectedColor;
		this->image = image;
	}

	bool isInside(CoordInt pos){
		if(pos.x < position.x){return false;}
		if(pos.y < position.y){return false;}
		if(pos.x > position.x+ICON_RES){return false;}
		if(pos.y > position.y+ICON_RES){return false;}
		return true;
	}

	void render(SDL_Renderer* renderer, SDL_Texture* texture, bool isHovering){
		SDL_UpdateTexture(texture, NULL, (*image).data,(*image).cols * (*image).channels());
		SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
		SDL_Rect rect = {position.x, position.y, ICON_RES, ICON_RES};
		if(isHovering){
			SDL_SetTextureColorMod(texture, hoveringColor.r, hoveringColor.g, hoveringColor.b);
		}
		else if(isToggled){
			SDL_SetTextureColorMod(texture, selectedColor.r, selectedColor.g, selectedColor.b);
		}
		else{
			SDL_SetTextureColorMod(texture, color.r, color.g, color.b);
		}
		SDL_RenderCopy(renderer, texture, NULL, &rect);
		SDL_SetTextureColorMod(texture, 255, 255, 255);
	}
	void toggle(){
		isToggled = !isToggled;
	}

};

struct Marker{
	CoordInt position;
	Color color;
	Color labelColor;
	std::string label;
	cv::Mat* icon;
	int hitbox_size;

	Marker(): position(), color(), labelColor(), icon(), label() {};
	Marker(CoordInt position, Color color, Color labelColor, std::string label, cv::Mat* icon, int hitbox_size){
		this->position = position;
		this->color = color;
		this->labelColor = labelColor;
		this->label = label;
		this->icon = icon;
		this->hitbox_size = hitbox_size;
	}

	bool isVisible(Camera* camera){
		return true;

		//TODO: do not render unseen markers;
	}

	bool isInside(CoordInt pos, Camera* camera){
		CoordInt screenPos = camera->toCameraCoordinates(position);
		if(screenPos.x < pos.x - hitbox_size){return false;}
		if(screenPos.y < pos.y - hitbox_size){return false;}
		if(screenPos.x > pos.x + hitbox_size){return false;}
		if(screenPos.y > pos.y + hitbox_size){return false;}
		return true;
	}

	void render(SDL_Renderer* renderer, SDL_Texture* texture, Camera* camera){
		SDL_UpdateTexture(texture, NULL, (*icon).data, (*icon).cols * (*icon).channels());
		CoordInt renderPos = (*camera).toCameraCoordinates(position);
		SDL_Rect rect = {renderPos.x - (ICON_RES/2), renderPos.y-(ICON_RES/2), ICON_RES, ICON_RES};
		SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
		SDL_RenderCopy(renderer, texture, NULL, &rect);
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

	bool isUnhandledMouseClick = false;
	bool isMarkerSelected = false;
	Marker* selectedMarker;

	float zoomFactor = 1.0f;

	GuiToggleComponent markerToggle;

	SDL_Texture* texture;
	SDL_Texture* iconTexture;

	std::vector<cv::Mat> icons = {};
	std::vector<Marker> markers = {};
	
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
		loadIcons();
		int res = w.init();
		texture = SDL_CreateTexture(w.renderer, SDL_PIXELFORMAT_BGR24, SDL_TEXTUREACCESS_STATIC, camera.renderWidth, camera.renderHeight);
		iconTexture = SDL_CreateTexture(w.renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, ICON_RES, ICON_RES);

		markerToggle = GuiToggleComponent(CoordInt(0, 0), Color(0, 255, 0), Color(255, 255, 255), Color(255, 0, 0), &(icons.at(1)));
		return res;
	}

	void loadIcons(){
		icons.push_back(cv::imread("Assets\\Icon1.png", cv::IMREAD_UNCHANGED));
		icons.push_back(cv::imread("Assets\\marker_toggle.png", cv::IMREAD_UNCHANGED));
	}

	void addMarker(Marker marker){
		markers.push_back(marker);
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
				isUnhandledMouseClick = true;
				SDL_GetMouseState(
					&(mouseLeftDownPosition.x),
					&(mouseLeftDownPosition.y));
				mouseLeftDownCameraPosition = camera.position;
				SDL_GetMouseState(
					&(mousePosition.x),
					&(mousePosition.y));
				for(int i = 0; i < markers.size(); i++){
					if(markers.at(i).isInside(mousePosition, &camera)){
						isMarkerSelected = true;
						selectedMarker = &(markers.at(i));
						break;
					}
				}
				
			}
		}
		if(event.type == SDL_MOUSEBUTTONUP){
			if(event.button.button == SDL_BUTTON_LEFT){isMouseLeftDown = false;}
			isMarkerSelected = false;
		}
		if(event.type == SDL_MOUSEMOTION){
			SDL_GetMouseState(
				&(mousePosition.x),
				&(mousePosition.y));
				
		}
		if(event.type == SDL_KEYDOWN){
			if(event.key.keysym.sym == SDLK_DELETE){
				if(!isMarkerSelected){
					for(int i = 0; i < markers.size(); i++){
						if(markers.at(i).isInside(mousePosition, &camera)){
							markers.erase(markers.begin()+i);
						}
					}
				}
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

	void renderMarkers(SDL_Renderer* renderer){
		for(int i = 0; i < markers.size(); i++){
			if(markers.at(i).isVisible(&camera)){
				markers.at(i).render(renderer, iconTexture, &camera);
			}
		}
	}

	void zoomToFactor(float factor){
		changeCameraResolution(
			std::round((float)baseZoomCameraWidth*factor),
			std::round((float)baseZoomCameraHeight*factor)
		);

	}

	void changeCameraResolution(int width, int height){
		int xmovement = std::round((float)(width-camera.width)/2.0f);
		int ymovement = std::round((float)(height-camera.height)/2.0f);
		camera.width = width;
		camera.height = height;
		camera.position.x -= xmovement;
		camera.position.y -= ymovement;
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
		renderMarkers(w.renderer);
		markerToggle.render(w.renderer, iconTexture, markerToggle.isInside(mousePosition));
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

		if(isUnhandledMouseClick && markerToggle.isInside(mousePosition)){
			markerToggle.toggle();
			isUnhandledMouseClick = false;
			isMouseLeftDown = false;
		}

		if(markerToggle.isToggled && isUnhandledMouseClick){
			markers.push_back(Marker(camera.fromCameraCoordinates(mousePosition), Color(255, 255, 255), Color(0, 0, 0), std::string("new Marker"), &(icons.at(0)), 25));
			isMouseLeftDown = false;
			isUnhandledMouseClick = false;
			markerToggle.toggle();
		}
		
		if(isMarkerSelected){
			(*selectedMarker).position = camera.fromCameraCoordinates(mousePosition);
			isUnhandledMouseClick = false;
		}

		if(isMouseLeftDown && !isMarkerSelected){
			camera.position.x = mouseLeftDownCameraPosition.x - (camera.scaleFromCameraCoordinates(mousePosition).x-camera.scaleFromCameraCoordinates(mouseLeftDownPosition).x);
			camera.position.y = mouseLeftDownCameraPosition.y - (camera.scaleFromCameraCoordinates(mousePosition).y-camera.scaleFromCameraCoordinates(mouseLeftDownPosition).y);
			isUnhandledMouseClick = false;
		}
		clipCamera();
	}
};
