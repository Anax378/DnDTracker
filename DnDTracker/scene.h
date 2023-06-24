#pragma once
#include "FileIO.h"
#include <SDL.h>
#include <iostream>
#include <string>
#include <queue>
#include <filesystem>
#include "opencv2/opencv.hpp"

#define DEFAULT_MAP_FILE_NAME "Assets\\default_map.png"
#define LOG(x) std::cout << x << std::endl;
#define ICON_RES 50
#define TEXT_WIDTH 200
#define TEXT_HEIGHT 200

#define MENU_OPTIONWIDTH 150
#define MENU_OPTIONHEIGHT 30

#define LINE_SPACE 5

#define TEXT_BOX_COLOR Color(0, 255, 0)

#define OPTION_DELETE 1
#define OPTION_RENAME 2
#define OPTION_ADD_LEVEL 3
#define OPTION_OPEN_LEVEL 4

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
		cv::resize(im, ret, cv::Size(renderWidth, renderHeight), 0, 0);
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

struct MenuOption{
	Color textColor;
	Color backgroundColor;

	cv::Mat textImage;
	bool isEnabled = true;

	std::string name;
	int width;
	int height;

	int optionType;

	MenuOption(): textColor(), backgroundColor(), name(), width(), height() {}
	MenuOption(int width, int height, std::string name, Color textColor, Color backgroundColor, int optionType,bool isEnabled = true){
		this->width = width;
		this->height = height;
		this->name = name;
		this->textColor = textColor;
		this->backgroundColor = backgroundColor;
		this->isEnabled = isEnabled;
		this->optionType = optionType;
		updateName(name);
	}

	void updateName(std::string name){
		this->name = name;
		textImage = cv::Mat(MENU_OPTIONHEIGHT, MENU_OPTIONWIDTH, CV_8UC4, cv::Scalar(0, 0, 0, 0));
		cv::putText(textImage, name, cv::Point(0, MENU_OPTIONHEIGHT-5), cv::FONT_HERSHEY_SIMPLEX, 0.75, cv::Scalar(255, 255, 255, 255), 2, 8, false);
	}

	bool isInsde(CoordInt position, CoordInt pos, int Yoffset){
		if(!isEnabled){return false;}
		if(pos.x < position.x){return false;}
		if(pos.y < position.y+Yoffset){return false;}
		if(pos.x > position.x+width){return false;}
		if(pos.y > position.y+Yoffset+height){return false;}
		return true;
	}

	void render(SDL_Renderer* renderer, SDL_Texture* texture, CoordInt position, int Yoffset){
		SDL_UpdateTexture(texture, NULL, textImage.data, textImage.cols * textImage.channels());
		SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
		SDL_Rect rect = {position.x, position.y+Yoffset, MENU_OPTIONWIDTH, MENU_OPTIONHEIGHT};
		SDL_SetRenderDrawColor(renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, 255);
		SDL_RenderFillRect(renderer, &rect);
		SDL_SetTextureColorMod(texture, textColor.r, textColor.g, textColor.b);
		SDL_RenderCopy(renderer, texture, NULL, &rect);
		SDL_SetTextureColorMod(texture, 255, 255, 255);
	}

};

struct LeftCLickMenu{
	CoordInt position;
	std::vector<MenuOption> options;

	LeftCLickMenu(): position() {}
	LeftCLickMenu(CoordInt position){
		this->position = position;
	}

	int getOption(CoordInt pos){

		int Yoffset = 0;
		for(int i = 0; i < options.size(); i++){
			if(options.at(i).isInsde(position, pos, Yoffset)){
				return i;
			}
			if(options.at(i).isEnabled){
				Yoffset += options.at(i).height;
			}
		}
		return -1;
	}

	void render(SDL_Renderer* renderer, SDL_Texture* texture){
		int Yoffset = 0;
		for(int i = 0; i < options.size(); i++){
			if(options.at(i).isEnabled){
				options.at(i).render(renderer, texture, position, Yoffset);
				Yoffset += options.at(i).height;
			}
		}
	}

