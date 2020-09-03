#pragma once
#include "olcPixelGameEngine.h"

bool bVisible[6];
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

std::array<vec3d, 8> CreateSprite(const olc::vi2d& vCell, const float fAngle, const float fPitch, const float fScale, const vec3d& vCamera)
{
	// Unit Cube
	std::array<vec3d, 8> unitCube, rotCube, worldCube, projCube;

	float zero = fScale * 0.1;
	float one = fScale * 0.9;

	unitCube[0] = { zero, zero, one };
	unitCube[1] = { one, zero, zero };
	unitCube[2] = { one, -one, zero };
	unitCube[3] = { zero, -one, one };

	// Translate Cube in X-Z Plane
	for (int i = 0; i < 8; i++)
	{
		unitCube[i].x += (vCell.x * fScale - vCamera.x);
		unitCube[i].y += -vCamera.y;
		unitCube[i].z += (vCell.y * fScale - vCamera.z);
	}

	float s = sin(fAngle);
	float c = cos(fAngle);
	// Rotate sprite in Y-Axis around origin
	for (int i = 0; i < 4; i++)
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
		projCube[i].z = worldCube[i].z + 15; // add a bunch to this to ensure sprites are drawn over world objects
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