#include <stdio.h>
#include <string>
#include <3ds.h>
#include <citro2d.h>
#include <citro3d.h>

errorConf kerr;
extern C3D_RenderTarget *top;
extern C3D_RenderTarget *bottom;

std::string keyboardInput(const char* hint, const char* content, u16 numChars=4096, u8 pswdmode=0, u32 feat=678, u32 filter=0, u8 numDigits=5, SwkbdValidInput IsValidInput=SWKBD_NOTEMPTY_NOTBLANK) {
	C3D_FrameEnd(0);
	SwkbdState keyboardState;
	numChars=numChars-1 % 4096;
	char input[numChars];

	swkbdInit(&keyboardState, SWKBD_TYPE_NORMAL, 2, sizeof(input));
	swkbdSetHintText(&keyboardState, hint);
	swkbdSetInitialText(&keyboardState, content);
	swkbdSetPasswordMode(&keyboardState, (SwkbdPasswordMode)pswdmode);
	swkbdSetFeatures(&keyboardState, feat);
	swkbdSetValidation(&keyboardState, IsValidInput, filter, numDigits);
	swkbdInputText(&keyboardState, input, sizeof(input));

	return std::string(input);
}

int keyboardInputInt(const char* hint) {
	C3D_FrameEnd(0);
	SwkbdState keyState;
	char input[4];

	swkbdInit(&keyState, SWKBD_TYPE_NUMPAD, 2, 4);
	swkbdSetHintText(&keyState, hint);

	SwkbdButton pressed = swkbdInputText(&keyState, input, 4);
	int res;
	if(pressed == SWKBD_BUTTON_LEFT) {
		res = 0;
	} else {
		res = strtol(input, NULL, 10);
		if(res > 255)
			res = 255;
	}

	return res;
}

void showError(const char* errstr, int errcode=-1){
	errorType typ=ERROR_TEXT_WORD_WRAP;
	CFG_Language lang=CFG_LANGUAGE_EN;
	errorInit(&kerr, typ, lang);
	if (errstr[0]) errorText(&kerr, errstr);
	if (errcode != -1) errorCode(&kerr,errcode);
	C3D_FrameEnd(C3D_FRAME_SYNCDRAW); // If frame not ended, your 3DS dies XD
	errorDisp(&kerr);
}