	void addOption(MenuOption option){
		options.push_back(option);
	}

};

struct Marker{
	CoordInt position;
	Color color;
	Color labelColor;
	std::string label;
	int iconIndex;
	int hitbox_size;

	int levelLink = -1;

	cv::Mat textImage;

	Marker(): position(), color(), labelColor(), iconIndex(), label() {};
	Marker(CoordInt position, Color color, Color labelColor, std::string label, int iconIndex, int hitbox_size){
		this->position = position;
		this->color = color;
		this->labelColor = labelColor;
		this->iconIndex = iconIndex;
		this->hitbox_size = hitbox_size;

		UpdateLabel(label);
	}

	void UpdateLabel(std::string label){
		this->label = label;
		textImage = cv::Mat(TEXT_HEIGHT, TEXT_WIDTH, CV_8UC4, cv::Scalar(0, 0, 0, 0));

		std::vector<std::string> lines;
		std::string currentLine = "";

		cv::Size textSize;
		int baseline;
		int lastSplitIndex = 0;

		for(int i = 0; i < label.length(); i++){
			currentLine += label.at(i);
			textSize = cv::getTextSize(currentLine, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseline);

			if(textSize.width > TEXT_WIDTH || (i == 0 ? false : label.at(i-1) == '\n') ){
				if(currentLine.length() != 0){
					currentLine.pop_back();
					if(i == 0 ? false : label.at(i-1) == '\n'){
						currentLine.pop_back();
					}
					lines.push_back(currentLine);
					lastSplitIndex = i;
				}
				currentLine = label.at(i);
			}
		}

		currentLine = label.substr(lastSplitIndex);
		if(currentLine.length() != 0){
			lines.push_back(currentLine);
			if((label.size() == 0 ? false : label.at(label.size()-1) == '\n')){
				lines.at(lines.size()-1).pop_back();
				lines.push_back("");
			}
		}

		cv::Point point;
		int lineCount = lines.size();

		for(int i = 0; i < lineCount; i++){
			textSize = cv::getTextSize(lines.at(i), cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseline);
			point = cv::Point(
				(TEXT_WIDTH/2)-(textSize.width/2),
				TEXT_HEIGHT - ((lineCount-i)*textSize.height + (lineCount-i-1)*LINE_SPACE)
			);

			cv::putText(textImage, lines.at(i), point, cv::FONT_HERSHEY_SIMPLEX, 0.5f, cv::Scalar(255, 255, 255, 255), 1, 8, false);
		}

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

	void render(SDL_Renderer* renderer, SDL_Texture* texture, Camera* camera, cv::Mat* icon, SDL_Texture* textTexture, bool showTextBox){
		SDL_UpdateTexture(texture, NULL, (*icon).data, (*icon).cols * (*icon).channels());
		CoordInt renderPos = (*camera).toCameraCoordinates(position);
		SDL_Rect rect = {renderPos.x - (ICON_RES/2), renderPos.y-(ICON_RES/2), ICON_RES, ICON_RES};
		SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
		SDL_SetTextureColorMod(texture, color.r, color.g, color.b);

		SDL_SetTextureBlendMode(textTexture, SDL_BLENDMODE_BLEND);
		SDL_UpdateTexture(textTexture, NULL, textImage.data, textImage.cols*textImage.channels());
		SDL_SetTextureColorMod(textTexture, labelColor.r, labelColor.g, labelColor.b);
		SDL_Rect textRect = {rect.x - ((TEXT_WIDTH-ICON_RES)/2), rect.y-TEXT_HEIGHT, TEXT_WIDTH, TEXT_HEIGHT};

		SDL_RenderCopy(renderer, texture, NULL, &rect);
		SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
		if(showTextBox){
		SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
		SDL_RenderDrawRect(renderer, &textRect);
		}

		SDL_SetTextureColorMod(textTexture, 255, 255, 255);
		SDL_SetTextureColorMod(texture, 255, 255, 255);
	}

};

struct GuiScrollComponent{
	CoordInt position;
	int maxScroll;
	int scrollIndex;

