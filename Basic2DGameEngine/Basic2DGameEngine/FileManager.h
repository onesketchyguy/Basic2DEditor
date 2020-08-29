#pragma once
#include <string>
#include <iostream>
#include <filesystem>

#define WINDOWS

#ifdef WINDOWS
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

namespace fs = std::filesystem;

std::vector<std::string> mapFiles;
std::string* noPaths;
int noPathLength;

std::string get_current_dir() {
	char buff[FILENAME_MAX]; //create string buffer to hold path
	GetCurrentDir(buff, FILENAME_MAX);
	std::string current_working_dir(buff);
	return current_working_dir;
}

std::vector<std::string> GetFiles(std::string loc) {
	mapFiles.clear();
	noPathLength = 0;

	std::string path = get_current_dir() + "\\" + loc;
	for (const auto& entry : fs::directory_iterator(path)) {
		std::string name = entry.path().string();

		noPathLength++;

		mapFiles.push_back(name);
	}

	int index = 0;

	noPathLength++;
	noPaths = new std::string[noPathLength];
	for (auto name : mapFiles)
	{
		std::string toAdd = "";
		for (int i = 0; i < name.length(); i++)
		{
			if (name[i] == '/' || name[i] == '\\') {
				toAdd = "";
				continue;
			}

			if (name[i] == '.') break;

			toAdd += name[i];
		}

		noPaths[index] = toAdd;
		index++;
	}

	noPaths[noPathLength - 1] = "New...";

	return mapFiles;
}