/*
*   This file is part of Universal-Manager
*   Copyright (C) 2019 VoltZ, Epicpkmn11, Flame, RocketRobz, TotallyNotGuy
*
*   This program is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*   Additional Terms 7.b and 7.c of GPLv3 apply to this file:
*       * Requiring preservation of specified reasonable legal notices or
*         author attributions in that material or in the Appropriate Legal
*         Notices displayed by works containing it.
*       * Prohibiting misrepresentation of the origin of that material,
*         or requiring that modified versions of such material be marked in
*         reasonable ways as different from the original version.
*/

#pragma once
#include "common.hpp"

// Spritesheets.
#include "gfx_main.h"
#include "gfx_region.h"

#define COLOR_BT_BG_BAD		0xFF402C1C
#define COLOR_BT_BG_NORMAL	0xFF503820
#define COLOR_BT_BG_HOVER	0xFF805038
#define COLOR_BT_TX_NORMAL	-1
#define COLOR_BT_TX_BAD	0xFF805038
#define GuiButtonColor(h,d)		d?(h?COLOR_BT_BG_HOVER:COLOR_BT_BG_NORMAL):COLOR_BT_BG_BAD
#define GuiButtonTextColor(h,d)	d?COLOR_BT_TX_NORMAL:COLOR_BT_TX_BAD

#define BUTTON_OK KEY_A
#define BUTTON_BACK KEY_B
#define BUTTON_DIR KEY_DOWN | KEY_UP | KEY_LEFT | KEY_RIGHT

namespace Gui {
	Result init(void);
	void exit(void);

	C3D_RenderTarget* target(gfxScreen_t t);

	void clearTextBufs(void);
	void gcls(C3D_RenderTarget * screen, u32 color);
	void sprite(C2D_SpriteSheet sheet, size_t key, int x, int y, float sx = 1, float sy = 1);
	void DrawScreen(void);
	void ScreenLogic(u32 hDown, u32 hHeld, touchPosition touch);
	void fadeEffects(void);
}
void Draw_ImageBlend(C2D_SpriteSheet sheet, size_t key, int x, int y, u32 color, float sx = 1, float sy = 1);
void Draw_ImageAlpha(C2D_SpriteSheet sheet, size_t key, int x, int y, u8 alpha, float sx = 1, float sy = 1);
void Draw_EndFrame(void);
void Draw_Text(float x, float y, float size, u32 color, const char *text);
void DrawStrBox(float x, float y, float size, u32 color, const char *text, float width, float maxwidth = 1);
void DrawStrBoxC(float x, float y, float size, u32 color, const char *text, float width, float maxwidth = 1);
void DrawStrBoxCC(float x, float y, float size, u32 color, const char *text, float width, float height);
void Draw_Textf(float x, float y, float size, u32 color, const char* text, ...);
void Draw_Text_Center(float x, float y, float size, u32 color, const char *text);
void Draw_Textf_Center(float x, float y, float size, u32 color, const char* text, ...);
void Draw_Text_Right(float x, float y, float size, u32 color, const char *text);
void Draw_Textf_Right(float x, float y, float size, u32 color, const char* text, ...);
void Draw_GetTextSize(float size, float *width, float *height, const char *text);
float Draw_GetTextWidth(float size, const char *text);
float Draw_GetTextHeight(float size, const char *text);
bool Draw_Rect(float x, float y, float w, float h, u32 color);
float math_abs(float val);
float percvalf(float at0, float at100, float perc);
int percval(int at0, int at100, float perc);
float math_pow2(float val);
float _1ddist(float x0,float x1);
float _2ddist(float x0,float y0,float x1, float y1);