	GuiScrollComponent(): position(), maxScroll(), scrollIndex() {}
	GuiScrollComponent(CoordInt position, int maxScroll){
		this->position = position;
		this->maxScroll = maxScroll;
		scrollIndex = 0;
	}

	void scroll(int toScroll){
		scrollIndex += toScroll;
		scrollIndex = scrollIndex % maxScroll;
		if(scrollIndex < 0){
			scrollIndex = scrollIndex+maxScroll;
		}
	}

	bool isInside(CoordInt pos){
		if(pos.x < position.x){return false;}
		if(pos.y < position.y){return false;}
		if(pos.x > position.x+ICON_RES){return false;}
		if(pos.y > position.y+ICON_RES){return false;}
		return true;
	}

	void render(SDL_Renderer* renderer, SDL_Texture* texture, cv::Mat* image, Color color = Color(255, 255, 255)){
		SDL_UpdateTexture(texture, NULL, (*image).data,  (*image).cols * (*image).channels());
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
		SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
		SDL_Rect rect = {position.x, position.y, ICON_RES, ICON_RES};
		SDL_SetTextureColorMod(texture, color.r, color.g,color.b);
		SDL_RenderCopy(renderer, texture, NULL, &rect);
		SDL_SetTextureColorMod(texture, 255, 255, 255);
	}

};

struct GuiColorScrollComponent{
	CoordInt position;
	int maxScroll;
	int scrollIndex;

	GuiColorScrollComponent(): position(), maxScroll(), scrollIndex() {}
	GuiColorScrollComponent(CoordInt position, int maxScroll){
		this->position = position;
		this->maxScroll = maxScroll;
		scrollIndex = 0;
	}

	void scroll(int toScroll){
		scrollIndex += toScroll;
		if(scrollIndex >= maxScroll){scrollIndex = maxScroll-1;}
		if(scrollIndex < 0){scrollIndex = 0;}
	}

	bool isInside(CoordInt pos){
		if(pos.x < position.x){return false;}
		if(pos.y < position.y){return false;}
		if(pos.x > position.x+ICON_RES){return false;}
		if(pos.y > position.y+ICON_RES){return false;}
		return true;
	}

	void render(SDL_Renderer* renderer, SDL_Texture* texture, cv::Mat* image, Color color = Color(255, 255, 255)){
		SDL_UpdateTexture(texture, NULL, (*image).data,  (*image).cols * (*image).channels());
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
		SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
		SDL_Rect rect = {position.x, position.y, ICON_RES, ICON_RES};
		SDL_SetTextureColorMod(texture, color.r, color.g,color.b);
		SDL_RenderCopy(renderer, texture, NULL, &rect);
		SDL_SetTextureColorMod(texture, 255, 255, 255);
	}

};

struct Level {
	int parentId;
	cv::Mat backgroundImage;
	std::vector<Marker> markers = {};

	Level(): backgroundImage(){}
	Level(cv::Mat backgroundimage, int parentId){
		this->backgroundImage = backgroundimage;
		this->parentId = parentId;
	}

};


struct Scene{
	float zoomSpeed = -0.1f;

	Window w;
	Camera camera;
	CoordInt mousePosition = CoordInt(0, 0);
	CoordInt mouseLeftDownPosition = CoordInt(0, 0);
	CoordInt mouseLeftDownCameraPosition = CoordInt(0, 0);

	int toScroll = 0;
	int originalCameraWidth;
	int originalCameraHeight;

	int baseZoomCameraWidth;
	int baseZoomCameraHeight;

	int currentLevel = 0;

	bool isMouseLeftDown = false;
	bool isUnhandledLeftMouseClick = false;
	bool isMouseRightDown = false;
	bool isUnhandledRightMouseClick = false;
	bool isUnhandledCharacterType = false;
	bool changedWindowSize = false;
	bool isMarkerSelected = false;
	bool isLeftClickMenuActive = false;
	bool isShiftDown = false;
	bool isTyping = false;
	bool isUnhandledEscape = false;

	Marker* selectedMarker;
	int rightClickedMarkerIndex = -1;

