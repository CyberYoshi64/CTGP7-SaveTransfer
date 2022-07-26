#include "gui.hpp"
#include "transfer.hpp"

static C2D_SpriteSheet gfx_main;
static C2D_SpriteSheet gfx_region;
char errorstr[2048];
int errorcode=0;
int touchpt, touchpx, touchpy;
int touchot, touchox, touchoy;
int maincnt;
float millisec;
std::string keyboardIn;
C2D_TextBuf sizeBuf;
C2D_Font systemFont;
int CFGLang;
extern bool exiting;

extern bool transferMode;
extern char transferPath[];
extern u8 transferRegion;
extern bool transferIsSD;
extern u8 trnsfGameCartRegion;
extern u8 trnsfAvailRegion;
extern bool transferIncludeGhosts;

bool fadein = true, fadeout = false;
s16 fadealpha = 250;
u32 fadecolor;
C3D_RenderTarget *top;
C3D_RenderTarget *bottom;

u16 appMode;
int buttonSel;

void Gui::clearTextBufs(void) {
	C2D_TextBufClear(sizeBuf);
}
Result Gui::init(void) {
	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
	C2D_Prepare();

	u32 transFlags=	GX_TRANSFER_FLIP_VERT(false)|
					GX_TRANSFER_OUT_TILED(false)|
					GX_TRANSFER_RAW_COPY(false)|
					GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8)|
					GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB8)|
					GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO);

	top = C3D_RenderTargetCreate(240, 400, GPU_RB_RGBA8, GPU_RB_DEPTH16);
	C3D_RenderTargetClear(top, C3D_CLEAR_ALL, 0, 0);
	C3D_RenderTargetSetOutput(top, GFX_TOP, GFX_LEFT, transFlags);
	
	bottom = C3D_RenderTargetCreate(240, 320, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
	C3D_RenderTargetClear(bottom, C3D_CLEAR_ALL, 0, 0);
	C3D_RenderTargetSetOutput(bottom, GFX_BOTTOM, GFX_LEFT, transFlags);

	sizeBuf		= C2D_TextBufNew(4096);
	gfx_main	= C2D_SpriteSheetLoad("romfs:/gfx/gfx_main.t3x");
	gfx_region	= C2D_SpriteSheetLoad("romfs:/gfx/gfx_region.t3x");
	systemFont	= C2D_FontLoadSystem(CFG_REGION_EUR);
	return 0;
}
void Gui::exit(void) {
	if (gfx_main) C2D_SpriteSheetFree(gfx_main);
	if (gfx_region) C2D_SpriteSheetFree(gfx_region);
	C2D_TextBufDelete(sizeBuf);
	C2D_FontFree(systemFont);
	C2D_Fini();
	C3D_Fini();
}

void Gui::fadeEffects(void){
	if (fadein && fadealpha>0){
		fadealpha -= 25; fadeout=false;
	} else {
		fadein=false;
	}
	if (fadeout && fadealpha<255){
		fadealpha += 15; fadein=false;
	} else {
		fadeout=false;
	}
	fadealpha=C2D_Clamp(fadealpha, 0, 255);
}

void Gui::sprite(C2D_SpriteSheet sheet, size_t key, int x, int y, float sx, float sy) {
	if (key < 0 || key >= C2D_SpriteSheetCount(sheet)) return;
	C2D_DrawImageAt(C2D_SpriteSheetGetImage(sheet, key), x, y, 0.5f, nullptr, sx, sy);
}
void Gui::gcls(C3D_RenderTarget * screen, u32 color){
	C2D_TargetClear(screen,color);
}
void Draw_ImageBlend(C2D_SpriteSheet sheet, size_t key, int x, int y, u32 color, float sx, float sy) {
	if (key < 0 || key >= C2D_SpriteSheetCount(sheet)) return;
	C2D_ImageTint tint;
	C2D_PlainImageTint(&tint, color, 1);
	C2D_DrawImageAt(C2D_SpriteSheetGetImage(sheet, key), x, y, 0.5f, &tint, sx, sy);
}
void Draw_ImageAlpha(C2D_SpriteSheet sheet, size_t key, int x, int y, u8 alpha, float sx, float sy) {
	if (key < 0 || key >= C2D_SpriteSheetCount(sheet)) return;
	C2D_ImageTint tint;
	C2D_PlainImageTint(&tint, alpha<<24, 0);
	C2D_DrawImageAt(C2D_SpriteSheetGetImage(sheet, key), x, y, 0.5f, &tint, sx, sy);
}
void Draw_EndFrame(void) {
	C2D_TextBufClear(sizeBuf);
	C3D_FrameEnd(0);
}

