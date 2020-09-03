#pragma once
#include "olcPixelGameEngine.h"
#include "CoreData.h"

class Drawable {
public:
	olc::Decal* decal;
	sQuad quad;
	olc::vf2d scale;
};

class olcDungeonPlayer : public olc::PixelGameEngine
{
public:
	olcDungeonPlayer()
	{
		sAppName = "Dungeon Explorer";
	}

	olc::vi2d playerPos = { 0.0f, 0.0f };
	olc::vi2d targetPlayerPos = { 0.0f, 0.0f };

	const float gameplayAngle = pi * 1.75f;
	float cinematicZoom = 60.0f;
	float gameplayZoom = 30.0f;

	olc::vf2d vCameraPos = { 0.0f, 0.0f };
	float fCameraAngle = gameplayAngle;
	float fCameraAngleTarget = fCameraAngle;
	float fCameraPitch = 5.8f;
	float fCameraZoom = gameplayZoom;
	float fCameraZoomTarget = fCameraZoom;

	float smoothing;

	float timeBeforeUpdate;
	float timeBetweenUpdates = .1f;

	bool movingCamera;
	olc::vi2d clickPos;

public:
	std::vector<Drawable> drawables;

	void UpdateOnInterval() {
		if (timeBeforeUpdate > 0) return;

		if (targetPlayerPos.x != playerPos.x || targetPlayerPos.y != playerPos.y)
		{
			int x = (targetPlayerPos.x - playerPos.x);
			int y = (targetPlayerPos.y - playerPos.y);

			if (x > 1) x = 1; if (x < -1) x = -1;
			if (y > 1) y = 1; if (y < -1) y = -1;

			playerPos.x += x;
			playerPos.y += y;
		}

		timeBeforeUpdate = timeBetweenUpdates;
	}

	void DrawQuad(olc::Decal* decal, sQuad quad) {
		Drawable drawable;
		drawable.decal = decal;
		drawable.quad = quad;
		drawable.scale = vTileSize;

		// We should calculate if this tile is even visable

		drawables.push_back(drawable);
	}

	void DrawDrawables() {
		std::sort(drawables.begin(), drawables.end(), [](const Drawable& a, const Drawable& b)
			{
				auto q1 = a.quad;
				auto q2 = b.quad;

				float z1 = (q1.points[0].z + q1.points[1].z + q1.points[2].z + q1.points[3].z) * 0.25f;
				float z2 = (q2.points[0].z + q2.points[1].z + q2.points[2].z + q2.points[3].z) * 0.25f;
				return z1 < z2;
			});

		for (auto drawbale : drawables) {
			DrawPartialWarpedDecal
			(
				drawbale.decal,
				{
					{drawbale.quad.points[0].x, drawbale.quad.points[0].y},
					{drawbale.quad.points[1].x, drawbale.quad.points[1].y},
					{drawbale.quad.points[2].x, drawbale.quad.points[2].y},
					{drawbale.quad.points[3].x, drawbale.quad.points[3].y}
				},
				drawbale.quad.tile,
				drawbale.scale
			);
		}

		drawables.clear();
	}