	float zoomFactor = 1.0f;

	GuiScrollComponent uppermostIconScroll;
	GuiScrollComponent upperIconScroll;
	GuiScrollComponent primaryIconScroll;
	GuiScrollComponent lowerIconScroll;
	GuiScrollComponent lowestIconScroll;

	GuiColorScrollComponent ColorRedScroll;
	GuiColorScrollComponent ColorGreenScroll;
	GuiColorScrollComponent ColorBlueScroll;

	GuiScrollComponent colorDisplayScroll;

	SDL_Texture* texture;
	SDL_Texture* iconTexture;

	SDL_Texture* textTexture;
	SDL_Texture* menuTexture;

	LeftCLickMenu lClickMenu;

	std::vector<cv::Mat> icons = {};
	std::vector<cv::Mat*> marker_icons = {};
	std::string typedText = "";
	std::vector<Level> levels = {};
	
	Scene(Window w, Camera camera){
		this->w = w;
		this->camera = camera;
		this->originalCameraWidth = camera.width;
		this->originalCameraHeight = camera.height;

		this->baseZoomCameraWidth = camera.width;
		this->baseZoomCameraHeight = camera.height;
	}

	Level getDefaultLevel(){
		return Level(getImageFromUser(), -1);
	}

	void resetGui(){
		isMouseLeftDown = false;
		isUnhandledLeftMouseClick = false;
		isMouseRightDown = false;
		isUnhandledRightMouseClick = false;
		isUnhandledCharacterType = false;
		changedWindowSize = false;
		isMarkerSelected = false;
		isLeftClickMenuActive = false;
		isShiftDown = false;
		isUnhandledEscape = false;
		toScroll = 0;
		zoomFactor = 1.0f;
		stopTyping();
	}

	void updateIconScrollComponents(){
		uppermostIconScroll.scrollIndex = primaryIconScroll.scrollIndex;
		upperIconScroll.scrollIndex = primaryIconScroll.scrollIndex;
		lowerIconScroll.scrollIndex = primaryIconScroll.scrollIndex;
		lowestIconScroll.scrollIndex = primaryIconScroll.scrollIndex;

		uppermostIconScroll.scroll(2);
		upperIconScroll.scroll(1);
		lowerIconScroll.scroll(-1);
		lowestIconScroll.scroll(-2);
	}

	void renderIconScrollComponents(SDL_Renderer* renderer){
		Color color = Color(ColorRedScroll.scrollIndex, ColorGreenScroll.scrollIndex, ColorBlueScroll.scrollIndex);
		uppermostIconScroll.render(renderer, iconTexture, marker_icons.at(uppermostIconScroll.scrollIndex), color);
		upperIconScroll.render(renderer, iconTexture, marker_icons.at(upperIconScroll.scrollIndex), color);
		primaryIconScroll.render(renderer, iconTexture, marker_icons.at(primaryIconScroll.scrollIndex), color);
		lowerIconScroll.render(renderer, iconTexture, marker_icons.at(lowerIconScroll.scrollIndex), color);
		lowestIconScroll.render(renderer, iconTexture, marker_icons.at(lowestIconScroll.scrollIndex), color);

	}

