#include "library.h"
#include "shared/AML/amlmod.h"
#include "shared/ini/inireader.h"
#include "GTASA/common.h"

#include "shared/AML/isautils.h"
ISAUtils* sautils = nullptr;

#define sizeofA(__aVar)  ((int)(sizeof(__aVar)/sizeof(__aVar[0])))

MYMOD(net.peepo.BobbleHeads, BobbleHeads, 1.0, GTAPLUGIN)

bool bobbleHeads;

const char* pYesNo[] = 
{
    "FEM_OFF",
    "FEM_ON",
};

void DebugBobbleHead(int oldVal, int newVal)
{
    if (newVal) 
        bobbleHeads = true;
   else 
        bobbleHeads = false;
}

void ProcessBobbleHead(CPed* ped) {
    if (!bobbleHeads) return;
	
    const float scale = 3.0f;
	
    auto hier = GetAnimHierarchyFromSkinClump(ped->m_pRwClump);
    auto* matrices = RpHAnimHierarchyGetMatrixArray(hier);
	
    const int boneTag = 5;
    RwV3d s = { scale, scale, scale };
    RwV3d t = { 0.0f, -(scale / 6.0f) / 10.0f, 0.0f };
    
    for (int i = boneTag + 1; i < 9; i++) {
    	auto index = RpHAnimIDGetIndex(hier, i);
    	if (RwMatrix* mat = &matrices[index]) {
            RwMatrixScale(mat, &s, rwCOMBINEPRECONCAT);
            // Fix forehead and jaw
            if (i == 8) { //if (i == 7 || i == 6 || i == 8) {
                t.x = ((scale / 8.0f) / 10.0f) / 8.0f;
                t.y /= 8.0f;
            }
            RwMatrixTranslate(mat, &t, rwCOMBINEPRECONCAT);
        }
    }
    
    auto index = RpHAnimIDGetIndex(hier, boneTag);
    if (RwMatrix* mat = &matrices[index]) {
        RwMatrixScale(mat, &s, rwCOMBINEPRECONCAT);
    }
}

DECL_HOOKv(CPed_Render, CPed* ped)
{
    ProcessBobbleHead(ped);
	
    CPed_Render(ped);
}

extern "C" void OnModLoad()
{
    ALOGI(GTAPLUGIN, "libBobbleHeadsSA.so init...");
	
    char path[0xFF];
    sprintf(path, "%sBobbleHeadsSA.ini", aml->GetConfigPath());
    inireader.SetIniPath(path);
	
    bobbleHeads = inireader.ReadBoolean("Main", "BobbleHeads", false);
	
    if (inireader.ReadBoolean("Main", "DisablePedSpec", false)) {
        aml->Write(libs.pGame + 0x1CE2F0, (uintptr_t)"\x40\x46\x00\xBF", 4);
    }
	
    HOOK(CPed_Render, library::GetSym("_ZN4CPed6RenderEv"));
	
    sautils = (ISAUtils*)GetInterface("SAUtils");
    if(sautils != nullptr)
    {
    	sautils->AddClickableItem(SetType_Mods, "BobbleHeads", bobbleHeads, 0, sizeofA(pYesNo)-1, pYesNo, DebugBobbleHead);
    }
}