void Draw_Text(float x, float y, float size, u32 color, const char *text) {
	C2D_Text c2d_text;
	C2D_TextFontParse(&c2d_text, systemFont, sizeBuf, text);
	C2D_TextOptimize(&c2d_text);
	C2D_DrawText(&c2d_text, C2D_WithColor | C2D_AlignLeft, x, y, 0.5f, size, size, color);
}

void DrawStrBox(float x, float y, float size, u32 color, const char *text, float width, float maxwidth = 1) {
	C2D_Text c2d_text;
	C2D_TextFontParse(&c2d_text, systemFont, sizeBuf, text);
	C2D_TextOptimize(&c2d_text);
	C2D_DrawText(&c2d_text, C2D_WithColor, x, y, 0.5f, C2D_Clamp(size*(width/Draw_GetTextWidth(size, text)),0.001, size * maxwidth), size, color);
}

void Draw_Text_Center(float x, float y, float size, u32 color, const char *text) {
	C2D_Text c2d_text;
	C2D_TextFontParse(&c2d_text, systemFont, sizeBuf, text);
	C2D_TextOptimize(&c2d_text);
	C2D_DrawText(&c2d_text, C2D_WithColor | C2D_AlignCenter, x, y, 0.5f, size, size, color);
}

void DrawStrBoxC(float x, float y, float size, u32 color, const char *text, float width, float maxwidth = 1) {
	C2D_Text c2d_text;
	C2D_TextFontParse(&c2d_text, systemFont, sizeBuf, text);
	C2D_TextOptimize(&c2d_text);
	float temp=C2D_Clamp(size*(width/Draw_GetTextWidth(size, text)),0.001, size * maxwidth);
	C2D_DrawText(&c2d_text, C2D_WithColor | C2D_AlignCenter, x, y, 0.5f, temp, size, color);
}
void DrawStrBoxCC(float x, float y, float size, u32 color, const char *text, float width, float height) {
	C2D_Text c2d_text;
	C2D_TextFontParse(&c2d_text, systemFont, sizeBuf, text);
	C2D_TextOptimize(&c2d_text);
	float tempx=C2D_Clamp(width / Draw_GetTextWidth(size, text), 0.0001f, 1.f) * size;
	float tempy=C2D_Clamp(height / Draw_GetTextHeight(size, text), 0.0001f, 1.f) * size;
	C2D_DrawText(&c2d_text, C2D_WithColor | C2D_AlignCenter, x, y - Draw_GetTextHeight(tempy, text) / 2, 0.5f, tempx, tempy, color);
}
void Draw_Text_Right(float x, float y, float size, u32 color, const char *text) {
	C2D_Text c2d_text;
	C2D_TextFontParse(&c2d_text, systemFont, sizeBuf, text);
	C2D_TextOptimize(&c2d_text);
	C2D_DrawText(&c2d_text, C2D_WithColor | C2D_AlignRight, x, y, 0.5f, size, size, color);
}

