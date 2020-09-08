#pragma once
#include "olcPixelGameEngine.h"
#include "CoreData.h"
#include "luaEditor.h"
#include "FileManager.h"
#include "UIObjects.h"

class Drawable {
public:
	olc::Decal* decal;
	sQuad quad;
};

class olcDungeonPlayer : public olc::PixelGameEngine
{
public:
	olcDungeonPlayer()
	{
		sAppName = "Dungeon Explorer";
	}

	olc::vf2d worldMousePos;

	olc::vi2d playerPos = { 0.0f, 0.0f };
	olc::vi2d targetPlayerPos = { 0.0f, 0.0f };

	float* presetAngles;
	int angleIndex = 0;

	float cinematicZoom = 60.0f;
	float gameplayZoom = 30.0f;

	olc::vf2d vCameraPos = { 0.0f, 0.0f };
	float fCameraAngle = pi * 1.75f;
	float fCameraPitch = 5.8f;
	float fCameraZoom = gameplayZoom;
	float fCameraZoomTarget = fCameraZoom;

	float smoothing;

	float timeBeforeUpdate;
	float timeBetweenUpdates = .1f;

	float menuTime;

	bool canChange = true;
	bool creatingNew;

	bool invert = true;

	std::string sInputBuffer;

	std::string FlushInput(std::string target = "")
	{
		std::string ret = sInputBuffer;
		if (target == sInputBuffer) sInputBuffer.clear();
		return ret;
	}

	olc::vf2d GetMousePos() {
		olc::vf2d mouse =
		{
			(float)GetMouseX() / (float)screenWidth,
			(float)GetMouseY() / (float)screenHeight
		};

		// Send the mousePos into world space
		// Somehow...
		olc::vf2d cameraCellPos =
		{
			(vCameraPos.x / fCameraZoom) / 2.0f,
			(vCameraPos.y / fCameraZoom) / 2.0f
		};

		olc::vf2d worldMousePos =
		{
			cameraCellPos.x + (world.size.x * mouse.x),
			cameraCellPos.y + (world.size.y * mouse.y)
		};

		return worldMousePos;
	}

public:
	std::vector<Drawable> drawables;

	void DisplayMessages(float elapsedTime) {
		olc::vi2d messagePos = { 200, 0 };

		std::vector<UIObject::TextMessage> n_messages;

		for (auto message : textMessages)
		{
			DrawStringDecal(messagePos, message.text, olc::YELLOW, { 1, 1 });
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

	void TryLoadMap(std::string map) {
		if (map == "New...") {
			// Get input from the user to set the name of the map
			sInputBuffer = "";
			creatingNew = true;
		}
		else {
			ConstructMap(map);
			CurrentScene = map.c_str();
			//ToggleMenu();
		}

		CreateMessage("Loaded " + CurrentScene, 1);
	}

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

		invert = !invert;

		timeBeforeUpdate = timeBetweenUpdates;
	}

	void DrawQuad(olc::Decal* decal, sQuad quad) {
		Drawable drawable;
		drawable.decal = decal;
		drawable.quad = quad;

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
				vTileSize
			);
		}