	bool OnUserCreate() override
	{
		rendPlayer.Load("./gfx/player.png");
		rendSelect.Load("./gfx/dng_select.png");
		rendAllWalls.Load("./gfx/" + tileMapLocation + ".png");

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

		fCameraAngle = gameplayAngle;
		fCameraAngleTarget = fCameraAngle;
		fCameraZoom = gameplayZoom;
		fCameraZoomTarget = fCameraZoom;

		playerPos = { world.playerSpawnPoint.x, world.playerSpawnPoint.y };
		targetPlayerPos = { playerPos.x, playerPos.y };
		vCursor = { playerPos.x, playerPos.y };
		vCameraPos = { playerPos.x + 0.5f, playerPos.y + 0.5f };
		vCameraPos *= fCameraZoom;

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		// Grab mouse for convenience
		olc::vi2d mouseCellPos =
		{
			(int)(((float)GetMouseX() / (float)screenWidth) * world.size.x),
			(int)(((float)GetMouseY() / (float)screenHeight) * world.size.y)
		};

		// Send the mousePos into world space
		// Somehow...
		olc::vf2d worldMousePos =
		{
			(vCameraPos.x / fCameraZoom) + ((int)mouseCellPos.x / (float)cos(fCameraAngle)),
			(vCameraPos.y / fCameraZoom) + ((int)mouseCellPos.y / (float)sin(fCameraAngle))
		};

		// QZ Keys to zoom in or out
		if (GetKey(olc::Key::Q).bHeld) fCameraZoomTarget = cinematicZoom;
		if (GetKey(olc::Key::Z).bHeld)  fCameraZoomTarget = gameplayZoom;

		// Smooth camera
		smoothing = 10.0f * fElapsedTime;

		fCameraAngle += (fCameraAngleTarget - fCameraAngle) * smoothing;
		fCameraZoom += (fCameraZoomTarget - fCameraZoom) * smoothing;

		// Arrow keys to move the selection cursor around map (boundary checked)

		timeBeforeUpdate -= fElapsedTime;
		UpdateOnInterval();

		if (GetKey(olc::Key::A).bHeld) vCameraPos.x--;
		if (GetKey(olc::Key::D).bHeld) vCameraPos.x++;
		if (GetKey(olc::Key::W).bHeld) vCameraPos.y--;
		if (GetKey(olc::Key::S).bHeld) vCameraPos.y++;

		// Move player with mouse
		if (GetMouse(0).bPressed)
		{
			clickPos = mouseCellPos;
		}

		if (GetMouse(0).bHeld)
		{
			movingCamera = (clickPos.x != mouseCellPos.x || clickPos.y != mouseCellPos.y);

			if (movingCamera)
			{
				vCameraPos.x += (clickPos.x - mouseCellPos.x) * smoothing;
				vCameraPos.y += (clickPos.y - mouseCellPos.y) * smoothing;
			}
		}

		if (GetMouse(0).bReleased)
		{
			if (movingCamera == false)
				if (world.GetCell(mouseCellPos).wall == false)
					targetPlayerPos = mouseCellPos;

			movingCamera = false;
		}

		if (vCursor.x < 0) vCursor.x = 0;
		if (vCursor.y < 0) vCursor.y = 0;
		if (vCursor.x >= world.size.x) vCursor.x = world.size.x - 1;
		if (vCursor.y >= world.size.y) vCursor.y = world.size.y - 1;

		// Position camera in world
		//vCameraPos = { vCursor.x + 0.5f, vCursor.y + 0.5f };
		//vCameraPos *= fCameraZoom;

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
			DrawQuad(rendAllWalls.decal, q);

		// Place a cursor in world
		vQuads.clear();
		GetFaceQuads(mouseCellPos, fCameraAngle, fCameraPitch, fCameraZoom, { vCameraPos.x, 0.0f, vCameraPos.y }, vQuads);
		for (auto& q : vQuads)
			DrawQuad(rendSelect.decal, q);

		vQuads.clear();
		GetFaceQuads(worldMousePos, fCameraAngle, fCameraPitch, fCameraZoom, { vCameraPos.x, 0.0f, vCameraPos.y }, vQuads);
		for (auto& q : vQuads)
			DrawQuad(rendSelect.decal, q);

		// Numpad keys used to rotate camera to fixed angles
		if (GetKey(olc::Key::NP1).bHeld) fCameraAngleTarget -= pi * fElapsedTime;
		if (GetKey(olc::Key::NP2).bHeld) fCameraAngleTarget = gameplayAngle;
		if (GetKey(olc::Key::NP3).bHeld) fCameraAngleTarget += pi * fElapsedTime;
		if (GetKey(olc::Key::NP5).bHeld) fCameraAngleTarget = 0;

		// 6) Draw player
		vQuads.clear();
		GetSpriteQuads(playerPos, fCameraAngle, fCameraPitch, fCameraZoom, { vCameraPos.x, 0.0f, vCameraPos.y }, vQuads);

		DrawQuad(rendPlayer.decal, vQuads.back());
		DrawDrawables();

		// 7) Draw some debug info
		//DrawStringDecal({ 0,0 }, "User: " + std::to_string(vCursor.x) + ", " + std::to_string(vCursor.y), olc::YELLOW, { 0.5f, 0.5f });
		DrawStringDecal({ 0,0 }, "Mouse Cell: " + std::to_string(mouseCellPos.x) + ", " + std::to_string(mouseCellPos.y), olc::YELLOW, { 0.5f, 0.5f });
		DrawStringDecal({ 0,10 }, "World Cell: " + std::to_string(worldMousePos.x) + ", " + std::to_string(worldMousePos.y), olc::YELLOW, { 0.5f, 0.5f });

		// Debug let the user run the editor
		if (GetKey(olc::Key::CTRL).bHeld && GetKey(olc::Key::F5).bReleased) {
			runEditor = true;
			return false;
		}

		// Graceful exit if user is in full screen mode
		return !GetKey(olc::Key::ESCAPE).bPressed;
	}
};