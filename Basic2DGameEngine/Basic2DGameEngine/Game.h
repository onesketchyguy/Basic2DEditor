#pragma once
#include "olcPixelGameEngine.h"
#include "CoreData.h"

class olcDungeonPlayer : public olc::PixelGameEngine
{
public:
	olcDungeonPlayer()
	{
		sAppName = "Dungeon Explorer";
	}

	Renderable rendSelect;
	Renderable rendPlayer;
	Renderable rendAllWalls;

	olc::vf2d vCameraPos = { 0.0f, 0.0f };
	float fCameraAngle = 0.0f;
	float fCameraAngleTarget = fCameraAngle;
	float fCameraPitch = 5.5f;
	float fCameraZoom = 16.0f;
	float fCameraZoomTarget = fCameraZoom;

	float cinematicZoom = 60.0f;
	float gameplayZoom = 30.0f;

	float timeBeforeUpdate;
	float timeBetweenUpdates = .1f;

public:
	void UpdateCursorPosition() {
		if (GetKey(olc::Key::LEFT).bHeld) vCursor.x--;
		if (GetKey(olc::Key::RIGHT).bHeld) vCursor.x++;
		if (GetKey(olc::Key::UP).bHeld) vCursor.y--;
		if (GetKey(olc::Key::DOWN).bHeld) vCursor.y++;

		timeBeforeUpdate = timeBetweenUpdates;
	}

