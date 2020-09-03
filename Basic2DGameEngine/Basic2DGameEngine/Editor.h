#pragma once

#include "olcPixelGameEngine.h"
#include "CoreData.h"
#include "luaEditor.h"
#include "FileManager.h"
#include "UIObjects.h"

class olcDungeonEditor : public olc::PixelGameEngine
{
public:
	olcDungeonEditor()
	{
		sAppName = "Dungeon Explorer";
	}

	olc::vf2d vCameraPos = { 0.0f, 0.0f };
	float fCameraAngle = 0.0f;
	float fCameraAngleTarget = fCameraAngle;
	float fCameraPitch = 5.5f;
	float fCameraZoom = 16.0f;

	olc::vi2d vTileCursor = { 0,0 };

	float timeBeforeUpdate;
	float timeBetweenUpdates = .1f;

	bool canChange = true;
	bool loading = false;
	bool creatingNew;

	bool invert = true;

	std::string sInputBuffer;

	std::string FlushInput(std::string target = "")
	{
		std::string ret = sInputBuffer;
		if (target == sInputBuffer) sInputBuffer.clear();
		return ret;
	}

public:

	void DisplayMessages(float elapsedTime) {
		olc::vi2d messagePos = { 200, 0 };

		std::vector<UIObject::TextMessage> n_messages;

		for (auto message : textMessages)
		{
			DrawStringDecal(messagePos, message.text, olc::VERY_DARK_CYAN, { 1, 1 });
			messagePos.y += 20;
			message.timeRemaining -= elapsedTime;

			if (message.timeRemaining > 0) {
				// Retain this message if there is still remaining time
				n_messages.push_back(message);
			}
		}

		textMessages.clear();
		textMessages = n_messages;
	}

	void UpdateOnInterval() {
		if (timeBeforeUpdate > 0) return;

		if (GetKey(olc::Key::LEFT).bHeld) vCursor.x--;
		if (GetKey(olc::Key::RIGHT).bHeld) vCursor.x++;
		if (GetKey(olc::Key::UP).bHeld) vCursor.y--;
		if (GetKey(olc::Key::DOWN).bHeld) vCursor.y++;

		invert = !invert;

		timeBeforeUpdate = timeBetweenUpdates;
	}

	void TryLoadMap(std::string map) {
		if (map == "New...") {
			// Get input from the user to set the name of the map
			sInputBuffer = "";
			creatingNew = true;
		}
		else {
			ConstructMap(map);
			CurrentScene = map.c_str();
			loading = false;
		}

		CreateMessage("Loaded " + CurrentScene, 1);
	}