		drawables.clear();
	}

	void ToggleMenu() {
		buttons.clear();
		GetFiles("luaScripts\\maps");

		menuOpen = !menuOpen;

		if (menuOpen == true) {
			olc::vi2d screenMiddle = { screenWidth / 2, screenHeight / 2 };

			UIObject::Button quitButton(screenMiddle.x - 50, screenMiddle.y, 100, 10, "Exit Application");
			quitButton.onButtonPressed.push_back([] { runApplication = false; });
			UIObject::Button resumeButton(screenMiddle.x - 50, screenMiddle.y, 100, 10, "Return");
			resumeButton.onButtonPressed.push_back([]
				{
					menuOpen = false;
					buttons.clear();
				});

			UIObject::Button editorButton(screenMiddle.x - 50, screenMiddle.y, 100, 10, "Toggle Editor");
			editorButton.onButtonPressed.push_back([] { runEditor = !runEditor; });

			buttons.push_back(resumeButton);
			buttons.push_back(editorButton);
			buttons.push_back(quitButton);
		}
		else buttons.clear();
	}

	void SmoothCameraMovement(float elapsedTime) {
		// QZ Keys to zoom in or out
		if (GetKey(olc::Key::Q).bPressed) fCameraZoomTarget = cinematicZoom;
		if (GetKey(olc::Key::Z).bPressed) fCameraZoomTarget = gameplayZoom;

		// Smooth camera
		smoothing = 10.0f * elapsedTime;

		if (fCameraZoom > fCameraZoomTarget + 0.1 || fCameraZoom < fCameraZoomTarget - 0.1) {
			fCameraZoom += (fCameraZoomTarget - fCameraZoom) * smoothing;
			RecenterCamera();
		}
		else fCameraZoom = fCameraZoomTarget;

		if (GetKey(olc::Key::A).bHeld) vCameraPos.x -= cos(fCameraAngle);
		if (GetKey(olc::Key::D).bHeld) vCameraPos.x += cos(fCameraAngle);
		if (GetKey(olc::Key::W).bHeld) vCameraPos.y -= cos(fCameraAngle);
		if (GetKey(olc::Key::S).bHeld) vCameraPos.y += cos(fCameraAngle);

		// Camera controls
		if (GetKey(olc::Key::NP1).bHeld) fCameraAngle -= pi * elapsedTime;
		if (GetKey(olc::Key::NP2).bPressed) fCameraAngle = presetAngles[0];
		if (GetKey(olc::Key::NP3).bHeld) fCameraAngle += pi * elapsedTime;
		if (GetKey(olc::Key::NP5).bHeld) fCameraAngle = 0;

		if (GetKey(olc::Key::LEFT).bPressed) {
			angleIndex--;
			if (angleIndex < 0) angleIndex = 3;
			fCameraAngle = presetAngles[angleIndex];
		}

		if (GetKey(olc::Key::RIGHT).bPressed) {
			angleIndex++;
			if (angleIndex > 3) angleIndex = 0;
			fCameraAngle = presetAngles[angleIndex];
		}
	}

	void Rendering() {
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
		GetFaceQuads(worldMousePos, fCameraAngle, fCameraPitch, fCameraZoom, { vCameraPos.x, 0.0f, vCameraPos.y }, vQuads);
		for (auto& q : vQuads)
			DrawQuad(rendSelect.decal, q);

		// 6) Draw player
		vQuads.clear();

		GetSpriteQuads((runEditor ? world.playerSpawnPoint : playerPos),
			fCameraAngle,
			fCameraPitch,
			fCameraZoom,
			{ vCameraPos.x, 0.0f, vCameraPos.y },
			vQuads);

		for (auto& q : vQuads)
			DrawQuad(rendPlayer.decal, q);

		DrawDrawables();

		if (menuOpen) return; // Stop line, everythying below this will only show when the menu is clsoed

		// 7) Draw some debug info
		if (runEditor) {
			olc::vi2d scale = { ScreenWidth(), 30 };
			FillRectDecal({ 0,0 }, scale, olc::BLACK);
			std::string editing = "Currently editing: ";
			DrawStringDecal({ 0,0 }, editing + CurrentScene, olc::YELLOW, { 0.5f, 0.5f });
			DrawStringDecal({ 0,10 }, "Cursor: " + std::to_string(vCursor.x) + ", " + std::to_string(vCursor.y), olc::YELLOW, { 0.5f, 0.5f });
			DrawStringDecal({ 0,20 }, "Angle: " + std::to_string(fCameraAngle) + ", " + std::to_string(fCameraPitch), olc::YELLOW, { 0.5f, 0.5f });
			//DrawPartialDecal({ 10, 30 }, rendAllWalls.decal, vTileCursor * vTileSize, vTileSize);
		}
		else {
			//DrawStringDecal({ 0,0 }, "User: " + std::to_string(vCursor.x) + ", " + std::to_string(vCursor.y), olc::YELLOW, { 0.5f, 0.5f });
			//DrawStringDecal({ 0,0 }, "Mouse Screen: " + std::to_string(mouse.x) + ", " + std::to_string(mouse.y), olc::YELLOW, { 0.5f, 0.5f });
			//DrawStringDecal({ 0,10 }, "Mouse Cell: " + std::to_string(mouseCellPos.x) + ", " + std::to_string(mouseCellPos.y), olc::YELLOW, { 0.5f, 0.5f });
			DrawStringDecal({ 0, 0 }, "World Cell: " + std::to_string(worldMousePos.x) + ", " + std::to_string(worldMousePos.y), olc::YELLOW, { 0.5f, 0.5f });
		}
	}

	void Editor(float elapsedTime) {
		olc::vi2d lastCursorPos = { vCursor.x, vCursor.y };

		vCursor = { (int)worldMousePos.x, (int)worldMousePos.y };

		// Edit mode - Selection from tile sprite sheet
		// TO BE IMPLEMENTED

		// zoom in or out
		if (GetMouseWheel() > 0) fCameraZoom += 150.0f * elapsedTime;
		if (GetMouseWheel() < 0) fCameraZoom -= 150.0f * elapsedTime;

		// Numeric keys apply selected tile to specific face
		//if (GetKey(olc::Key::K1).bPressed) world.GetCell(vCursor).id[Face::North] = vTileCursor * vTileSize;
		//if (GetKey(olc::Key::K2).bPressed) world.GetCell(vCursor).id[Face::East] = vTileCursor * vTileSize;
		//if (GetKey(olc::Key::K3).bPressed) world.GetCell(vCursor).id[Face::South] = vTileCursor * vTileSize;
		//if (GetKey(olc::Key::K4).bPressed) world.GetCell(vCursor).id[Face::West] = vTileCursor * vTileSize;
		//if (GetKey(olc::Key::K5).bPressed) world.GetCell(vCursor).id[Face::Floor] = vTileCursor * vTileSize;
		//if (GetKey(olc::Key::K6).bPressed) world.GetCell(vCursor).id[Face::Top] = vTileCursor * vTileSize;

		// Set player spawn
		if (GetKey(olc::Key::SPACE).bPressed) {
			world.playerSpawnPoint = vCursor;
			CreateMessage("Set player spawn: " + std::to_string(vCursor.x) + "," +
				std::to_string(vCursor.y), 2);
		}

		if (lastCursorPos.x != vCursor.x || lastCursorPos.y != vCursor.y) {
			canChange = true;
		}

		// Place block with space
		if (GetMouse(0).bHeld && canChange)
		{
			world.GetCell(vCursor).wall = !world.GetCell(vCursor).wall;
			canChange = false;
		}

		if (GetMouse(0).bReleased && !canChange) canChange = true;

		// Save key
		if (GetKey(olc::Key::F3).bPressed) {
			SaveMapData(world);
			CreateMessage("Saved.", 1);
		}
	}

	void Game(float elapsedTime) {
		// Move player with mouse
		if (GetMouse(0).bReleased)
		{
			if (world.GetCell(worldMousePos).wall == false)
				targetPlayerPos = worldMousePos;
		}
	}

	void RecenterCamera() {
		vCameraPos = { playerPos.x + 0.5f, playerPos.y + 0.5f };
		vCameraPos *= fCameraZoom;
	}

	bool OnUserCreate() override
	{
		rendPlayer.Load("./gfx/player.png");
		rendSelect.Load("./gfx/dng_select.png");
		rendAllWalls.Load("./gfx/" + tileMapLocation + ".png");

		if (world.size.x == 0) { // if there is no world data create a demo world
			std::cout << "Creating demo world." << std::endl;
			world.Create(32, 32);

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

		presetAngles = new float[4]
		{
			pi * 1.75f,
			pi * 1.25f,
			pi * 0.75f,
			pi * 0.25f
		};

		fCameraAngle = presetAngles[angleIndex];
		fCameraZoom = gameplayZoom;
		fCameraZoomTarget = fCameraZoom;

		playerPos = { world.playerSpawnPoint.x, world.playerSpawnPoint.y };
		targetPlayerPos = { playerPos.x, playerPos.y };
		vCursor = { playerPos.x, playerPos.y };
		RecenterCamera();

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		timeBeforeUpdate -= fElapsedTime;
		UpdateOnInterval();

		// Render world
		Rendering();

		if (creatingNew) {
			std::string title = "New map name: ";
			DrawStringDecal({ 0,0 }, title + sInputBuffer + (invert ? '_' : ' '), olc::YELLOW, { 0.5f, 0.5f });

			int shift = !GetKey(olc::Key::SHIFT).bHeld ? 96 : 64;

			if (GetKey(olc::Key::BACK).bPressed) {
				std::string old;
				for (size_t i = 0; i < sInputBuffer.length(); i++)
					if (i != sInputBuffer.length() - 1)old += sInputBuffer[i];

				sInputBuffer = "";

				for (size_t i = 0; i < old.length(); i++)
					sInputBuffer += old[i];

				return runApplication;
			}

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
				creatingNew = false;

				CreateMessage("Canceled.", 1);

				FlushInput();
				ToggleMenu();
			}

			if (GetKey(olc::Key::ENTER).bReleased || GetKey(olc::Key::RETURN).bReleased) {
				creatingNew = false;
				CurrentScene = sInputBuffer.c_str();
				ConstructMap("templateMap");

				CreateMessage("Created new map: " + CurrentScene, 1);

				FlushInput();
				ToggleMenu();
			}

			return runApplication;
		}

		// Grab mouse for convenience
		olc::vi2d vMouse = { GetMouseX(), GetMouseY() };

		if (GetKey(olc::Key::ESCAPE).bPressed) ToggleMenu();

		if (menuOpen) {
			menuTime = .15f;

			// Display menu buttons
			int padding = 5;
			int offsetY = 0;

			for (auto button : buttons)
			{
				if (GetKey(olc::F1).bPressed) developerMode = !developerMode;

				if (button.text == "Toggle Editor" && !developerMode) continue;

				if (button.MouseOver(vMouse.x, vMouse.y - offsetY))
				{
					// Use button
					if (GetMouse(0).bPressed) {
						// do a thing
						button.Activate();
					}
				}

				button.Draw(this, offsetY);
				offsetY += button.height + padding;
			}

			if (runEditor) {
				// Display loadable levels
				offsetY = 10;

				DrawStringDecal({ 0,0 }, "Available Levels:", olc::YELLOW, { 0.75f, 0.75f });

				for (int n = 0; n < noPathLength; n++) {
					olc::vi2d pos = { 0, offsetY };

					if (vMouse.x < 100 && vMouse.y < offsetY + 5 && vMouse.y > offsetY - 5) {
						FillRectDecal(pos, { 100, 7 }, olc::RED);
						if (GetMouse(0).bPressed)
						{
							// Trigger load
							TryLoadMap(noPaths[n]);
						}
					}

					DrawStringDecal(pos, noPaths[n], olc::YELLOW, { 0.5f, 0.5f });
					offsetY += 10;
				}
			}
		}

		DisplayMessages(fElapsedTime);

		if (menuTime > 0) {
			menuTime -= fElapsedTime;
			return runApplication;
		}

		// Smooth camera
		SmoothCameraMovement(fElapsedTime);
		worldMousePos = GetMousePos();

		if (runEditor) Editor(fElapsedTime);
		else Game(fElapsedTime);

		// Graceful exit if user is in full screen mode
		return runApplication;
	}
};