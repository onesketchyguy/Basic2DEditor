#pragma once
#include "rendering.h"

/// <summary>
/// Initialized in lua
/// </summary>
bool runEditor = true;
std::string tileMapLocation;

const double pi = 3.14159265359;

olc::vi2d vCursor = { 1, 1 };
Renderable rendAllWalls;
Renderable rendSelect;
Renderable rendPlayer;
olc::vi2d vTileSize = { 24, 24 };

struct tile
{
	bool wall = false;
	olc::vi2d id[6]{  };
};

class World
{
public:
	World()
	{
	}

	void Create(int w, int h)
	{
		size = { w, h };
		vCells.resize(w * h);
	}

	tile& GetCell(const olc::vi2d& v)
	{
		if (v.x >= 0 && v.x < size.x && v.y >= 0 && v.y < size.y)
			return vCells[v.y * size.x + v.x];
		else
			return NullCell;
	}

	bool* GetWallData() {
		bool* data = new bool[size.x * size.y];

		for (int i = 0; i < size.x * size.y; i++)
			data[i] = vCells[i].wall;

		return data;
	}

public:
	olc::vi2d size;
	olc::vi2d playerSpawnPoint;
private:
	std::vector<tile> vCells;
	tile NullCell;
};

World world;

std::string CurrentScene = "templateMap";

void GetFaceQuads(const olc::vi2d& vCell, const float fAngle, const float fPitch, const float fScale, const vec3d& vCamera, std::vector<sQuad>& render)
{
	std::array<vec3d, 8> projCube = CreateCube(vCell, fAngle, fPitch, fScale, vCamera);

	auto& cell = world.GetCell(vCell);

	auto MakeFace = [&](int v1, int v2, int v3, int v4, Face f)
	{
		render.push_back({ projCube[v1], projCube[v2], projCube[v3], projCube[v4], cell.id[f] });
	};

	if (!cell.wall)
	{
		if (bVisible[Face::Floor]) MakeFace(4, 0, 1, 5, Face::Floor);
	}
	else
	{
		if (bVisible[Face::South]) MakeFace(3, 0, 1, 2, Face::South);
		if (bVisible[Face::North]) MakeFace(6, 5, 4, 7, Face::North);
		if (bVisible[Face::East]) MakeFace(7, 4, 0, 3, Face::East);
		if (bVisible[Face::West]) MakeFace(2, 1, 5, 6, Face::West);
		if (bVisible[Face::Top]) MakeFace(7, 3, 2, 6, Face::Top);
	}
}

void GetSpriteQuads(const olc::vi2d& vCell, const float fAngle, const float fPitch, const float fScale, const vec3d& vCamera, std::vector<sQuad>& render)
{
	std::array<vec3d, 8> projSprite = CreateSprite(vCell, fAngle, fPitch, fScale, vCamera);

	auto& cell = world.GetCell(vCell);

	auto MakeFace = [&](int v1, int v2, int v3, int v4, Face f)
	{
		render.push_back({ projSprite[v1], projSprite[v2], projSprite[v3], projSprite[v4], cell.id[f] });
	};

	MakeFace(3, 0, 1, 2, Face::North);
	MakeFace(7, 4, 5, 6, Face::South);
}