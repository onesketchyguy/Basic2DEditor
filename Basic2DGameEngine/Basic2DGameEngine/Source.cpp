/*
	Dungeon Warping via Orthographic Projections
	"For my Mother-In-Law, you will be missed..." - javidx9

	License (OLC-3)
	~~~~~~~~~~~~~~~

	Copyright 2018-2020 OneLoneCoder.com

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions
	are met:

	1. Redistributions or derivations of source code must retain the above
	copyright notice, this list of conditions and the following disclaimer.

	2. Redistributions or derivative works in binary form must reproduce
	the above copyright notice. This list of conditions and the following
	disclaimer must be reproduced in the documentation and/or other
	materials provided with the distribution.

	3. Neither the name of the copyright holder nor the names of its
	contributors may be used to endorse or promote products derived
	from this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
	"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
	LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
	A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
	HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
	SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
	LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
	DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
	THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
	OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	Relevant Video: https://youtu.be/Ql5VZGkL23o

	Links
	~~~~~
	YouTube:	https://www.youtube.com/javidx9
				https://www.youtube.com/javidx9extra
	Discord:	https://discord.gg/WhwHUMV
	Twitter:	https://www.twitter.com/javidx9
	Twitch:		https://www.twitch.tv/javidx9
	GitHub:		https://www.github.com/onelonecoder
	Patreon:	https://www.patreon.com/javidx9
	Homepage:	https://www.onelonecoder.com

	Community Blog: https://community.onelonecoder.com

	Author
	~~~~~~
	David Barr, aka javidx9, ©OneLoneCoder 2018, 2019, 2020
*/

#define OLC_PGE_APPLICATION
#include "CoreData.h"
#include "Editor.h"
#include "Game.h"
#include "luaEditor.h"

int pScale = 2;

void ConstructEditor() {
	olcDungeonEditor editor;
	if (editor.Construct(screenWidth, screenHeight, pScale, pScale, false))
		editor.Start();
}

void ConstructGame() {
	olcDungeonPlayer game;
	if (game.Construct(screenWidth, screenHeight, pScale, pScale, false))
		game.Start();
}

void SetupSettings()
{
	lua_State* L = luaL_newstate();
	luaL_openlibs(L);

	int mapX = 0, mapY = 0;
	int sX = 0, sY = 0;

	std::string mapLocation = "luaScripts\\settings.lua";

	if (CheckLua(L, luaL_dofile(L, mapLocation.c_str()))) {
		// Set wether or not to run the editor
		lua_getglobal(L, "runEditor");
		if (lua_isboolean(L, -1)) {
			runEditor = (bool)lua_toboolean(L, -1);
			std::cout << "runEditor initialized to: " << runEditor << std::endl;
			lua_pop(L, -1);
		}
		// Which level should be the first level
		lua_getglobal(L, "initScene");
		if (lua_isstring(L, -1)) {
			CurrentScene = (std::string)lua_tostring(L, -1);
			std::cout << "CurrentScene initalized to: " << CurrentScene << std::endl;
			lua_pop(L, -1);
		}
	}
	else {
		// Log Error
		std::string errorMessage = lua_tostring(L, -1);
		std::cout << errorMessage << std::endl;
	}

	// Close this lua instance once we have completed as we won't continue using it.
	lua_close(L);

	runEditor = false;
}

int main()
{
	SetupSettings();
	ConstructMap(CurrentScene);

	screenHeight = 360;
	screenWidth = 440;

	while (true) {
		if (runEditor == true) {
			ConstructEditor();

			if (runEditor == false) break;

			ConstructGame();
		}
		else {
			ConstructGame();

			if (runEditor) continue;

			break;
		}
	}

	return 0;
}