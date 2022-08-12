#pragma once

#include "common.hpp"

#define FILE_BUF_SIZE   16384

#define TRANSFER_TRACKCOUNT 32

#define TRANSFER_REGION_JPN 0
#define TRANSFER_REGION_EUR 1
#define TRANSFER_REGION_USA 2
#define TRANSFER_REGION_KOR 3
#define TRANSFER_REGION_TWN 4
#define TRANSFER_REGION_CHN 5
#define TRANSFER_REGION_TOTAL   6

#define TRANSFER_TID_JPN 0x00030600
#define TRANSFER_TID_EUR 0x00030700
#define TRANSFER_TID_USA 0x00030800
#define TRANSFER_TID_KOR 0x00030A00
#define TRANSFER_TID_TWN 0x0008B400
#define TRANSFER_TID_CHN 0x0008B500

#define TRANSFER_DEST_MK7   false
#define TRANSFER_DEST_CTGP7 true

#define TRANSFER_CTGP7_PREFIX "/CTGP-7/savefs/game/"
#define TRANSFER_MK7_PREFIX "/"

namespace Transfer {
    u32 GetRegionTID(u8 region);
    std::string GetRegionString(u8 region);
    u8 GetIndex(u32 tidLow);
    Result Init();
    Result Perform();
    void ProbeVersions();
    void GetTrackName(u8 id);
    void Exit();
    void PrePerform();
}