	static cv::Mat generateNoiseImage(int width, int height) {
		cv::Mat noiseImage(height, width, CV_8UC4);

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

	static Scene createSceneFromImage(cv::Mat image){
		int minRes = std::min(image.cols, image.rows);
		Scene scene = Scene(
			Window(500, 500, "Default Scene"),
			Camera(CoordInt(0, 0), minRes, minRes, 500, 500)
		);
		scene.addLevel(Level(image, -1));
		return scene;
	}

	static Scene getDefaultScene(){
		return createSceneFromImage(getImageFromUser());
	}

	void addLevel(Level level){
		levels.push_back(level);
	}

	void startTyping(){
		if(!isTyping){
			SDL_StartTextInput();
			typedText = "";
			isTyping = true;
		}
	}

	void stopTyping(){
		if(isTyping){
			SDL_StopTextInput();
			typedText = "";
			isTyping = false;
		}
	}

	int init(){
		loadIcons();
		int res = w.init();
		texture = SDL_CreateTexture(w.renderer, SDL_PIXELFORMAT_BGR24, SDL_TEXTUREACCESS_STATIC, camera.renderWidth, camera.renderHeight);
		iconTexture = SDL_CreateTexture(w.renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, ICON_RES, ICON_RES);
		textTexture = SDL_CreateTexture(w.renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, TEXT_WIDTH, TEXT_HEIGHT);

		menuTexture = SDL_CreateTexture(w.renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, MENU_OPTIONWIDTH, MENU_OPTIONHEIGHT);

		uppermostIconScroll = GuiScrollComponent(CoordInt(0, 0), marker_icons.size());
		upperIconScroll = GuiScrollComponent(CoordInt(0, 50), marker_icons.size());

		primaryIconScroll = GuiScrollComponent(CoordInt(0, 100), marker_icons.size());

		lowerIconScroll = GuiScrollComponent(CoordInt(0, 150), marker_icons.size());
		lowestIconScroll = GuiScrollComponent(CoordInt(0, 200), marker_icons.size());

		ColorRedScroll = GuiColorScrollComponent(CoordInt(50, 0), 256);
		ColorGreenScroll = GuiColorScrollComponent(CoordInt(100, 0), 256);
		ColorBlueScroll = GuiColorScrollComponent(CoordInt(150, 0), 256);

		ColorRedScroll.scroll(255);
		ColorGreenScroll.scroll(255);
		ColorBlueScroll.scroll(255);

		colorDisplayScroll = GuiScrollComponent(CoordInt(200, 0), 1);

		lClickMenu = LeftCLickMenu(CoordInt(0, 0));
		lClickMenu.addOption(MenuOption(MENU_OPTIONWIDTH, MENU_OPTIONHEIGHT, "Delete", Color(172, 176, 189), Color(37, 22, 5), OPTION_DELETE));
		lClickMenu.addOption(MenuOption(MENU_OPTIONWIDTH, MENU_OPTIONHEIGHT, "Rename", Color(172, 176, 189), Color(37, 22, 5), OPTION_RENAME));
		lClickMenu.addOption(MenuOption(MENU_OPTIONWIDTH, MENU_OPTIONHEIGHT, "+ Level", Color(172, 176, 189), Color(37, 22, 5), OPTION_ADD_LEVEL));
		lClickMenu.addOption(MenuOption(MENU_OPTIONWIDTH, MENU_OPTIONHEIGHT, "Open", Color(172, 176, 189), Color(37, 22, 5), OPTION_OPEN_LEVEL, false));

		updateIconScrollComponents();

		return res;
	}

	static cv::Mat getImageFromUser(){
		cv::Mat image;
		std::string path = "";

	read_again:
		path = getFileFromUser(std::vector<std::string> {"bmp", "dib", "jpeg", "jpg", "jpe", "jp2", "png", "webp", "pbm", "pgm", "ppm", "pxm", "pnm", "sr", "ras", "tiff", "tif", "exr", "hdr", "pic"});
		if(path.length() == 0){goto read_again;}
		if(!std::filesystem::exists(path)){goto read_again;}
		image = cv::imread(path, cv::IMREAD_COLOR);
		if(image.empty()){goto read_again;}

		std::cout << path << std::endl;

		return image;
	}

	static std::string getExecutableDirectory()
	{
		char path[FILENAME_MAX];
		std::size_t length = sizeof(path);

		if (::GetModuleFileNameA(NULL, path, length) != 0)
		{
			std::filesystem::path executablePath = std::filesystem::path(path);
			return executablePath.parent_path().string();
		}

		return "";
	}

	void loadIcons(){
		icons.push_back(cv::imread(getExecutableDirectory() + "\\Assets\\icon1.png", cv::IMREAD_UNCHANGED));
		icons.push_back(cv::imread(getExecutableDirectory() + "\\Assets\\marker_toggle.png", cv::IMREAD_UNCHANGED));
		icons.push_back(cv::imread(getExecutableDirectory() + "\\Assets\\icon2.png", cv::IMREAD_UNCHANGED));
		icons.push_back(cv::Mat(50, 50, CV_8UC4 ,cv::Scalar(255, 255, 255, 255)));

		std::cout << getExecutableDirectory() + "\\Assets\\icon1.png" <<std::endl;
		std::cout << std::filesystem::exists(getExecutableDirectory() + "\\Assets\\icon1.png") << std::endl;

		marker_icons.push_back( &(icons.at(0)) );
		marker_icons.push_back( &(icons.at(2)) );

	}

	void addMarker(Marker marker){
		levels.at(currentLevel).markers.push_back(marker);
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

		if(isTyping){
			if(event.type == SDL_KEYDOWN){
				if(event.key.keysym.sym == SDLK_KP_ENTER || event.key.keysym.sym == SDLK_RETURN){
					typedText += '\n';
					isUnhandledCharacterType = true;
				}
				if(event.key.keysym.sym == SDLK_BACKSPACE){
					if(typedText.length() != 0){
						typedText.pop_back();
						isUnhandledCharacterType = true;
					}
				}
			}

			else if(event.type == SDL_TEXTINPUT){
				typedText += event.text.text;
				isUnhandledCharacterType = true;
			}
		}

		if(event.type == SDL_WINDOWEVENT){
			if(event.window.event == SDL_WINDOWEVENT_RESIZED){
				changeOutputResolution(event.window.data1, event.window.data2);
				changedWindowSize = true;
			}
		}

		if(event.type == SDL_MOUSEWHEEL){
			toScroll += event.wheel.y;
			isLeftClickMenuActive = false;
			stopTyping();
		}

		if(event.type == SDL_MOUSEBUTTONDOWN){
			stopTyping();
			if(event.button.button == SDL_BUTTON_LEFT){

				isMouseLeftDown = true;
				isUnhandledLeftMouseClick = true;
				SDL_GetMouseState(
					&(mouseLeftDownPosition.x),
					&(mouseLeftDownPosition.y));
				mouseLeftDownCameraPosition = camera.position;
				SDL_GetMouseState(
					&(mousePosition.x),
					&(mousePosition.y));
				for(int i = 0; i < levels.at(currentLevel).markers.size(); i++){
					if(levels.at(currentLevel).markers.at(i).isInside(mousePosition, &camera)){
						isMarkerSelected = true;
						selectedMarker = &(levels.at(currentLevel).markers.at(i));
						break;
					}
				}
				
			}
			if(event.button.button == SDL_BUTTON_RIGHT){
				isMouseRightDown = true;
				isUnhandledRightMouseClick = true;
			}
		}
		if(event.type == SDL_MOUSEBUTTONUP){
			if(event.button.button == SDL_BUTTON_LEFT){isMouseLeftDown = false;}
			if(event.button.button == SDL_BUTTON_RIGHT){isMouseRightDown = false;}
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
					for(int i = 0; i < levels.at(currentLevel).markers.size(); i++){
						if(levels.at(currentLevel).markers.at(i).isInside(mousePosition, &camera)){
							levels.at(currentLevel).markers.erase(levels.at(currentLevel).markers.begin()+i);
						}
					}
				}
				isLeftClickMenuActive = false;
			}
			if(event.key.keysym.sym == SDLK_LSHIFT){isShiftDown = true;}
			if(event.key.keysym.sym == SDLK_ESCAPE){
				isUnhandledEscape = true && !isTyping;
			}
		}
		if(event.type == SDL_KEYUP){
			if(event.key.keysym.sym == SDLK_LSHIFT){isShiftDown = false;}
		}
		

	}

