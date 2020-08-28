#pragma once
#include "olcPixelGameEngine.h"

bool runEditor = true;
bool CAN_RUN_EDITOR = true;

olc::vi2d vCursor = { 1, 1 };

struct sCell
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

	sCell& GetCell(const olc::vi2d& v)
	{
		if (v.x >= 0 && v.x < size.x && v.y >= 0 && v.y < size.y)
			return vCells[v.y * size.x + v.x];
		else
			return NullCell;
	}

	bool* GetWallData() {
		bool* data = new bool[size.x * size.y];

		int index = 0;
		for (auto cell : vCells)
		{
			data[index] = cell.wall;
			index++;
		}

		return data;
	}

public:
	olc::vi2d size;
private:
	std::vector<sCell> vCells;
	sCell NullCell;
};

World world;

std::string CurrentScene = "templateMap";

int screenHeight;
int screenWidth;

struct vec3d
{
	float x, y, z;
};

struct sQuad
{
	vec3d points[4];
	olc::vf2d tile;
};

struct Renderable
{
	Renderable() {}

	void Load(const std::string& sFile)
	{
		sprite = new olc::Sprite(sFile);
		decal = new olc::Decal(sprite);
	}

	~Renderable()
	{
		delete decal;
		delete sprite;
	}

	olc::Sprite* sprite = nullptr;
	olc::Decal* decal = nullptr;
};

enum Face
{
	Floor = 0,
	North = 1,
	East = 2,
	South = 3,
	West = 4,
	Top = 5
};

olc::vi2d vTileSize = { 24, 24 };
bool bVisible[6];

std::array<vec3d, 8> CreateSprite(const olc::vi2d& vCell, const float fAngle, const float fPitch, const float fScale, const vec3d& vCamera)
{
	// Unit Cube
	std::array<vec3d, 8> unitCube, rotCube, worldCube, projCube;
	unitCube[0] = { 0.0f, 0.0f, 0.0f };
	unitCube[1] = { fScale, 0.0f, 0.0f };
	unitCube[2] = { fScale, -fScale, 0.0f };
	unitCube[3] = { 0.0f, -fScale, 0.0f };

	// Translate Cube in X-Z Plane
	for (int i = 0; i < 4; i++)
	{
		unitCube[i].x += (vCell.x * fScale - vCamera.x);
		unitCube[i].y += -vCamera.y;
		unitCube[i].z += ((vCell.y + 0.5f) * fScale - vCamera.z);
	}

	// Rotate Cube in Y-Axis around origin
	for (int i = 0; i < 4; i++)
	{
		rotCube[i].x = unitCube[i].x;
		rotCube[i].y = unitCube[i].y;
		rotCube[i].z = unitCube[i].x;

		// Rotate Cube in X-Axis around origin (tilt slighly overhead);
		worldCube[i].x = rotCube[i].x;
		worldCube[i].y = rotCube[i].y;
		worldCube[i].z = rotCube[i].y;
	}

	// Project Cube Orthographically - Full Screen Centered
	for (int i = 0; i < 4; i++)
	{
		projCube[i].x = worldCube[i].x + screenHeight * 0.5f;
		projCube[i].y = worldCube[i].y + screenWidth * 0.5f;
		projCube[i].z = worldCube[i].z;
	}

	return projCube;
}

