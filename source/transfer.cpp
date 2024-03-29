#include "common.hpp"
#include "transfer.hpp"

FS_Archive saveArc;
bool transferMode;
char transferPath[256];
u8 transferRegion;
bool transferIsSD;
u8 trnsfGameCartRegion;
u8 trnsfAvailRegion;
bool mk7hasCTGhosts = false;
bool transferStatistics = true; // user can opt-out
bool transferIncludeGhosts[TRANSFER_TRACKCOUNT];
bool transferHasGhosts[TRANSFER_TRACKCOUNT];
u8 transferIsViable = 0;

u64 transferCurrentSize;
u32 transferFileProgress;

u32 Transfer::GetRegionTID(u8 region){
    switch (region){
        case TRANSFER_REGION_JPN: return TRANSFER_TID_JPN;
        case TRANSFER_REGION_EUR: return TRANSFER_TID_EUR;
        case TRANSFER_REGION_USA: return TRANSFER_TID_USA;
        case TRANSFER_REGION_KOR: return TRANSFER_TID_KOR;
        case TRANSFER_REGION_TWN: return TRANSFER_TID_TWN;
        case TRANSFER_REGION_CHN: return TRANSFER_TID_CHN;
        default: return 0;
    }
}

u8 Transfer::GetIndex(u32 tidLow){
    switch (tidLow){
        case TRANSFER_TID_JPN: return TRANSFER_REGION_JPN;
        case TRANSFER_TID_EUR: return TRANSFER_REGION_EUR;
        case TRANSFER_TID_USA: return TRANSFER_REGION_USA;
        case TRANSFER_TID_KOR: return TRANSFER_REGION_KOR;
        case TRANSFER_TID_TWN: return TRANSFER_REGION_TWN;
        case TRANSFER_TID_CHN: return TRANSFER_REGION_CHN;
        default: return 255;
    }
}

std::string Transfer::GetRegionString(u8 region){
    switch (region){
        case TRANSFER_REGION_JPN: return "Japan";
        case TRANSFER_REGION_EUR: return "Europe";
        case TRANSFER_REGION_USA: return "USA";
        case TRANSFER_REGION_KOR: return "Korea";
        case TRANSFER_REGION_TWN: return "Taiwan";
        case TRANSFER_REGION_CHN: return "China";
        default: return "Unknown region";
    }
}

Result Transfer::Init(){
    Result res;
    u32 tidLow = GetRegionTID(transferRegion);
    u32 path[3] = {transferIsSD?MEDIATYPE_SD:MEDIATYPE_GAME_CARD, tidLow, 0x00040000};
    res = FSUSER_OpenArchive(&saveArc, ARCHIVE_USER_SAVEDATA, (FS_Path){PATH_BINARY, sizeof(path), path});
    if (R_FAILED(res)) return res;
    return 0;
}

void ShowProgress() {
    if (transferCurrentSize < 1) transferCurrentSize = 1;
    extern C3D_RenderTarget* bottom;
	Gui::clearTextBufs();
    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    C2D_SceneBegin(bottom);
    Draw_Rect(0,0,320,240,COLOR_APPBG_BOT);
	DrawStrBoxCC(160, 100, 0.5625f, COLOR_APPTX, "The transfer is currently in progress.\n\nPlease do not remove the SD and/or game card\nand don't press the POWER button.", 272, 200);
    Draw_Rect(0,200,320,40,0xA0000000);
    char str[512];
    sprintf(str,"Copying %s ... (%.1f KiB / %.1f KiB)", transferPath, (u32)transferFileProgress/1024.f, (u32)transferCurrentSize/1024.f);
    DrawStrBoxC(160, 204, 0.45f, 0x7FFFFFFF, str, 280);
    Draw_Rect(40,224,240,12,0xFF382008);
    Draw_Rect(40,224,240 * C2D_Clamp((float)transferFileProgress / transferCurrentSize, 0, 1),12,0xFF408000);
    C3D_FrameEnd(0);
}

