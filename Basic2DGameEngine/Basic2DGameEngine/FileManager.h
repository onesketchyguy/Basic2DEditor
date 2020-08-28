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

vector<string> mapFiles;
string* noPaths;
int noPathLength;

std::string get_current_dir() {
	char buff[FILENAME_MAX]; //create string buffer to hold path
	GetCurrentDir(buff, FILENAME_MAX);
	string current_working_dir(buff);
	return current_working_dir;
}

vector<string> GetFiles() {
	mapFiles.clear();
	noPathLength = 0;

	std::string path = get_current_dir() + "/luaScripts";
	for (const auto& entry : fs::directory_iterator(path)) {
		string name = entry.path().string();

		noPathLength++;

		mapFiles.push_back(name);
	}

	int index = 0;

	noPathLength++;
	noPaths = new string[noPathLength];
	for (auto name : mapFiles)
	{
		string toAdd = "";
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