	bool OnUserCreate() override
	{
		rendSelect.Load("./gfx/dng_select.png");
		rendPlayer.Load("./gfx/player.png");
		rendAllWalls.Load("./gfx/oldDungeon.png");

		if (world.size.x == 0) { // if there is no world data create a demo world
			world.Create(64, 64);

			for (int y = 0; y < world.size.y; y++)
				for (int x = 0; x < world.size.x; x++)
				{
					world.GetCell({ x, y }).wall = (y == 0 || y == world.size.y - 1 || x == 0 || x == world.size.x - 1);
					world.GetCell({ x, y }).id[Face::Floor] = olc::vi2d{ 0, 2 } *vTileSize;
					world.GetCell({ x, y }).id[Face::Top] = olc::vi2d{ 0, 0 } *vTileSize;
					world.GetCell({ x, y }).id[Face::North] = olc::vi2d{ 0, 1 } *vTileSize;
					world.GetCell({ x, y }).id[Face::South] = olc::vi2d{ 0, 1 } *vTileSize;
					world.GetCell({ x, y }).id[Face::West] = olc::vi2d{ 0, 1 } *vTileSize;
					world.GetCell({ x, y }).id[Face::East] = olc::vi2d{ 0, 1 } *vTileSize;
				}
		}

		fCameraAngle = 3.14159f * 1.75f;
		fCameraAngleTarget = fCameraAngle;
		fCameraZoom = gameplayZoom;
		fCameraZoomTarget = fCameraZoom;

		vCameraPos = { vCursor.x + 0.5f, vCursor.y + 0.5f };

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		// Grab mouse for convenience
		olc::vi2d vMouse = { GetMouseX(), GetMouseY() };

		// QZ Keys to zoom in or out
		if (GetKey(olc::Key::Q).bHeld) fCameraZoomTarget = cinematicZoom;
		if (GetKey(olc::Key::Z).bHeld)  fCameraZoomTarget = gameplayZoom;

		// Smooth camera
		float smoothing = 10.0f * fElapsedTime;

		fCameraAngle += (fCameraAngleTarget - fCameraAngle) * smoothing;
		fCameraZoom += (fCameraZoomTarget - fCameraZoom) * smoothing;

		// Arrow keys to move the selection cursor around map (boundary checked)

		olc::vi2d lastCursorPos = { vCursor.x, vCursor.y };

		if (GetKey(olc::Key::LEFT).bPressed) {
			vCursor.x--;
			timeBeforeUpdate = timeBetweenUpdates;
		}
		if (GetKey(olc::Key::RIGHT).bPressed) {
			vCursor.x++;
			timeBeforeUpdate = timeBetweenUpdates;
		}
		if (GetKey(olc::Key::UP).bPressed) {
			vCursor.y--;
			timeBeforeUpdate = timeBetweenUpdates;
		}
		if (GetKey(olc::Key::DOWN).bPressed) {
			vCursor.y++;
			timeBeforeUpdate = timeBetweenUpdates;
		}

		if (timeBeforeUpdate <= 0)
			UpdateCursorPosition();
		timeBeforeUpdate -= fElapsedTime;

		if (vCursor.x < 0) vCursor.x = 0;
		if (vCursor.y < 0) vCursor.y = 0;
		if (vCursor.x >= world.size.x) vCursor.x = world.size.x - 1;
		if (vCursor.y >= world.size.y) vCursor.y = world.size.y - 1;

		if (world.GetCell(vCursor).wall) {
			vCursor = lastCursorPos;
		}

		// Position camera in world
		vCameraPos = { vCursor.x + 0.5f, vCursor.y + 0.5f };
		vCameraPos *= fCameraZoom;

		// Rendering

		// 1) Create dummy cube to extract visible face information
		// Cull faces that cannot be seen
		std::array<vec3d, 8> cullCube = CreateCube({ 0, 0 }, fCameraAngle, fCameraPitch, fCameraZoom, { vCameraPos.x, 0.0f, vCameraPos.y });
		CalculateVisibleFaces(cullCube);

		// 2) Get all visible sides of all visible "tile cubes"
		std::vector<sQuad> vQuads;
		for (int y = 0; y < world.size.y; y++)
			for (int x = 0; x < world.size.x; x++)
				GetFaceQuads({ x, y }, fCameraAngle, fCameraPitch, fCameraZoom, { vCameraPos.x, 0.0f, vCameraPos.y }, vQuads);

		// 3) Sort in order of depth, from farthest away to closest
		std::sort(vQuads.begin(), vQuads.end(), [](const sQuad& q1, const sQuad& q2)
			{
				float z1 = (q1.points[0].z + q1.points[1].z + q1.points[2].z + q1.points[3].z) * 0.25f;
				float z2 = (q2.points[0].z + q2.points[1].z + q2.points[2].z + q2.points[3].z) * 0.25f;
				return z1 < z2;
			});

		// 4) Iterate through all "tile cubes" and draw their visible faces
		Clear(olc::BLACK);
		for (auto& q : vQuads)
			DrawPartialWarpedDecal
			(
				rendAllWalls.decal,
				{ {q.points[0].x, q.points[0].y}, {q.points[1].x, q.points[1].y}, {q.points[2].x, q.points[2].y}, {q.points[3].x, q.points[3].y} },
				q.tile,
				vTileSize
			);

		// 6) Draw selection "tile cube"

		sQuad playerQuad;
		GetSpriteQuads(vCursor, fCameraAngle, fCameraPitch, fCameraZoom, { vCameraPos.x, 0.0f, vCameraPos.y }, playerQuad);

		DrawWarpedDecal(rendPlayer.decal,
			{
				{
					playerQuad.points[0].x,
					playerQuad.points[0].y
				},
				{
					playerQuad.points[1].x,
					playerQuad.points[1].y
				},
				{
					playerQuad.points[2].x,
					playerQuad.points[2].y
				},
				{
					playerQuad.points[3].x,
					playerQuad.points[3].y
				}
			});

		// 7) Draw some debug info
		//DrawStringDecal({ 0,0 }, "User: " + std::to_string(vCursor.x) + ", " + std::to_string(vCursor.y), olc::YELLOW, { 0.5f, 0.5f });

		// Graceful exit if user is in full screen mode
		return !GetKey(olc::Key::ESCAPE).bPressed;
	}
};