	bool OnUserCreate() override
	{
		runEditor = false;

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

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		olc::vi2d lastCursorPos = { vCursor.x, vCursor.y };

		timeBeforeUpdate -= fElapsedTime;

		UpdateOnInterval();

		if (creatingNew) {
			std::string title = "New map name: ";
			DrawStringDecal({ 0,0 }, title + sInputBuffer + (invert ? '_' : ' '), olc::YELLOW, { 0.5f, 0.5f });

			int shift = !GetKey(olc::Key::SHIFT).bHeld ? 96 : 64;

			for (int i = 1; i <= 26; i++)
			{
				if (GetKey((olc::Key)i).bPressed)
				{
					char ch = i + shift;
					sInputBuffer += ch;
				}
			}

			if (GetKey(olc::Key::SPACE).bReleased) {
				sInputBuffer += ' ';
			}

			if (GetKey(olc::Key::ESCAPE).bReleased) {
				// Cancel
				loading = false;
				creatingNew = false;

				CreateMessage("Canceled.", 1);

				FlushInput();
			}

			if (GetKey(olc::Key::ENTER).bReleased || GetKey(olc::Key::RETURN).bReleased) {
				creatingNew = false;
				loading = false;
				CurrentScene = sInputBuffer.c_str();
				ConstructMap("templateMap");

				CreateMessage("Created new map: " + CurrentScene, 1);

				FlushInput();
			}

			return true;
		}

		// Grab mouse for convenience
		olc::vi2d vMouse = { GetMouseX(), GetMouseY() };

		// Load key
		if (GetKey(olc::Key::F1).bPressed)
		{
			// Open a debug menu
			loading = !loading;
			if (loading) GetFiles("luaScripts\\maps");
		}

		if (loading) {
			// Display loadable levels
			int y = 5;

			for (int n = 0; n < noPathLength; n++) {
				olc::vi2d pos = { 0, y };

				if (vMouse.x < 100 && vMouse.y < y + 5 && vMouse.y > y - 5) {
					FillRectDecal(pos, { 100, 7 }, olc::RED);
					if (GetMouse(0).bPressed)
					{
						// Trigger load
						TryLoadMap(noPaths[n]);
					}
				}

				DrawStringDecal(pos, noPaths[n], olc::YELLOW, { 0.5f, 0.5f });
				y += 10;
			}

			return true;
		}

		// Edit mode - Selection from tile sprite sheet
		if (GetKey(olc::Key::TAB).bHeld)
		{
			DrawSprite({ 0, 0 }, rendAllWalls.sprite);
			DrawRect(vTileCursor * vTileSize, vTileSize);
			if (GetMouse(0).bPressed) vTileCursor = vMouse / vTileSize;
			return true;
		}

		// WS keys to tilt camera
		if (GetKey(olc::Key::W).bHeld) fCameraPitch += 1.0f * fElapsedTime;
		if (GetKey(olc::Key::S).bHeld) fCameraPitch -= 1.0f * fElapsedTime;

		// DA Keys to manually rotate camera
		if (GetKey(olc::Key::D).bHeld) fCameraAngleTarget += 1.0f * fElapsedTime;
		if (GetKey(olc::Key::A).bHeld) fCameraAngleTarget -= 1.0f * fElapsedTime;

		// zoom in or out
		if (GetMouseWheel() > 0) fCameraZoom += 150.0f * fElapsedTime;
		if (GetMouseWheel() < 0) fCameraZoom -= 150.0f * fElapsedTime;

		// Numpad keys used to rotate camera to fixed angles
		if (GetKey(olc::Key::NP2).bPressed) fCameraAngleTarget = 3.14159f * 0.0f;
		if (GetKey(olc::Key::NP1).bPressed) fCameraAngleTarget = 3.14159f * 0.25f;
		if (GetKey(olc::Key::NP4).bPressed) fCameraAngleTarget = 3.14159f * 0.5f;
		if (GetKey(olc::Key::NP7).bPressed) fCameraAngleTarget = 3.14159f * 0.75f;
		if (GetKey(olc::Key::NP8).bPressed) fCameraAngleTarget = 3.14159f * 1.0f;
		if (GetKey(olc::Key::NP9).bPressed) fCameraAngleTarget = 3.14159f * 1.25f;
		if (GetKey(olc::Key::NP6).bPressed) fCameraAngleTarget = 3.14159f * 1.5f;
		if (GetKey(olc::Key::NP3).bPressed) fCameraAngleTarget = 3.14159f * 1.75f;

		// Numeric keys apply selected tile to specific face
		if (GetKey(olc::Key::K1).bPressed) world.GetCell(vCursor).id[Face::North] = vTileCursor * vTileSize;
		if (GetKey(olc::Key::K2).bPressed) world.GetCell(vCursor).id[Face::East] = vTileCursor * vTileSize;
		if (GetKey(olc::Key::K3).bPressed) world.GetCell(vCursor).id[Face::South] = vTileCursor * vTileSize;
		if (GetKey(olc::Key::K4).bPressed) world.GetCell(vCursor).id[Face::West] = vTileCursor * vTileSize;
		if (GetKey(olc::Key::K5).bPressed) world.GetCell(vCursor).id[Face::Floor] = vTileCursor * vTileSize;
		if (GetKey(olc::Key::K6).bPressed) world.GetCell(vCursor).id[Face::Top] = vTileCursor * vTileSize;

		// Smooth camera
		fCameraAngle += (fCameraAngleTarget - fCameraAngle) * 10.0f * fElapsedTime;

		// Arrow keys to move the selection cursor around map (boundary checked)
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

		// Set player spawn
		if (GetKey(olc::Key::SPACE).bPressed && GetKey(olc::Key::SHIFT).bHeld) {
			world.playerSpawnPoint = vCursor;
			CreateMessage("Set player spawn: " + std::to_string(vCursor.x) + "," +
				std::to_string(vCursor.y), 2);
		}

		if (lastCursorPos.x != vCursor.x || lastCursorPos.y != vCursor.y) {
			canChange = true;
		}

		if (vCursor.x < 0) vCursor.x = 0;
		if (vCursor.y < 0) vCursor.y = 0;
		if (vCursor.x >= world.size.x) vCursor.x = world.size.x - 1;
		if (vCursor.y >= world.size.y) vCursor.y = world.size.y - 1;

		// Place block with space
		if (GetKey(olc::Key::SPACE).bHeld && canChange)
		{
			world.GetCell(vCursor).wall = !world.GetCell(vCursor).wall;
			canChange = false;
		}

		if (GetKey(olc::Key::SPACE).bReleased && !canChange) canChange = true;

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
		vQuads.clear();
		GetFaceQuads(vCursor, fCameraAngle, fCameraPitch, fCameraZoom, { vCameraPos.x, 0.0f, vCameraPos.y }, vQuads);
		for (auto& q : vQuads)
			DrawWarpedDecal(rendSelect.decal, { {q.points[0].x, q.points[0].y}, {q.points[1].x, q.points[1].y}, {q.points[2].x, q.points[2].y}, {q.points[3].x, q.points[3].y} });

		vQuads.clear();
		GetFaceQuads(world.playerSpawnPoint, fCameraAngle, fCameraPitch, fCameraZoom, { vCameraPos.x, 0.0f, vCameraPos.y }, vQuads);
		for (auto& q : vQuads)
			DrawWarpedDecal(rendPlayer.decal, { {q.points[0].x, q.points[0].y}, {q.points[1].x, q.points[1].y}, {q.points[2].x, q.points[2].y}, {q.points[3].x, q.points[3].y} });

		// Save key
		if (GetKey(olc::Key::F3).bPressed) {
			SaveMapData(world);
			CreateMessage("Saved.", 1);
		}

		// 7) Draw some debug info
		olc::vi2d scale = { ScreenWidth(), 30 };
		FillRectDecal({ 0,0 }, scale, olc::BLACK);
		std::string editing = "Currently editing: ";
		DrawStringDecal({ 0,0 }, editing + CurrentScene, olc::YELLOW, { 0.5f, 0.5f });
		DrawStringDecal({ 0,10 }, "Cursor: " + std::to_string(vCursor.x) + ", " + std::to_string(vCursor.y), olc::YELLOW, { 0.5f, 0.5f });
		DrawStringDecal({ 0,20 }, "Angle: " + std::to_string(fCameraAngle) + ", " + std::to_string(fCameraPitch), olc::YELLOW, { 0.5f, 0.5f });
		DrawPartialDecal({ 10, 30 }, rendAllWalls.decal, vTileCursor * vTileSize, vTileSize);

		DisplayMessages(fElapsedTime);

		// Enter playmode
		if (GetKey(olc::Key::F5).bHeld) {
			runEditor = true;
			return false;
		}

		// Graceful exit if user is in full screen mode
		return !GetKey(olc::Key::ESCAPE).bReleased;
	}
};