Result writeToCTGP(const char* format, int idxstart = 0, int idxend = 0) {
    char buf[FILE_BUF_SIZE];
    std::string path1, path2;
    Result res = 0;
    Handle handle; Handle sdHdl;
    if (strchr(format, '%') == NULL){
        sprintf(transferPath, format);
        idxend = idxstart+1;
    }
    for (int i=idxstart; i<idxend; i++) {
        if (strchr(format, '%') != NULL) {
            sprintf(transferPath, format, i);
        }
        path1 = (std::string)TRANSFER_MK7_PREFIX+transferPath;
        path2 = (std::string)TRANSFER_CTGP7_PREFIX+transferPath;
        remove(("sdmc:"+path2).c_str());
        res = FSUSER_OpenFile(&handle, saveArc, fsMakePath(PATH_ASCII, path1.c_str()), FS_OPEN_READ, 0);
        if (R_SUCCEEDED(res)) {
            FSFILE_GetSize(handle, &transferCurrentSize);
            res = FSUSER_OpenFileDirectly(&sdHdl, ARCHIVE_SDMC, fsMakePath(PATH_EMPTY,""), fsMakePath(PATH_ASCII, path2.c_str()), FS_OPEN_CREATE|FS_OPEN_WRITE, FS_ATTRIBUTE_ARCHIVE);
            u64 total = 0; u32 byteread = 0, bytewritten = 0;
            if (R_SUCCEEDED(res)){
                while (total < transferCurrentSize) {
                    res = FSFILE_Read(handle, &byteread, total, buf, FILE_BUF_SIZE);
                    do {
                        res = FSFILE_Write(sdHdl, &bytewritten, total, buf, byteread, 0);
                        if (R_FAILED(res)) break;
                    } while (bytewritten < byteread);
                    if (R_FAILED(res)) break;
                    total += byteread; transferFileProgress = total;
                    ShowProgress();
                }
                FSFILE_Flush(sdHdl);
                FSFILE_Close(sdHdl);
            }
            FSFILE_Close(handle);
            if (R_FAILED(res)) break;
        } else {
            res = 0;
        }
        if (R_FAILED(res)) break;
    }
    return res;
}

Result writeToMK7(const char* format, int idxstart=0, int idxend=0) {
    char buf[FILE_BUF_SIZE];
    Result res = 0;
    Handle handle; Handle sdHdl;
    std::string path1, path2;
    if (strchr(format, '%') == NULL){
        sprintf(transferPath, format);
        idxend = idxstart+1;
    }
    for (int i=idxstart; i<idxend; i++) {
        if (strchr(format, '%') != NULL) {
            sprintf(transferPath, format, i);
        }
        path1 = (std::string)TRANSFER_CTGP7_PREFIX+transferPath;
        path2 = (std::string)TRANSFER_MK7_PREFIX+transferPath;
        FSUSER_DeleteFile(saveArc, fsMakePath(PATH_ASCII, path2.c_str()));
        res = FSUSER_OpenFileDirectly(&sdHdl, ARCHIVE_SDMC, fsMakePath(PATH_EMPTY,""), fsMakePath(PATH_ASCII, path1.c_str()), FS_OPEN_READ, 0);
        if (R_SUCCEEDED(res)) {
            FSFILE_GetSize(sdHdl, &transferCurrentSize);
            res = FSUSER_OpenFile(&handle, saveArc, fsMakePath(PATH_ASCII, path2.c_str()), FS_OPEN_CREATE|FS_OPEN_WRITE, FS_ATTRIBUTE_ARCHIVE);
            u64 total = 0; u32 byteread = 0, bytewritten = 0;
            if (R_SUCCEEDED(res)){
                while (total < transferCurrentSize) {
                    res = FSFILE_Read(sdHdl, &byteread, total, buf, FILE_BUF_SIZE);
                    do {
                        res = FSFILE_Write(handle, &bytewritten, total, buf, byteread, 0);
                        if (R_FAILED(res)) break;
                    } while (bytewritten < byteread);
                    if (R_FAILED(res)) break;
                    total += byteread; transferFileProgress = total;
                    ShowProgress();
                }
                FSFILE_Flush(handle);
                FSFILE_Close(handle);
                FSUSER_ControlArchive(saveArc, ARCHIVE_ACTION_COMMIT_SAVE_DATA, nullptr, 0, nullptr, 0);
            }
            FSFILE_Close(sdHdl);
            if (R_FAILED(res)) break;
        } else {
            res = 0;
        }
        if (R_FAILED(res)) break;
    }
    return res;
}