void Draw_GetTextSize(float size, float *width, float *height, const char *text) {
	C2D_Text c2d_text;
	C2D_TextFontParse(&c2d_text, systemFont, sizeBuf, text);
	C2D_TextGetDimensions(&c2d_text, size, size, width, height);
}
float Draw_GetTextWidth(float size, const char *text) {
	float width = 0;
	Draw_GetTextSize(size, &width, NULL, text);
	return width;
}
float Draw_GetTextHeight(float size, const char *text) {
	float height = 0;
	Draw_GetTextSize(size, NULL, &height, text);
	return height;
}
bool Draw_Rect(float x, float y, float w, float h, u32 color) {
	return C2D_DrawRectSolid(x, y, 0.5f, w, h, color);
}
float math_abs(float val){
	float res = val;
	if (res<0){res -= val*2;}
	return res;
}
float percvalf(float at0, float at100, float perc){
	return at0*(1-perc)+at100*(perc);
}
int percval(int at0, int at100, float perc){
	return at0*(1-perc)+at100*(perc);
}
float math_pow2(float val){
	return val*val;
}
float _1ddist(float x0,float x1){
	return math_abs(x0-x1);
}
float _2ddist(float x0,float y0,float x1, float y1){
	return std::sqrt(math_pow2(math_abs(x0-x1))+math_pow2(math_abs(y0-y1)));
}
bool touchedArea(int tx, int ty, u16 x, u16 y, u16 w, u16 h){
	return tx>=x && ty >= y && tx < x + w && ty < y + h;
}

