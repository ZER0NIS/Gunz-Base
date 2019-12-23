#pragma once

class MBitmap;
struct MMatchItemDesc;

MBitmap* GetItemIconBitmap(MMatchItemDesc* pItemDesc);
const char* GetItemIconBitmap_Potion(MMatchItemDesc* pDesc);
const char* GetItemIconBitmap_Trap(MMatchItemDesc* pDesc);
MBitmap* GetItemThumbnailBitmap(MMatchItemDesc* pDesc);
