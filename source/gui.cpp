#include "gui.hpp"
#include "main.hpp"
#include "transfer.hpp"
#include "mk7courseid.hpp"

#define showErr(a)	retAppMode=a; appMode=32768; sel=false; SFX::Fail();
#define min(a,b)	(a<=b?a:b)
#define max(a,b)	(a>b?a:b)

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
extern bool mk7hasCTGhosts;
extern bool transferStatistics;
extern bool transferIncludeGhosts[];
extern bool transferHasGhosts[];
extern u8 transferIsViable;
s8 availGhostList[TRANSFER_TRACKCOUNT + 1];
size_t availGhostCount;
s8 availGhostTableOff;

bool fadein = true, fadeout = false;
s16 fadealpha = 250;
u32 fadecolor;
C3D_RenderTarget *top;
C3D_RenderTarget *bottom;

u16 appMode, retAppMode;
int buttonSel;

void Gui::clearTextBufs(void) {
	C2D_TextBufClear(sizeBuf);
}
bool exitCondition(void){return(exiting&&!fadeout&&(appMode<9999));}
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
	systemFont	= NULL;
	return 0;
}
void Gui::exit(void) {
	if (gfx_main) C2D_SpriteSheetFree(gfx_main);
	if (gfx_region) C2D_SpriteSheetFree(gfx_region);
	C2D_TextBufDelete(sizeBuf);
	//C2D_FontFree(systemFont);
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

void Gui::Checkmark(float x, float y, u16 align, bool flag, bool selected = 0){
	float xoff = ((align>>0) & 3)*12;
	float yoff = ((align>>2) & 3)*12;
	x -= xoff; y -= yoff;
	if (selected){
		Draw_Rect(x, y, 24, 24, flag?COLOR_CK_BG_HOVER:COLOR_BT_BG_HOVER);
	} else {
		Draw_Rect(x, y, 24, 24, flag?COLOR_CK_BG_NORMAL:COLOR_BT_BG_NORMAL);
	}
	if (flag) {
		C2D_DrawLine(x+4, y+11, COLOR_CK_TICK_COL, x+10, y+17, COLOR_CK_TICK_COL2, 2, 0.5f);
		C2D_DrawLine(x+20, y+7, COLOR_CK_TICK_COL, x+10, y+17, COLOR_CK_TICK_COL2, 2, 0.5f);
	}
}

void Draw_Text(float x, float y, float size, u32 color, const char *text) {
	C2D_Text c2d_text;
	C2D_TextFontParse(&c2d_text, systemFont, sizeBuf, text);
	C2D_TextOptimize(&c2d_text);
	C2D_DrawText(&c2d_text, C2D_WithColor | C2D_AlignLeft, x, y, 0.5f, size, size, color);
}

void DrawStrBox(float x, float y, float size, u32 color, const char *text, float width, float maxwidth) {
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

void DrawStrBoxC(float x, float y, float size, u32 color, const char *text, float width, float maxwidth) {
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
void DrawStrBoxCCW(float x, float y, float size, u32 color, const char *text, float width, float height, float ww2) {
	C2D_Text c2d_text;
	C2D_TextFontParse(&c2d_text, systemFont, sizeBuf, text);
	C2D_TextOptimize(&c2d_text);
	float w = Draw_GetTextWidth(size, text);
	float tempx=C2D_Clamp(width / min(ww2, w), 0.0001f, 1.f) * size;
	float tempy=C2D_Clamp(height / Draw_GetTextHeight(size, text), 0.0001f, 1.f) * size;
	C2D_DrawText(&c2d_text, C2D_WithColor | C2D_WordWrap | C2D_AlignCenter, x, y - Draw_GetTextHeight(tempy, text) / 2, 0.5f, tempx, tempy, color, width);
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

	if ((hDown & KEY_START) && appMode < 8 && fadealpha < 128){
		exiting=true; SFX::Back();
		fadecolor=0; fadeout=true;
	}
	if (touchpx+touchpy!=0){
		touchpt++;
	} else {
		touchpt=0;
	}

	C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
	gcls(top, COLOR_APPBG_TOP);
	gcls(bottom, COLOR_APPBG_BOT);
	C2D_SceneBegin(top);
	Draw_Text_Center(200, 48, 0.6f, COLOR_TTLBNR, "CTGP-7 Save Transfer Tool");
	Draw_Text_Right(380,216,.5f, COLOR_VERBNR, VERSION_STRING);
	sprite(gfx_main, gfx_main_mainicon_idx, 128, 80);
	Draw_Rect(0,0,400,240,(fadecolor & 16777215)|(u8)fadealpha<<24);
	C2D_SceneBegin(bottom);
	bool gameCartGood = trnsfGameCartRegion<100;
	bool anyGhostsAvailable = availGhostCount > 0;
	bool anyGhostPicked=false; u8 ghostsPicked = 0;
	if (anyGhostsAvailable) {
		for (int i=0; i<TRANSFER_TRACKCOUNT; i++) {
			anyGhostPicked |= transferIncludeGhosts[i];
			if (transferIncludeGhosts[i]) ghostsPicked++;
		}
	}
	bool pg2cont = anyGhostPicked||transferStatistics;
	switch (appMode) {
		case 32768:
			DrawStrBoxCCW(160, 104, 0.5f, COLOR_BT_TX_NORMAL, errorstr, 304, 192, 336);
			Draw_Rect(40, 200, 240, 32, GuiButtonColor(buttonSel==0, 1));
			Draw_Text_Center(160, 205, 0.625f, COLOR_BT_TX_NORMAL, "OK");
			break;
		case 0:
			DrawStrBoxC(160, 16, 0.5f, -1, "Which way do you want to transfer?",300);
			Draw_Rect(40, 85, 240, 40, GuiButtonColor(buttonSel==0, 1));
			Draw_Text_Center(160, 95, 0.75f, COLOR_BT_TX_NORMAL, "CTGP-7 \uE019 MK7");
			Draw_Rect(40, 135, 240, 40, GuiButtonColor(buttonSel==1, 1));
			Draw_Text_Center(160, 145, 0.75f, COLOR_BT_TX_NORMAL, "MK7 \uE019 CTGP-7");
			break;
		case 2:
			DrawStrBoxC(160, 16, 0.5f, COLOR_APPTX, "Which version of MK7 do you want to choose?",300);
			Draw_Rect(25, 48, 270, 32, GuiButtonColor(buttonSel==0, gameCartGood));
			if (trnsfGameCartRegion == 255) {
				DrawStrBox(60, 56, 0.5f, GuiButtonTextColor(buttonSel==0, gameCartGood), "Game Card couldn't be detected.", 224);
			} else if (trnsfGameCartRegion == 254) {
				DrawStrBox(60, 56, 0.5f, GuiButtonTextColor(buttonSel==0, gameCartGood), "Game Card is not inserted.", 224);
			} else {
				DrawStrBox(60, 56, 0.5f, COLOR_BT_TX_NORMAL, ("Game Card ("+Transfer::GetRegionString(trnsfGameCartRegion)+")").c_str(), 224);
			}
			Draw_ImageAlpha(gfx_main, gfx_main_gamecart_idx, 30, 52, 127*(1+gameCartGood));
			Draw_ImageAlpha(gfx_region, trnsfGameCartRegion, 36, 60, 127*(1+gameCartGood));
			for (u8 i=0; i < TRANSFER_REGION_TOTAL; i++){
				bool c = (trnsfAvailRegion>>i)&1;
				Draw_Rect(25+(i%2)*140, 90+(i/2)*40, 130, 32, GuiButtonColor(buttonSel == 1+i, c));
				Draw_ImageAlpha(gfx_region, i, 29+(i%2)*140, 100+(i/2)*40, 127*(1+c));
				DrawStrBox(52+(i%2)*140, 98+(i/2)*40, 0.5f, GuiButtonTextColor(buttonSel == 1+i, c), ("Digital ("+Transfer::GetRegionString(i)+")").c_str(),95);
			}
			break;
		case 3:
			if (!transferMode) {
				DrawStrBoxC(160, 8, 0.5f, COLOR_APPTX, "Transfer options\nCTGP-7 to MK7",300);
			} else {
				DrawStrBoxC(160, 8, 0.5f, COLOR_APPTX, "Transfer options\nMK7 to CTGP-7",300);
			}
			sprintf(errorstr,"MK7 version selected: %s (%s)",Transfer::GetRegionString(transferRegion).c_str(), transferIsSD?"Digital":"Physical");
			DrawStrBoxC(160, 40, 0.5f, COLOR_APPTX, errorstr,300);
			Checkmark(32, 68, 0, transferStatistics, buttonSel==0);
			Draw_Text(72, 72, .5f, COLOR_BT_TX_NORMAL, "Transfer main save data");
			Draw_Rect(32, 100, 256, 36, GuiButtonColor(buttonSel==1,anyGhostsAvailable));
			sprintf(errorstr,"Transfer ghosts (%d selected)",ghostsPicked);
			DrawStrBox(40, 110, .5f, GuiButtonTextColor(buttonSel==1,anyGhostsAvailable), errorstr, 184);
			DrawStrBoxCC(256, 118, 0.625f, COLOR_BT_TX_NORMAL, "\u2192", 32, 24);
			Draw_Rect(32, 160, 256, 48, GuiButtonColor(buttonSel==2,pg2cont));
			DrawStrBoxC(160, 172, 0.625f, GuiButtonTextColor(buttonSel==2,pg2cont), "Let's go!", 232);
			break;
		case 4:
			DrawStrBoxC(160, 8, 0.5f, COLOR_APPTX, "Ghosts to transfer",300);
			Draw_Rect(32, 180, 256, 36, GuiButtonColor(buttonSel==0,1));
			DrawStrBoxC(160, 188, 0.5625f, COLOR_BT_TX_NORMAL, "Continue", 232);
			Draw_Rect( 25,  28, 130, 32, GuiButtonColor(buttonSel==1,1));
			Draw_Rect(165,  28, 130, 32, GuiButtonColor(buttonSel==2,1));
			DrawStrBoxCC(90, 42, .5f, COLOR_BT_TX_NORMAL, "Unselect all", 120, 36);
			DrawStrBoxCC(230, 42, .5f, COLOR_BT_TX_NORMAL, "Select all", 120, 36);
			Draw_Rect(256,  88, 36, 36, GuiButtonColor(buttonSel==3,1));
			DrawStrBoxCC(272, 106, 0.625f, COLOR_BT_TX_NORMAL, "\u2191", 36, 36);
			Draw_Rect(256, 128, 36, 36, GuiButtonColor(buttonSel==4,1));
			DrawStrBoxCC(272, 146, 0.625f, COLOR_BT_TX_NORMAL, "\u2193", 36, 36);
			for (size_t i=0; i<4; i++){
				bool k = (availGhostTableOff+i < availGhostCount) && (availGhostList[availGhostTableOff+i]>=0);
				if (k) {
					Checkmark(24,64+i*28,0,transferIncludeGhosts[availGhostList[availGhostTableOff+i]],buttonSel==5+i);
					DrawStrBox(60, 68+i*28, .5f, COLOR_APPTX, CTRDash::Course::GetHumanName(availGhostList[availGhostTableOff+i]), 172);
				} else {
					Checkmark(24,64+i*28,0,0,buttonSel==5+i);
				}
			}
			break;
		case 9:
			DrawStrBoxC(160, 36, 0.5f, COLOR_APPTX, "Transfer completed?\nDo you want to transfer again?",300);
			Draw_Rect(40, 85, 240, 40, GuiButtonColor(buttonSel==0, 1));
			Draw_Text_Center(160, 95, 0.7f, COLOR_BT_TX_NORMAL, "Yes, continue.");
			Draw_Rect(40, 135, 240, 40, GuiButtonColor(buttonSel==1, 1));
			Draw_Text_Center(160, 145, 0.7f, COLOR_BT_TX_NORMAL, "No, exit.");
			break;
	}
	if (appMode < 8)
		Draw_Text_Right(310, 220, 0.5f, COLOR_HINT_TX, "\uE073HOME/START ー Exit");
	if (appMode > 0 && appMode < 8)
		Draw_Text(10, 220, 0.5f, COLOR_HINT_TX, "\uE001 Back");
	Draw_Rect(0,0,320,240,(fadecolor & 16777215)|(u8)fadealpha<<24);
	C3D_FrameEnd(0);
	if (!exitCondition()){
		u16 appMode_b = appMode; bool sel=false; Result res;
		u32 dir = !!(hDown & BUTTON_DIR);
		int dirh = !!(hDown & KEY_LEFT)*-1 + !!(hDown & KEY_RIGHT);
		int dirv = !!(hDown & KEY_UP)*-1 + !!(hDown & KEY_DOWN);
		switch (appMode) {
			case 777:
				exiting = true;
				break;
			case 32768:
				if (hDown & BUTTON_OK) sel=true;
				sel |= (touchedArea(touchpx, touchpy, 40, 200, 240, 32) && touchpt && !touchot);
				if (sel) {
					SFX::Accept();
					appMode = retAppMode;
				}
				break;
			case 0:
				if (dirv) buttonSel = !buttonSel;
				if (hDown & BUTTON_OK) sel=true;
				if (touchedArea(touchpx, touchpy, 40, 95, 240, 40) && touchpt && !touchot){buttonSel=0; sel=1;}
				if (touchedArea(touchpx, touchpy, 40, 135, 240, 40) && touchpt && !touchot){buttonSel=1; sel=1;}
				if (sel) {
					transferMode = !!buttonSel; SFX::Accept(); appMode=1;
					if (access("sdmc:/CTGP-7/config/version.bin", F_OK) == -1){
						showErr(0);
						exiting=true; fadeout=true;
						sprintf(errorstr, "CTGP-7 is either not installed or its version is not supported.\n\nThis application will now close.");
					}
				}
				break;
			case 1:
				trnsfAvailRegion=0; gameCartGood=0;
				trnsfGameCartRegion=254;
				appMode = 2; break;
			case 2:
				if ((maincnt % 30)==0) Transfer::ProbeVersions();
				if (hDown & BUTTON_OK) sel=1;
				if (hDown & BUTTON_BACK) {
					appMode = 0; SFX::Back();
				}
				if (dir) {
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
				}
				for (u8 i=0; i < TRANSFER_REGION_TOTAL; i++){
					if (touchedArea(touchpx, touchpy, 25+(i%2)*140, 90+(i/2)*40, 130, 32) && !touchot){
						buttonSel=1+i; sel=true; break;
					}
				}
				if (touchedArea(touchpx, touchpy, 25, 48, 270, 32) && !touchot){
					buttonSel=0; sel=true;
				}
				if (sel){
					if (buttonSel==0){
						if (!gameCartGood){
							showErr(2);
							if (trnsfGameCartRegion == 255) {
								sprintf(errorstr,
								"The game cart cannot be used.\n\n"
								"Try taking the game cart out and reinsert it.\n"
								"The game cart might not be detected, it could be dirty or damaged.\n"
								"Make sure, you insert a Mario Kart 7 cart, that is a final release.\n"
								"Other games, as well as kiosk and debug builds are not supported."
								);
							} else {
								sprintf(errorstr,
								"The game cart cannot be detected.\n"
								"Did you insert a game cart in it?\n\n"
								"The game cart might be damaged or is dirty. Try removing and reinserting the game cart and try again."
								);
							}
						}
					} else {
						if(!((trnsfAvailRegion>>(buttonSel-1))&1)){
							showErr(2);
							sprintf(errorstr,"This version of Mario Kart 7 couldn't be detected.\nYou might not own it or it was modified.");
						}
					}
				}
				if (sel) {
					SFX::Accept();
					transferIsSD = (buttonSel > 0);
					if (buttonSel>1) transferRegion = buttonSel-1;
					if (!buttonSel) transferRegion = trnsfGameCartRegion;
					if (R_SUCCEEDED(Transfer::Init())){
						Transfer::PrePerform();
						if ((transferIsViable>>transferMode)&1){
							appMode=3;
							if (mk7hasCTGhosts) appMode = 5;
							memset(&transferIncludeGhosts, 0, TRANSFER_TRACKCOUNT);
							memset(&availGhostList, -1, sizeof(availGhostList));
							size_t i = 0;
							availGhostCount = availGhostTableOff = 0;
							for (i=0; i<TRANSFER_TRACKCOUNT; i++){
								if (transferHasGhosts[i])
									availGhostList[availGhostCount++] = i;
							}
						} else {
							showErr(2);
							sprintf(errorstr,"Cannot start the transfer.\n\nThe following save data is missing:\n%s%s\nMake sure you have run the games at least once to ensure their initial save data is created.",
							(transferIsViable & 1)?"":"- CTGP-7\n", (transferIsViable & 2)?"":"- Mario Kart 7\n");
						}
					} else {
						showErr(2);
						sprintf(errorstr,"The save data does not exist.\n\nPlease ensure to run Mario Kart 7 at least once, so the save data has been created.");
					}
					Transfer::Exit();
				}
				break;
			case 3:
				if (dirv) buttonSel += dirv;
				if (buttonSel<0) buttonSel=2;
				buttonSel %= 3;

				if (hDown & BUTTON_OK) sel=true;
				if (touchedArea(touchpx,touchpy,24,60,256,40) && !touchot){
					sel=true; buttonSel=0;
				}
				if (touchedArea(touchpx,touchpy,24,100,272,48) && !touchot){
					sel=true; buttonSel=1;
				}
				if (touchedArea(touchpx,touchpy,24,156,272,48) && !touchot){
					sel=true; buttonSel=2;
				}
				if (sel){
					switch (buttonSel){
						case 0:
							transferStatistics = !transferStatistics;
							break;
						case 1:
							if (anyGhostsAvailable) {
								appMode = 4; SFX::Accept();
							} else {
								showErr(3);
								sprintf(errorstr,"There isn't any ghost data you can transfer.\n\nMake sure you're going to transfer data\nin the correct direction and to the game of the\ncorrect region.");
							}
							break;
						case 2:
							if (pg2cont){
								appMode = 8; SFX::Accept();
							} else {
								showErr(3);
								sprintf(errorstr,"You are not transferring anything.\n\nTick \"Transfer main save data\"\nor select some course ghost data.");
							}
							break;
					}
				}
				if (hDown & BUTTON_BACK) {
					appMode = 2; SFX::Back();
				}
				break;
			case 4:
				if (hDown & BUTTON_BACK) {
					appMode = 3; SFX::Back();
				}
				if (hDown & BUTTON_OK) sel=true;
				if (dir) {
					if (hDown & KEY_DOWN) {
						buttonSel++;
						if (buttonSel == 2) buttonSel = 5;
					} else if (hDown & KEY_UP) {
						buttonSel--;
						if (buttonSel == 4) buttonSel = 1;
					} else if (hDown & KEY_LEFT) {
						if (buttonSel == 2) buttonSel--;
						if (buttonSel == 3) buttonSel=5;
						if (buttonSel == 4) buttonSel=7;
					} else if (hDown & KEY_RIGHT) {
						if (buttonSel == 1) buttonSel++;
						if (buttonSel == 5) buttonSel=3;
						if (buttonSel == 6) buttonSel=3;
						if (buttonSel == 7) buttonSel=4;
						if (buttonSel == 8) buttonSel=4;
					}
				}
				if (touchpt && !touchot) {
					if (touchedArea(touchpx,touchpy, 32,180,256, 36)){sel=true; buttonSel=0;}
					if (touchedArea(touchpx,touchpy, 25, 28,130, 32)){sel=true; buttonSel=1;}
					if (touchedArea(touchpx,touchpy,165, 28,130, 32)){sel=true; buttonSel=2;}
					if (touchedArea(touchpx,touchpy,256, 88, 36, 36)){sel=true; buttonSel=3;}
					if (touchedArea(touchpx,touchpy,256,128, 36, 36)){sel=true; buttonSel=4;}
					if (touchedArea(touchpx,touchpy, 24, 64,200, 24)){sel=true; buttonSel=5;}
					if (touchedArea(touchpx,touchpy, 24, 92,200, 24)){sel=true; buttonSel=6;}
					if (touchedArea(touchpx,touchpy, 24,120,200, 24)){sel=true; buttonSel=7;}
					if (touchedArea(touchpx,touchpy, 24,148,200, 24)){sel=true; buttonSel=8;}
				}
				if (buttonSel<0) buttonSel=8;
				buttonSel %= 9;
				if (sel){
					if (buttonSel == 0) {
						appMode = 3; SFX::Accept();
					} else if (buttonSel == 1) {
						size_t i=0; while (true) {
							if ((i>=availGhostCount)||(availGhostList[i]<0)) break;
							transferIncludeGhosts[availGhostList[i++]] = false;
						}
					} else if (buttonSel == 2) {
						size_t i=0; while (true) {
							if ((i>=availGhostCount)||(availGhostList[i]<0)) break;
							transferIncludeGhosts[availGhostList[i++]] = true;
						}
					} else if (buttonSel == 3) {
						availGhostTableOff -= 4;
						if (availGhostTableOff < 0)
							availGhostTableOff = (int)((availGhostCount-1)/4)*4;
					} else if (buttonSel == 4) {
						availGhostTableOff += 4;
						if (availGhostTableOff >= availGhostCount)
							availGhostTableOff = 0;
					} else if (buttonSel < 9) {
						size_t i = availGhostTableOff + buttonSel - 5;
						if ((i<availGhostCount)&&(availGhostList[i]>=0))
							transferIncludeGhosts[availGhostList[i]] = !transferIncludeGhosts[availGhostList[i]];
					}
				}
				break;
			case 5:
				sprintf(errorstr, "Warning:\nGhost data for custom tracks was found inside\nvanilla MK7's save data.\n\nThis data will be moved out to\n\"sdmc:" TRANSFER_CTGHOSTBAK_PATH "\"\nso you can manually move them to CTGP-7\nyourself, if you want to do so.");
				showErr(6); break;
			case 6:
				appMode=3;
				if (R_FAILED(res = Transfer::Init())){
					showErr(3);
					sprintf(errorstr,
					"Unable to initialize the transfer.\n"
					"Error code: 0x%08lX\n"
					"MK7 type: %s, %s\n"
					"Transferring %s\n\n"
					"If problems persist, ask for help in the\n"
					"CTGP-7 Discord server: invite.gg/ctgp7"
					,res
					,Transfer::GetRegionString(transferRegion).c_str()
					,transferIsSD?"Digital":"Physical"
					,transferMode?"MK7 \uE019 CTGP-7":"CTGP-7 \uE019 MK7");
				} else if (R_FAILED(res=Transfer::BackupGhosts(32, 255))) {
						showErr(3);
						sprintf(errorstr,
						"The transfer failed.\n\n"
						"Error code: 0x%08lX\n"
						"MK7 type: %s, %s\n"
						"Transferring MK7 \uE019 SD Card\n"
						"File: %s\n\n"
						"If problems persist, ask for help in the\n"
						"CTGP-7 Discord server: invite.gg/ctgp7"
						,res
						,Transfer::GetRegionString(transferRegion).c_str()
						,transferIsSD?"Digital":"Physical"
						,transferPath);
				}
				Transfer::Exit(); break;
			case 8:
				SFX::WaitStart(); appMode=0;
				if (R_FAILED(res = Transfer::Init())){
					showErr(0);
					sprintf(errorstr,
						"Unable to initialize the transfer.\n"
						"Error code: 0x%08lX\n"
						"MK7 type: %s, %s\n"
						"Transferring %s\n\n"
						"If problems persist, ask for help in the\n"
						"CTGP-7 Discord server: invite.gg/ctgp7"
						,res
						,Transfer::GetRegionString(transferRegion).c_str()
						,transferIsSD?"Digital":"Physical"
						,transferMode?"MK7 \uE019 CTGP-7":"CTGP-7 \uE019 MK7");
				} else {
					if (R_FAILED(res=Transfer::Perform())){
						showErr(0);
						sprintf(errorstr,
						"The transfer failed.\n\n"
						"Error code: 0x%08lX\n"
						"MK7 type: %s, %s\n"
						"Transferring %s\n"
						"File: %s\n\n"
						"If problems persist, ask for help in the\n"
						"CTGP-7 Discord server: invite.gg/ctgp7"
						,res
						,Transfer::GetRegionString(transferRegion).c_str()
						,transferIsSD?"Digital":"Physical"
						,transferMode?"MK7 \uE019 CTGP-7":"CTGP-7 \uE019 MK7"
						,transferPath);
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
		}
		if (appMode_b != appMode) buttonSel=0;
	}
}