	float getMaxZoomFactor(){
		return std::min(
			(float)levels.at(currentLevel).backgroundImage.cols/(float)baseZoomCameraWidth,
			(float)levels.at(currentLevel).backgroundImage.rows/(float)baseZoomCameraHeight
		);
	}
	void renderBackground(SDL_Renderer* renderer){
		cv::Mat image = camera.getBackgroundImage(levels.at(currentLevel).backgroundImage);
		SDL_UpdateTexture(texture, NULL, image.data, image.cols*3);
		SDL_RenderCopy(renderer, texture, NULL, NULL);

	}

	void renderMarkers(SDL_Renderer* renderer){
		for(int i = 0; i < levels.at(currentLevel).markers.size(); i++){
			if(levels.at(currentLevel).markers.at(i).isVisible(&camera)){
				cv::Mat* pt;
				if(levels.at(currentLevel).markers.at(i).iconIndex >= marker_icons.size()){
					pt = marker_icons.at(0);
				}else{
					pt = marker_icons.at(levels.at(currentLevel).markers.at(i).iconIndex);
				}
				levels.at(currentLevel).markers.at(i).render(renderer, iconTexture, &camera, pt, textTexture, isTyping && i == rightClickedMarkerIndex);
			}
		}
	}

	bool isInsideIconScrolls(CoordInt pos){
		if(pos.x < uppermostIconScroll.position.x){return false;}
		if(pos.y < uppermostIconScroll.position.y){return false;}
		if(pos.x > lowestIconScroll.position.x + ICON_RES){return false;}
		if(pos.y > lowestIconScroll.position.y + ICON_RES){return false;}
		return true;
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

		if(camera.width > levels.at(currentLevel).backgroundImage.cols){camera.width = levels.at(currentLevel).backgroundImage.cols;}
		if(camera.height > levels.at(currentLevel).backgroundImage.rows){camera.height = levels.at(currentLevel).backgroundImage.rows;}

		if(camera.position.x+camera.width > levels.at(currentLevel).backgroundImage.cols){
			camera.position.x -= camera.position.x+camera.width-levels.at(currentLevel).backgroundImage.cols;
		}
		if(camera.position.y+camera.height > levels.at(currentLevel).backgroundImage.rows){
			camera.position.y -= camera.position.y+camera.height-levels.at(currentLevel).backgroundImage.rows;
		}
	}

