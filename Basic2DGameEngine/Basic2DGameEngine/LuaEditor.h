#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include "luaReader.h"

void SaveFile(std::string fileName, std::string inputData) {
	std::ofstream fileStream;
	fileStream.open(fileName + ".lua");

	if (fileStream.is_open()) {
		while (1) {
			std::string data = "";

			for (size_t u = 0; u < inputData.length(); u++)
			{
				if (data.size() < data.max_size())
					data += inputData[u];
				else {
					std::cout << "Out of memory" << std::endl;
					fileStream << data;
					break;
				}
			}

			fileStream << data;
			break;
		}

		std::cout << "Finished saving " << fileName << std::endl;
	}

	fileStream.close();
}

/// <summary>
/// Create a string of map data formated for a lua file.
/// </summary>
/// <param name="mapX"></param>
/// <param name="mapY"></param>
/// <param name="wallData"></param>
/// <returns></returns>
void SaveMapData(World worldData) {
	// Create a map save file
	std::string data;

	auto wallData = worldData.GetWallData();

	// Save the map X and Y
	data += "sizeX = " + std::to_string(worldData.size.x) + "\n";
	data += "sizeY = " + std::to_string(worldData.size.y) + "\n\n";

	data += "pSpawnPoint = { x = " + std::to_string(worldData.playerSpawnPoint.x) + ", y = "
		+ std::to_string(worldData.playerSpawnPoint.y) + "}\n\n";

	// Save the map data to an array
	data += "tiles = \n{";
	for (int i = 0; i < worldData.size.x * worldData.size.y; i++)
	{
		int x = i % worldData.size.x;
		if (x == 0) {
			data += "\n\t";
		}

		data += std::to_string(wallData[i]) + ",";
	}
	data += "\n}\n\n";

	// can't forget the get data function
	data += "function GetMapData(index)\n\treturn tiles[index]\nend";

	std::string name = "luaScripts\\maps\\";
	SaveFile(name + CurrentScene, data);
}

void ConstructMap(std::string map) {
	lua_State* L = luaL_newstate();
	luaL_openlibs(L);

	int mapX = 0, mapY = 0;
	int sX = 0, sY = 0;

	std::string mapLocation = "luaScripts\\maps\\" + map + ".lua";

	if (CheckLua(L, luaL_dofile(L, mapLocation.c_str()))) {
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

		lua_getglobal(L, "pSpawnPoint");
		if (lua_istable(L, -1)) {
			lua_pushstring(L, "x");
			lua_gettable(L, -2);
			int num = (int)lua_tonumber(L, -1);

			sX = num;
			lua_pop(L, -1);
		}

		lua_getglobal(L, "pSpawnPoint");
		if (lua_istable(L, -1)) {
			lua_pushstring(L, "y");
			lua_gettable(L, -2);
			int num = (int)lua_tonumber(L, -1);

			sY = num;
			lua_pop(L, -1);
		}

		// Do a function to recieve map data from the LUA map
		lua_getglobal(L, "GetMapData");
		if (lua_isfunction(L, -1)) {
			world.Create(mapX, mapY);

			world.playerSpawnPoint = { sX, sY };

			int size = mapX * mapY;

			for (long i = 0; i < size; i++)
			{
				INT32 x = i % mapX;
				INT32 y = i / mapX;

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