std::array<vec3d, 8> CreateCube(const olc::vi2d& vCell, const float fAngle, const float fPitch, const float fScale, const vec3d& vCamera)
{
	// Unit Cube
	std::array<vec3d, 8> unitCube, rotCube, worldCube, projCube;
	unitCube[0] = { 0.0f, 0.0f, 0.0f };
	unitCube[1] = { fScale, 0.0f, 0.0f };
	unitCube[2] = { fScale, -fScale, 0.0f };
	unitCube[3] = { 0.0f, -fScale, 0.0f };
	unitCube[4] = { 0.0f, 0.0f, fScale };
	unitCube[5] = { fScale, 0.0f, fScale };
	unitCube[6] = { fScale, -fScale, fScale };
	unitCube[7] = { 0.0f, -fScale, fScale };

	// Translate Cube in X-Z Plane
	for (int i = 0; i < 8; i++)
	{
		unitCube[i].x += (vCell.x * fScale - vCamera.x);
		unitCube[i].y += -vCamera.y;
		unitCube[i].z += (vCell.y * fScale - vCamera.z);
	}

	// Rotate Cube in Y-Axis around origin
	float s = sin(fAngle);
	float c = cos(fAngle);
	for (int i = 0; i < 8; i++)
	{
		rotCube[i].x = unitCube[i].x * c + unitCube[i].z * s;
		rotCube[i].y = unitCube[i].y;
		rotCube[i].z = unitCube[i].x * -s + unitCube[i].z * c;
	}

	// Rotate Cube in X-Axis around origin (tilt slighly overhead)
	s = sin(fPitch);
	c = cos(fPitch);
	for (int i = 0; i < 8; i++)
	{
		worldCube[i].x = rotCube[i].x;
		worldCube[i].y = rotCube[i].y * c - rotCube[i].z * s;
		worldCube[i].z = rotCube[i].y * s + rotCube[i].z * c;
	}

	// Project Cube Orthographically - Full Screen Centered
	for (int i = 0; i < 8; i++)
	{
		projCube[i].x = worldCube[i].x + screenHeight * 0.5f;
		projCube[i].y = worldCube[i].y + screenWidth * 0.5f;
		projCube[i].z = worldCube[i].z;
	}

	// Project Cube Orthographically - Unit Cube Viewport
	//float fLeft = -ScreenWidth() * 0.5f;
	//float fRight = ScreenWidth() * 0.5f;
	//float fTop = ScreenHeight() * 0.5f;
	//float fBottom = -ScreenHeight() * 0.5f;
	//float fNear = 0.1f;
	//float fFar = 100.0f;
	//for (int i = 0; i < 8; i++)
	//{
	//	projCube[i].x = (2.0f / (fRight - fLeft)) * worldCube[i].x - ((fRight + fLeft) / (fRight - fLeft));
	//	projCube[i].y = (2.0f / (fTop - fBottom)) * worldCube[i].y - ((fTop + fBottom) / (fTop - fBottom));
	//	projCube[i].z = (2.0f / (fFar - fNear)) * worldCube[i].z - ((fFar + fNear) / (fFar - fNear));
	//	projCube[i].x *= -fRight;
	//	projCube[i].y *= -fTop;
	//	projCube[i].x += fRight;
	//	projCube[i].y += fTop;
	//}

	return projCube;
}

void CalculateVisibleFaces(std::array<vec3d, 8>& cube)
{
	auto CheckNormal = [&](int v1, int v2, int v3)
	{
		olc::vf2d a = { cube[v1].x, cube[v1].y };
		olc::vf2d b = { cube[v2].x, cube[v2].y };
		olc::vf2d c = { cube[v3].x, cube[v3].y };
		return  (b - a).cross(c - a) > 0;
	};

	bVisible[Face::Floor] = CheckNormal(4, 0, 1);
	bVisible[Face::South] = CheckNormal(3, 0, 1);
	bVisible[Face::North] = CheckNormal(6, 5, 4);
	bVisible[Face::East] = CheckNormal(7, 4, 0);
	bVisible[Face::West] = CheckNormal(2, 1, 5);
	bVisible[Face::Top] = CheckNormal(7, 3, 2);
}

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

void GetSpriteQuads(const olc::vi2d& vCell, const float fAngle, const float fPitch, const float fScale, const vec3d& vCamera, sQuad& render)
{
	std::array<vec3d, 8> projCube = CreateSprite(vCell, fAngle, fPitch, fScale, vCamera);

	auto& cell = world.GetCell(vCell);

	auto MakeFace = [&](int v1, int v2, int v3, int v4, Face f)
	{
		render = { projCube[v1], projCube[v2], projCube[v3], projCube[v4], cell.id[f] };
	};

	MakeFace(3, 0, 1, 2, Face::Top);
}