	void render(){
		renderBackground(w.renderer);
		renderMarkers(w.renderer);
		renderIconScrollComponents(w.renderer);
		ColorRedScroll.render(w.renderer, iconTexture, &(icons.at(3)), Color(ColorRedScroll.scrollIndex, 0, 0));
		ColorGreenScroll.render(w.renderer, iconTexture, &(icons.at(3)), Color(0, ColorGreenScroll.scrollIndex, 0));
		ColorBlueScroll.render(w.renderer, iconTexture, &(icons.at(3)), Color(0, 0, ColorBlueScroll.scrollIndex));

		colorDisplayScroll.render(w.renderer, iconTexture, &(icons.at(3)), Color(ColorRedScroll.scrollIndex, ColorGreenScroll.scrollIndex, ColorBlueScroll.scrollIndex));

		if(isLeftClickMenuActive){
			lClickMenu.render(w.renderer, menuTexture);
		}

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

		if(isUnhandledEscape){
			if(levels.at(currentLevel).parentId < 0){}
			else{

				currentLevel = levels.at(currentLevel).parentId;
				resetGui();
			}
			isUnhandledEscape = false;
		}

		if(isUnhandledRightMouseClick){
			for(int i = 0; i < levels.at(currentLevel).markers.size(); i++){
				if(levels.at(currentLevel).markers.at(i).isInside(mousePosition, &camera)){
					lClickMenu.position = mousePosition;
					if(levels.at(currentLevel).markers.at(i).levelLink < 0){
						lClickMenu.options.at(2).isEnabled = true;
						lClickMenu.options.at(3).isEnabled = false;
					}else{
						lClickMenu.options.at(2).isEnabled = false;
						lClickMenu.options.at(3).isEnabled = true;
					}

					isLeftClickMenuActive = true;
					rightClickedMarkerIndex = i;
					break;
				}
				if(i == levels.at(currentLevel).markers.size()-1){
					isLeftClickMenuActive = false;
				}
			}
			isUnhandledRightMouseClick = false;
		}

		if(isUnhandledLeftMouseClick && isLeftClickMenuActive){
			int optionIndex = lClickMenu.getOption(mousePosition);
			int option;
			if(optionIndex < 0){
				option == -1;
			}
			else{
				option = lClickMenu.options.at(optionIndex).optionType;
			}

			if(option == -1){

			}
			if(option == OPTION_DELETE){
				levels.at(currentLevel).markers.erase(levels.at(currentLevel).markers.begin()+rightClickedMarkerIndex);
				rightClickedMarkerIndex = -1;
			}			
			if(option == OPTION_RENAME){
				startTyping();
				typedText = levels.at(currentLevel).markers.at(rightClickedMarkerIndex).label;
				isLeftClickMenuActive = false;
			}
			if(option == OPTION_ADD_LEVEL){
				int newLevelId = levels.size();
				levels.push_back(Level(getImageFromUser(), currentLevel));
				levels.at(currentLevel).markers.at(rightClickedMarkerIndex).levelLink = newLevelId;
			}
			if(option == OPTION_OPEN_LEVEL){
				currentLevel = levels.at(currentLevel).markers.at(rightClickedMarkerIndex).levelLink;
				resetGui();
			}
			isLeftClickMenuActive = false;
			isUnhandledLeftMouseClick = false;
		}
		
		if(isUnhandledCharacterType){
			levels.at(currentLevel).markers.at(rightClickedMarkerIndex).UpdateLabel(typedText);
			isUnhandledCharacterType = false;
		}

		
		if(toScroll != 0 && isInsideIconScrolls(mousePosition)){
			primaryIconScroll.scroll(toScroll);
			updateIconScrollComponents();
			toScroll = 0;
		}

		if(toScroll != 0 && ColorRedScroll.isInside(mousePosition)){
			ColorRedScroll.scroll(toScroll * (isShiftDown ? 1 : 20));
			toScroll = 0;
		}

		if(toScroll != 0 && ColorGreenScroll.isInside(mousePosition)){
			ColorGreenScroll.scroll(toScroll * (isShiftDown ? 1 : 20));
			toScroll = 0;
		}

		if(toScroll != 0 && ColorBlueScroll.isInside(mousePosition)){
			ColorBlueScroll.scroll(toScroll * (isShiftDown ? 1 : 20));
			toScroll = 0;
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
		toScroll = 0;

		if(isUnhandledLeftMouseClick && !isMarkerSelected){
			GuiScrollComponent* pt = NULL;
			if(uppermostIconScroll.isInside(mousePosition)){pt = &uppermostIconScroll;}
			if(upperIconScroll.isInside(mousePosition)){pt = &upperIconScroll;}
			if(primaryIconScroll.isInside(mousePosition)){pt = &primaryIconScroll;}
			if(lowerIconScroll.isInside(mousePosition)){pt = &lowerIconScroll;}
			if(lowestIconScroll.isInside(mousePosition)){pt = &lowestIconScroll;}
			if(pt != NULL){
				addMarker(Marker(mousePosition, Color(ColorRedScroll.scrollIndex, ColorGreenScroll.scrollIndex, ColorBlueScroll.scrollIndex), Color(0, 0, 0), "new Marker", (*pt).scrollIndex, 25));
				selectedMarker = &(levels.at(currentLevel).markers.at(levels.at(currentLevel).markers.size() - 1));
				isMarkerSelected = true;
			}
		}

		
		if(isMarkerSelected){
			(*selectedMarker).position = camera.fromCameraCoordinates(mousePosition);
			isUnhandledLeftMouseClick = false;
		}

		if(isMouseLeftDown && !isMarkerSelected){
			camera.position.x = mouseLeftDownCameraPosition.x - (camera.scaleFromCameraCoordinates(mousePosition).x-camera.scaleFromCameraCoordinates(mouseLeftDownPosition).x);
			camera.position.y = mouseLeftDownCameraPosition.y - (camera.scaleFromCameraCoordinates(mousePosition).y-camera.scaleFromCameraCoordinates(mouseLeftDownPosition).y);
			isUnhandledLeftMouseClick = false;
		}
		clipCamera();
		}
};