void Gui::ScreenLogic(u32 hDown, u32 hHeld, touchPosition touch){
	maincnt++; millisec += 1/60;
	touchox = touchpx; touchoy = touchpy;
	touchpx = touch.px; touchpy = touch.py;
	touchot = touchpt;

	if (hDown & KEY_ZL){
		errorcode=1011337;
		SFX::Fail();
		sprintf(errorstr,"This is a dummy error. Do you really think that?");
	}
	if ((hDown & KEY_START) && appMode < 4 && fadealpha < 128){
		exiting=true; SFX::Back();
		fadecolor=0; fadeout=true;
	}
	if (touchpx+touchpy!=0){
		touchpt++;
	} else {
		touchpt=0;
	}

	C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
	gcls(top, 0xFF302018);
	gcls(bottom, 0xFF302018);
	C2D_SceneBegin(top);
	Draw_Text_Center(200, 48, 0.6f, 0xFFC08040, "CTGP-7 Save Transfer Tool");
	sprite(gfx_main, gfx_main_mainicon_idx, 128, 80);
	Draw_Rect(0,0,400,240,(fadecolor & 16777215)|(u8)fadealpha<<24);
	C2D_SceneBegin(bottom);
	bool gameCartGood = trnsfGameCartRegion<100;
	switch (appMode) {
		case 0:
			Draw_Text_Center(160, 16, 0.5f, -1, "Which way do you want to transfer?");
			Draw_Rect(40, 85, 240, 40, GuiButtonColor(buttonSel==0, 1));
			Draw_Text_Center(160, 95, 0.75f, COLOR_BT_TX_NORMAL, "CTGP-7 \uE019 MK7");
			Draw_Rect(40, 135, 240, 40, GuiButtonColor(buttonSel==1, 1));
			Draw_Text_Center(160, 145, 0.75f, COLOR_BT_TX_NORMAL, "MK7 \uE019 CTGP-7");
			break;
		case 2:
			Draw_Text_Center(160, 16, 0.5f, -1, "Which version of MK7 do you want to choose?");
			Draw_Rect(25, 48, 270, 32, GuiButtonColor(buttonSel==0, gameCartGood));
			if (trnsfGameCartRegion == 255) {
				Draw_Text(60, 56, 0.5f, COLOR_BT_TX_BAD, "Game Card couldn't be detected.");
			} else if (trnsfGameCartRegion == 254) {
				Draw_Text(60, 56, 0.5f, COLOR_BT_TX_BAD, "Game Card is not inserted.");
			} else {
				Draw_Text(60, 56, 0.5f, COLOR_BT_TX_NORMAL, ("Game Card ("+Transfer::GetRegionString(trnsfGameCartRegion)+")").c_str());
			}
			Draw_ImageAlpha(gfx_main, gfx_main_gamecart_idx, 30, 52, 127*(1+gameCartGood));
			Draw_ImageAlpha(gfx_region, trnsfGameCartRegion, 36, 60, 127*(1+gameCartGood));
			for (u8 i=0; i < TRANSFER_REGION_TOTAL; i++){
				bool c = (trnsfAvailRegion>>i)&1;
				Draw_Rect(25+(i%2)*140, 90+(i/2)*40, 130, 32, GuiButtonColor(buttonSel == 1+i, c));
				Draw_ImageAlpha(gfx_region, i, 30+(i%2)*140, 100+(i/2)*40, 127*(1+c));
				Draw_Text(55+(i%2)*140, 98+(i/2)*40, 0.5f, GuiButtonTextColor(buttonSel == 1+i, c), ("Digital ("+Transfer::GetRegionString(i)+")").c_str());
			}
			break;
		case 3:
			Draw_Text_Center(160, 16, 0.5f, -1, "Do you want to transfer your ghosts too?\n(Ghosts for original courses only.)");
			Draw_Rect(40, 85, 240, 40, GuiButtonColor(buttonSel==0, 1));
			Draw_Text_Center(160, 95, 0.75f, COLOR_BT_TX_NORMAL, "Yes");
			Draw_Rect(40, 135, 240, 40, GuiButtonColor(buttonSel==1, 1));
			Draw_Text_Center(160, 145, 0.75f, COLOR_BT_TX_NORMAL, "No");
			Draw_Text_Center(160, 192, 0.5f, 0xC0FFC080, ((std::string)"MK7 version selected: "+Transfer::GetRegionString(transferRegion)+", "+(transferIsSD?"Digital":"Physical")).c_str());
			break;
		case 9:
			Draw_Text_Center(160, 36, 0.5f, -1, "Transfer completed?\nDo you want to transfer again?");
			Draw_Rect(40, 85, 240, 40, GuiButtonColor(buttonSel==0, 1));
			Draw_Text_Center(160, 95, 0.7f, COLOR_BT_TX_NORMAL, "Yes, continue.");
			Draw_Rect(40, 135, 240, 40, GuiButtonColor(buttonSel==1, 1));
			Draw_Text_Center(160, 145, 0.7f, COLOR_BT_TX_NORMAL, "No, exit.");
			break;
	}
	if (appMode < 4)
		Draw_Text_Right(310, 220, 0.5f, 0x80FFFFFF, "\uE073HOME/START ー Exit");
	if (appMode > 0 && appMode < 4)
		Draw_Text(10, 220, 0.5f, 0x80FFFFFF, "\uE001 Back");
	Draw_Rect(0,0,320,240,(fadecolor & 16777215)|(u8)fadealpha<<24);
	C3D_FrameEnd(0);
	if (!exiting && !fadeout){
		u16 appMode_b = appMode; bool sel=false; Result res;
		u32 dir = !!(hDown & BUTTON_DIR);
		int dirh = !!(hDown & KEY_LEFT)*-1 + !!(hDown & KEY_RIGHT);
		int dirv = !!(hDown & KEY_UP)*-1 + !!(hDown & KEY_DOWN);
		u32 pg2while1=7;
		switch (appMode) {
			case 0:
				if (dirv) buttonSel = !buttonSel;
				if (hDown & BUTTON_OK) sel=true;
				if (touchedArea(touchpx, touchpy, 40, 95, 240, 40) && touchpt && !touchot){buttonSel=0; sel=1;}
				if (touchedArea(touchpx, touchpy, 40, 135, 240, 40) && touchpt && !touchot){buttonSel=1; sel=1;}
				if (sel) {
					transferMode = !!buttonSel; SFX::Accept(); appMode=1;
				}
				break;
			case 1:
				trnsfAvailRegion=0; gameCartGood=0;
				trnsfGameCartRegion=254;
				appMode = 2; break;
			case 2:
				if ((maincnt % 90)==0) Transfer::ProbeVersions();
				while (dir) {
					if (buttonSel == 0) {
						buttonSel += dirv;
						if (hDown & KEY_RIGHT) buttonSel=2;
						if (hDown & KEY_LEFT) buttonSel=1;
						if (buttonSel < 0) buttonSel=TRANSFER_REGION_TOTAL;
					} else {
						buttonSel += dirv * 2 + dirh;
						if (buttonSel < 0) buttonSel=0;
						if (buttonSel > TRANSFER_REGION_TOTAL) buttonSel=0;
					}
					if (buttonSel==0 && trnsfGameCartRegion < 100) break;
					if (buttonSel >0 && ((trnsfAvailRegion>>(buttonSel-1))&1)) break;
					pg2while1--;
					if (!pg2while1) break;
				}
				for (u8 i=0; i < TRANSFER_REGION_TOTAL; i++){
					if (touchedArea(touchpx, touchpy, 25+(i%2)*140, 90+(i/2)*40, 130, 32) && !touchot){
						if ((trnsfAvailRegion>>i)&1) {
							buttonSel=1+i; sel=1; break;
						} else {
							errorcode=-1;
							sprintf(errorstr,"This version of Mario Kart 7 couldn't be detected. Probably because you do not have it.");
						}
						break;
					}
				}
				if (touchedArea(touchpx, touchpy, 25, 48, 270, 32) && !touchot) {buttonSel=0; sel=1;}
				if (hDown & BUTTON_OK) sel=1;
				if (hDown & BUTTON_BACK) {
					appMode = 0; SFX::Back();
				}
				if (sel) {
					SFX::Accept();
					transferIsSD = (buttonSel > 0);
					if (buttonSel>1) transferRegion = buttonSel-1;
					if (!buttonSel) transferRegion = trnsfGameCartRegion;
					appMode=3;
				}
				break;
			case 3:
				if (dirv) buttonSel = !buttonSel;
				if (hDown & BUTTON_OK) sel=true;
				if (touchedArea(touchpx, touchpy, 40, 95, 240, 40) && touchpt && !touchot){buttonSel=0; sel=1;}
				if (touchedArea(touchpx, touchpy, 40, 135, 240, 40) && touchpt && !touchot){buttonSel=1; sel=1;}
				if (sel) {
					transferIncludeGhosts = !buttonSel; SFX::Accept(); appMode=8;
				}
				if (hDown & BUTTON_BACK) {
					appMode = 2; SFX::Back();
				}
				break;
			case 8:
				SFX::WaitStart(); appMode=0;
				if (R_FAILED(res = Transfer::Init())){
					errorcode=-1;
					sprintf(errorstr,
						"Unable to initialize the transfer.\n"
						"Error code: 0x%08lX\n"
						"Tried with MK7 (%s, %s)\n\n"
						"- Is CTGP-7 installed?\n"
						"- Was the medium removed?\n"
						"- Was Mario Kart 7 and CTGP-7 launched at least once?\n\n"
						"Should issues persist, try contacting CyberYoshi64.\n\n"
						,res,Transfer::GetRegionString(transferRegion).c_str(),transferIsSD?"Digital":"Physical");
				} else {
					if (R_FAILED(res=Transfer::Perform())){
						errorcode=-1;
						sprintf(errorstr,
						"Failed completing the transfer.\n\n"
						"Error code: 0x%08lX\n"
						"Type: %s, %s\n"
						"File: %s\n"
						"If this issue persists, look up the below error code in the Nintendo Homebrew Discord server."
						,res
						,Transfer::GetRegionString(transferRegion).c_str()
						,transferIsSD?"Digital":"Physical",transferPath);
					} else {
						SFX::Success(); appMode=9;
					}
				}
				Transfer::Exit();
				SFX::WaitEnd();
				break;
			case 9:
				if (dirv) buttonSel = !buttonSel;
				if (hDown & BUTTON_OK) sel=true;
				if (touchedArea(touchpx, touchpy, 40, 95, 240, 40) && touchpt && !touchot){buttonSel=0; sel=1;}
				if (touchedArea(touchpx, touchpy, 40, 135, 240, 40) && touchpt && !touchot){buttonSel=1; sel=1;}
				if (sel) {
					if (buttonSel) {
						SFX::Back();
						exiting=true; fadeout=true;
					} else {
						SFX::Accept(); appMode=0;
					}
				}
				break;
			default:
				errorcode=-1;
				sprintf(errorstr,"The application has reached an invalid mode and will be closed.\n\nMode: %d",appMode);
				exiting = true; fadeout=true;
		}
		if (appMode_b != appMode) buttonSel=0;
	}
	if (errorcode){
		SFX::Fail();
		showError(errorstr,errorcode);
		errorcode=0;
	}
}
