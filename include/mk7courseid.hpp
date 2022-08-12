#pragma once
#include <3ds.h>

// Taken from:
// https://github.com/PabloMK7/CTGP7EFEPlugin/blob/master/CTGP-7CourseNames.xml

namespace CTRDash {
    namespace Course {
        inline const char* GetName(u8 id){
            switch (id) {
            case  0: return "Gctr_MarioCircuit";
            case  1: return "Gctr_RallyCourse";
            case  2: return "Gctr_MarineRoad";
            case  3: return "Gctr_GlideLake";
            case  4: return "Gctr_ToadCircuit";
            case  5: return "Gctr_SandTown";
            case  6: return "Gctr_AdvancedCircuit";
            case  7: return "Gctr_DKJungle";
            case  8: return "Gctr_WuhuIsland1";
            case  9: return "Gctr_WuhuIsland2";
            case 10: return "Gctr_IceSlider";
            case 11: return "Gctr_BowserCastle";
            case 12: return "Gctr_UnderGround";
            case 13: return "Gctr_RainbowRoad";
            case 14: return "Gctr_WarioShip";
            case 15: return "Gctr_MusicPark";
            case 16: return "Gwii_CoconutMall";
            case 17: return "Gwii_KoopaCape";
            case 18: return "Gwii_MapleTreeway";
            case 19: return "Gwii_MushroomGorge";
            case 20: return "Gds_LuigisMansion";
            case 21: return "Gds_AirshipFortress";
            case 22: return "Gds_DKPass";
            case 23: return "Gds_WaluigiPinball";
            case 24: return "Ggc_DinoDinoJungle";
            case 25: return "Ggc_DaisyCruiser";
            case 26: return "Gn64_LuigiCircuit";
            case 27: return "Gn64_KalimariDesert";
            case 28: return "Gn64_KoopaTroopaBeach";
            case 29: return "Gagb_BowserCastle1";
            case 30: return "Gsfc_MarioCircuit2";
            case 31: return "Gsfc_RainbowRoad";
            default: return "";
            }
        }
        inline const char* GetHumanName(u8 id){
            switch (id) {
            case  0: return "Mario Circuit";
            case  1: return "Rock Rock Mountain";
            case  2: return "Cheep Cheep Lagoon";
            case  3: return "Daisy Hills";
            case  4: return "Toad Circuit";
            case  5: return "Shy Guy Bazaar";
            case  6: return "Neo Bowser City";
            case  7: return "DK Jungle";
            case  8: return "Wuhu Loop";
            case  9: return "Maka Wuhu";
            case 10: return "Rosalina's Ice World";
            case 11: return "Bowser's Castle";
            case 12: return "Piranha Plant Slide";
            case 13: return "Rainbow Road";
            case 14: return "Wario's Shipyard";
            case 15: return "Music Park";
            case 16: return "Wii Coconut Mall";
            case 17: return "Wii Koopa Cape";
            case 18: return "Wii Maple Treeway";
            case 19: return "Wii Mushroom Gorge";
            case 20: return "DS Luigi's Mansion";
            case 21: return "DS Airship Fortress";
            case 22: return "DS DK Pass";
            case 23: return "DS Waluigi Pinball";
            case 24: return "GCN Dino Dino Jungle";
            case 25: return "GCN Daisy Cruiser";
            case 26: return "GCN Luigi Circuit";
            case 27: return "N64 Kalimari Desert";
            case 28: return "N64 Koopa Troopa Beach";
            case 29: return "GBA Bowser Castle 1";
            case 30: return "SNES Mario Circuit 2";
            case 31: return "SNES Rainbow Road";
            default: return "<invalid>";
            }
        }
    }
}