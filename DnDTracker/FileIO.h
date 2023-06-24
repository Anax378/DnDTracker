#pragma once
#include <string>
#include <vector>

#ifdef _WIN32

#pragma once
#include <windows.h>

std::string getFileFromUser(std::vector<std::string> validExtensions){
	std::string filter;
    for(int i = 0; i < validExtensions.size(); i++){
        filter += "*." + validExtensions.at(i);
        if(i != validExtensions.size()-1){
            filter += "; ";
        }
    }

    filter.append("\0", 1);
	for(int i = 0; i < validExtensions.size(); i++){
        filter += "*."+validExtensions.at(i);
        if(i != validExtensions.size()-1){
            filter += ";";
        }
	}
    filter.append("\0", 1);
    std::cout <<filter << std::endl;
	//filter += "All Files\0*.*\0";

    OPENFILENAMEA ofn;
    CHAR fileName[MAX_PATH] = "";

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = filter.c_str();
    ofn.nFilterIndex = 1;
    ofn.lpstrTitle = "Open File";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileNameA(&ofn) == TRUE)
    {
        return std::string(ofn.lpstrFile);
    }
    else{
        return "";
    }
}



#elif defined(__linux__)


#else
#error No Mac
#endif