Result toSD(std::string fname, const char* dirname){
    std::string path1, path2; char buf[FILE_BUF_SIZE];
    Result res; Handle handle; Handle sdHdl;
    path1 = fname;
    path2 = (std::string)dirname+fname;
    remove(("sdmc:"+path2).c_str());
    res = FSUSER_OpenFile(&handle, saveArc, fsMakePath(PATH_ASCII, path1.c_str()), FS_OPEN_READ, 0);
    if (R_SUCCEEDED(res)) {
        FSFILE_GetSize(handle, &transferCurrentSize);
        res = FSUSER_OpenFileDirectly(&sdHdl, ARCHIVE_SDMC, fsMakePath(PATH_EMPTY,""), fsMakePath(PATH_ASCII, path2.c_str()), FS_OPEN_CREATE|FS_OPEN_WRITE, FS_ATTRIBUTE_ARCHIVE);
        u64 total = 0; u32 byteread = 0, bytewritten = 0;
        if (R_SUCCEEDED(res)){
            while (total < transferCurrentSize) {
                res = FSFILE_Read(handle, &byteread, total, buf, FILE_BUF_SIZE);
                do {
                    res = FSFILE_Write(sdHdl, &bytewritten, total, buf, byteread, 0);
                    if (R_FAILED(res)) break;
                } while (bytewritten < byteread);
                if (R_FAILED(res)) break;
                total += byteread; transferFileProgress = total;
                ShowProgress();
            }
            FSFILE_Flush(sdHdl);
            FSFILE_Close(sdHdl);
        }
        FSFILE_Close(handle);
        if (R_FAILED(res)) return res;
    }
    return res;
}

void Transfer::PrePerform(){
    Handle filehdl; transferIsViable = 0;
    bool mk7ok = false, ctgp7ok = false;
    FS_Archive archive; mk7hasCTGhosts = false;
    FSUSER_OpenArchive(&archive, ARCHIVE_SDMC, fsMakePath(PATH_EMPTY,""));
    for (int i=0; i<10; i++){
        sprintf(transferPath,TRANSFER_MK7_PREFIX"system%d.dat", i);
        if (R_SUCCEEDED(FSUSER_OpenFile(&filehdl, saveArc, fsMakePath(PATH_ASCII,transferPath), FS_OPEN_READ, 0)))
            mk7ok = true;
        FSFILE_Close(filehdl);
    }
    for (int i=0; i<10; i++){
        sprintf(transferPath,TRANSFER_CTGP7_PREFIX"system%d.dat", i);
        if (R_SUCCEEDED(FSUSER_OpenFile(&filehdl, archive, fsMakePath(PATH_ASCII,transferPath), FS_OPEN_READ, 0)))
            ctgp7ok = true;
        FSFILE_Close(filehdl);
    }
    transferIsViable = ((ctgp7ok) | (mk7ok<<1));
    if(transferIsViable == 0) return;
    if (transferMode){
        for (int i=0; i<TRANSFER_TRACKCOUNT; i++){
            sprintf(transferPath,TRANSFER_MK7_PREFIX"replay/replay%02d.dat", i);
            transferHasGhosts[i] = R_SUCCEEDED(FSUSER_OpenFile(&filehdl, saveArc, fsMakePath(PATH_ASCII,transferPath), FS_OPEN_READ, 0));
            FSFILE_Close(filehdl);
        }
    } else {
        for (int i=0; i<TRANSFER_TRACKCOUNT; i++){
            sprintf(transferPath,TRANSFER_CTGP7_PREFIX"replay/replay%02d.dat", i);
            transferHasGhosts[i] = R_SUCCEEDED(FSUSER_OpenFile(&filehdl, archive, fsMakePath(PATH_ASCII,transferPath), FS_OPEN_READ, 0));
            FSFILE_Close(filehdl);
        }
    }
    for (int i=32; i<256; i++){
        sprintf(transferPath,TRANSFER_MK7_PREFIX"replay/replay%02d.dat", i);
        mk7hasCTGhosts |= R_SUCCEEDED(FSUSER_OpenFile(&filehdl, saveArc, fsMakePath(PATH_ASCII,transferPath), FS_OPEN_READ, 0));
        FSFILE_Close(filehdl);
    }
    FSUSER_CloseArchive(archive);
}

