#include "common.hpp"

#define showErr()	retAppMode=777; appMode=32768;

static touchPosition touch;
extern u16 retAppMode;
extern u16 appMode;
extern int errorcode;
extern char errorstr[];
bool dspfirmfound = false;
bool exiting = false;
extern bool fadein, fadeout;

// Music and sound effects.
Sound *snd_accept = NULL;
Sound *snd_back = NULL;
Sound *snd_wait = NULL;
Sound *snd_succ = NULL;
Sound *snd_fail = NULL;

int main() {
	amInit();
	srvInit();
	fsInit();
	romfsInit();
	gfxInitDefault();
	Gui::init();
	osSetSpeedupEnable(true);

 	if (access("sdmc:/3ds/dspfirm.cdc", F_OK ) != -1 ) { // Was DSP firm dumped before?
		ndspInit(); // If so, then initialise the service
		dspfirmfound = true;
	}
	// Load the sound effects if DSP is available.
	if (dspfirmfound){
		snd_accept = new Sound("romfs:/snd/SE_ACCEPT.bcwav", 1);
		snd_back = new Sound("romfs:/snd/SE_RETURN.bcwav", 1);
		snd_wait = new Sound("romfs:/snd/SE_WAIT.bcwav", 1);
		snd_succ = new Sound("romfs:/snd/SE_SUCCESS.bcwav", 1);
		snd_fail = new Sound("romfs:/snd/SE_FAILURE.bcwav", 1);
	}
	if (access("sdmc:/CTGP-7/resources/CTGP-7.3gx", F_OK )==-1 || access("sdmc:/CTGP-7/config/version.bin", F_OK )==-1){
		sprintf(errorstr, "CTGP-7 has not been detected.\nPlease install it first, before you\ncan transfer your save data.\n\nThis app will now close.");
		showErr();
	} else if (access("sdmc:/CTGP-7/savefs/game", F_OK)==-1) {
		sprintf(errorstr, "The save data for CTGP-7 could not be detected.\n\nMake sure that your CTGP-7 installation is\nup-to-date and that you have run the mod\nat least once after installing it.\n\nThis app will now close.");
		showErr();
	}
	while (aptMainLoop()){
		hidScanInput();
		u32 hHeld = hidKeysHeld();
		u32 hDown = hidKeysDown();
		hidTouchRead(&touch);
		Gui::clearTextBufs();
		Gui::ScreenLogic(hDown, hHeld, touch);
		Gui::fadeEffects();
		if (exitCondition()) break;
	}
	delete snd_accept;
	delete snd_back;
	delete snd_wait;
	delete snd_succ;
	delete snd_fail;
	if (dspfirmfound) ndspExit();
	Gui::exit();
	romfsExit();
	srvExit();
	fsExit();
	amExit();
	gfxExit();
	return 0;
}

void SFX::Accept() { snd_accept->Play(); }
void SFX::Back() { snd_back->Play(); }
void SFX::WaitStart() { snd_wait->Play(); }
void SFX::WaitEnd() { snd_wait->Stop(); }
void SFX::Success() { snd_succ->Play(); }
void SFX::Fail() { snd_fail->Play(); }
