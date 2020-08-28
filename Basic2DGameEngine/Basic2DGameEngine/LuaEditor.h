#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include "luaReader.h"

using namespace std;

void SaveFile(string fileName, string inputData) {
	ofstream fileStream;
	fileStream.open(fileName + ".lua");

	if (fileStream.is_open()) {
		string data = "";

		for (size_t u = 0; u < inputData.length(); u++)
		{
			if (data.size() < data.max_size())
				data += inputData[u];
			else {
				cout << "Out of memory" << endl;
				fileStream << data;
				break;
			}
		}

		fileStream << data;

		cout << "Finished saving " << fileName << endl;
	}

	fileStream.close();
}

const char* GetMapData(int mapX, int mapY, bool* wallData) {
	// Goals
	// Create a map save file
	string data;

	// Save the map X and Y
	data += "sizeX = " + to_string(mapX) + "\n";
	data += "sizeY = " + to_string(mapY) + "\n\n";

	// Save the map data to an array
	data += "tiles = \n{";
	for (int i = 0; i < mapX * mapY; i++)
	{
		int x = i % mapX;
		if (x == 0) {
			data += "\n\t";
		}

		data += to_string(wallData[i]) + ",";
	}
	data += "\n}\n\n";

	// can't forget the get data function
	data += "function GetMapData(index)\n\treturn tiles[index]\nend";

	// Debug
	//cout << data << endl;

	return data.c_str();
}

void SaveMapData(int mapX, int mapY, bool* wallData) {
	const char* data = GetMapData(mapX, mapY, wallData);
	string name = "luaScripts\\";
	SaveFile(name + CurrentScene, data);
}

void ConstructMap(string map) {
	lua_State* L = luaL_newstate();
	luaL_openlibs(L);

	int mapX = 0, mapY = 0;

	const char* mapLocation = ("luaScripts/" + map + ".lua").c_str();

	if (CheckLua(L, luaL_dofile(L, mapLocation))) {
		// No Error
		lua_getglobal(L, "sizeX");
		if (lua_isnumber(L, -1)) {
			int num = (int)lua_tonumber(L, -1);
			std::cout << "map size X set to: " << num << std::endl;

			mapX = num;
			lua_pop(L, -1);
		}

		lua_getglobal(L, "sizeY");
		if (lua_isnumber(L, -1)) {
			int num = (int)lua_tonumber(L, -1);
			std::cout << "map size Y set to: " << num << std::endl;

			mapY = num;
			lua_pop(L, -1);
		}

		// Do a function to recieve map data from the LUA map
		lua_getglobal(L, "GetMapData");
		if (lua_isfunction(L, -1)) {
			world.Create(mapX, mapY);
			int size = mapX * mapY;

			for (int i = 0; i < size; i++)
			{
				int x = i % mapX;
				int y = i / mapX;

				lua_getglobal(L, "GetMapData"); // Access the global function for map data
				lua_pushinteger(L, i + 1); // Feed lua the index we want to access

				// Call the function
				if (CheckLua(L, lua_pcall(L, 1, 1, 0))) {
					// Recieved a number
					int num = (int)lua_tonumber(L, -1);
					world.GetCell({ x, y }).wall = num != 0;
				}
				else {
					// Failed to recieve number
					std::cout << "Failed to recieve number data at index: " << i << std::endl;
					world.GetCell({ x, y }).wall = (y == 0 || y == world.size.y - 1 || x == 0 || x == world.size.x - 1);
				}

				world.GetCell({ x, y }).id[Face::Floor] = olc::vi2d{ 0, 2 } *vTileSize;
				world.GetCell({ x, y }).id[Face::Top] = olc::vi2d{ 0, 0 } *vTileSize;
				world.GetCell({ x, y }).id[Face::North] = olc::vi2d{ 0, 1 } *vTileSize;
				world.GetCell({ x, y }).id[Face::South] = olc::vi2d{ 0, 1 } *vTileSize;
				world.GetCell({ x, y }).id[Face::West] = olc::vi2d{ 0, 1 } *vTileSize;
				world.GetCell({ x, y }).id[Face::East] = olc::vi2d{ 0, 1 } *vTileSize;
			}

			std::cout << "Generated " << map << " size of: " << size << std::endl << std::flush;
		}
		else {
			std::cout << "Failed to recieve map data when trying to access the 'GetMapData([index])' function!" << std::endl;;
		}
	}
	else {
		// Log Error
		std::string errorMessage = lua_tostring(L, -1);
		std::cout << errorMessage << std::endl;
	}

	// Close this lua instance once we have completed as we won't continue using it.
	lua_close(L);
}