Result Transfer::Perform(){
    Result res = 0;
    transferCurrentSize=1; transferFileProgress=0;
    sprintf(transferPath, "%c", 0); ShowProgress();
    if (transferMode){
        if (transferStatistics){
            for (int i=0; i<10; i++) {
                sprintf(transferPath, TRANSFER_CTGP7_PREFIX"system%d.dat", i);
                remove(transferPath);
            }
            if(R_FAILED(res = writeToCTGP("system%d.dat", 0, 10))) return res;
        }
        mkdir("sdmc:" TRANSFER_CTGP7_PREFIX "replay",777);
        for (int i=0; i<TRANSFER_TRACKCOUNT; i++) {
            if (transferIncludeGhosts[i]){
                sprintf(transferPath, "replay/replay%02d.dat", i);
                if(R_FAILED(res = writeToCTGP(transferPath))) return res;
            }
        }
    } else {
        if (transferStatistics) {
            for (int i=0; i<10; i++) {
                sprintf(transferPath, TRANSFER_MK7_PREFIX"system%d.dat", i);
                FSUSER_DeleteFile(saveArc, fsMakePath(PATH_ASCII,transferPath));
            }
            if(R_FAILED(res = writeToMK7("system%d.dat", 0, 10))) return res;
        }
        FSUSER_CreateDirectory(saveArc, fsMakePath(PATH_ASCII,TRANSFER_MK7_PREFIX"replay"), FS_ATTRIBUTE_DIRECTORY);
        for (int i=0; i<TRANSFER_TRACKCOUNT; i++) {
            if (transferIncludeGhosts[i]){
                sprintf(transferPath, "replay/replay%02d.dat", i);
                if(R_FAILED(res = writeToMK7(transferPath))) return res;
            }
        }
    }
    return res;
}

void Transfer::ProbeVersions(){
    bool inserted; FS_CardType type;
    bool checkGameCard = (R_SUCCEEDED(FSUSER_CardSlotIsInserted(&inserted)) && inserted && R_SUCCEEDED(FSUSER_GetCardType(&type)) && (type == CARD_CTR));
    u32 titleCount, titlesRead = 0; u64* titleBuf;
    
    AM_GetTitleCount(MEDIATYPE_SD, &titleCount);
    titleBuf = new u64[titleCount];
    while (titlesRead < titleCount)
        AM_GetTitleList(&titlesRead, MEDIATYPE_SD, titleCount, titleBuf);
    trnsfAvailRegion &= 0; trnsfGameCartRegion = 254 + inserted;
    for (u32 i=0; i < titleCount; i++){
        switch ((u32)(titleBuf[i] & 0xFFFFFFFF)){
        case TRANSFER_TID_JPN:
            trnsfAvailRegion |= BIT(TRANSFER_REGION_JPN); break;
        case TRANSFER_TID_EUR:
            trnsfAvailRegion |= BIT(TRANSFER_REGION_EUR); break;
        case TRANSFER_TID_USA:
            trnsfAvailRegion |= BIT(TRANSFER_REGION_USA); break;
        case TRANSFER_TID_KOR:
            trnsfAvailRegion |= BIT(TRANSFER_REGION_KOR); break;
        case TRANSFER_TID_TWN:
            trnsfAvailRegion |= BIT(TRANSFER_REGION_TWN); break;
        case TRANSFER_TID_CHN:
            trnsfAvailRegion |= BIT(TRANSFER_REGION_CHN); break;
        }
    }
    if (checkGameCard){
        titleBuf = new u64[1];
        titlesRead=0;
        while (titlesRead < 1)
            AM_GetTitleList(&titlesRead, MEDIATYPE_GAME_CARD, 1, titleBuf);
        trnsfGameCartRegion = Transfer::GetIndex(titleBuf[0]);
    }
    sprintf(transferPath, "0x%02X | %d,%d,%ld", trnsfAvailRegion, trnsfGameCartRegion, checkGameCard, titleCount);
}

void Transfer::Exit(){
    FSUSER_ControlArchive(saveArc, ARCHIVE_ACTION_COMMIT_SAVE_DATA, nullptr, 0, nullptr, 0);
	FSUSER_CloseArchive(saveArc);
}

bool Transfer::isConfigVerOK(char* fvc){
    return true;
}

Result Transfer::BackupGhosts(int start, int end) {
    Result res; sprintf(transferPath, "%c", 0); ShowProgress();
    mkdir("sdmc:" TRANSFER_CTGHOSTBAK_PATH, 777);
    mkdir("sdmc:" TRANSFER_CTGHOSTBAK_PATH "/replay", 777);
    for (int i=start; i<=end; i++){
        sprintf(transferPath, "/replay/replay%02d.dat", i);
        toSD(transferPath, TRANSFER_CTGHOSTBAK_PATH);
        FSUSER_DeleteFile(saveArc, fsMakePath(PATH_ASCII,transferPath));
    }
    return res;
}