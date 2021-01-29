#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#include <windows.h>
#include <TlHelp32.h>
#include <cstdint>
#include <wchar.h>
#include "dinput8/dinputWrapper.h"
#include "MinHook/include/MinHook.h"
#include <iostream>
#include "dllmain.h"
#include <thread>   
#include <string>
#include "PointersDef.h"
#include "stdint.h"
#include <tuple>

void MainLoop();

int StaticAddressPatcher();

int DifficultyModule();

int NGDifficulty(int RowOffset, float NGMultiplier, int DifficultyLevel);

unsigned int* GetPointer(bool PointerValid, int PointerName);

bool isSpeffectActive(int Speffect);

void FPstaminaDrain(int* stamina, unsigned int* maxstamina, unsigned int* fp, unsigned int* basemaxfp);

long long* Parameme(int RowID);

std::tuple<int16_t*, int16_t> EquipParamWeapon(int EquipParamWeaponRowID);

void EquipParamWeaponSwapper(int InfusionRange, int EquipParamWeaponRowID, int TriggerSpeffectID, int TargetspAtkCategory);

bool MovesetSwap1 = false;

tDirectInput8Create oDirectInput8Create;

const LPCWSTR AppWindowTitle = L"DARK SOULS III";
HWND g_hWnd = NULL;

DWORD WINAPI MainThread(HMODULE hModule)
{
	//delay patching process
	while (!g_hWnd)
		g_hWnd = FindWindow(0, AppWindowTitle);

	MH_Initialize();

	if (GetPrivateProfileIntW(L"Debug", L"HoodiePatcherDebugLog", 0, L".\\HoodiePatcher.ini") == 1) {
		FILE* fp;
		AllocConsole();
		SetConsoleTitleA("HoodiePatcher - Debug Log");
		freopen_s(&fp, "CONOUT$", "w", stdout);
		std::cout << "HoodiePatcher - Debug Log == Enabled" << std::endl << std::endl;
	}
	else { std::cout << "HoodiePatcher - Debug Log == Disabled" << std::endl << std::endl; }

	StaticAddressPatcher();

	DifficultyModule();

	std::cout << "HoodiePatcher - Complete" << std::endl;

	MainLoop();

	return S_OK;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	HANDLE thread;
	static HMODULE dinput8dll = nullptr;
	HMODULE chainModule = NULL;
	wchar_t chainPath[MAX_PATH];
	wchar_t dllPath[MAX_PATH];

	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:

		GetPrivateProfileStringW(L"Misc", L"ChainloadDinput8dll", L"", chainPath, MAX_PATH, L".\\HoodiePatcher.ini");

		if (chainPath && wcscmp(chainPath, L""))
		{
			GetCurrentDirectoryW(MAX_PATH, dllPath);
			wcscat_s(dllPath, MAX_PATH, L"\\");
			wcscat_s(dllPath, MAX_PATH, chainPath);
			chainModule = LoadLibraryW(dllPath);

			if (chainModule)
			{
				oDirectInput8Create = (tDirectInput8Create)GetProcAddress(chainModule, "DirectInput8Create");
			}
		}

		if (!chainModule)
		{
			wchar_t path[MAX_PATH];
			GetSystemDirectoryW(path, MAX_PATH);
			wcscat_s(path, MAX_PATH, L"\\dinput8.dll");
			dinput8dll = LoadLibraryW(path);

			if (dinput8dll)
			{
				oDirectInput8Create = (tDirectInput8Create)GetProcAddress(dinput8dll, "DirectInput8Create");
			}
		}

		thread = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)MainThread, hModule, NULL, NULL);

		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		if (chainModule)
		{
			FreeLibrary(chainModule);
		}
		else {
			FreeLibrary(dinput8dll);
		}

		MH_DisableHook(MH_ALL_HOOKS);
		MH_Uninitialize();

		break;
	}
	return TRUE;
}

long long* Parameme(int RowID) {
	int num = 0;
	byte* EquiParamWeapon = (byte*)GetPointer(true, PointerEquipParamWeapon);
	byte* DataOffset = EquiParamWeapon + (byte)0x40;
	if (*(unsigned int*)DataOffset == RowID) {
		return (long long*)(EquiParamWeapon + *(unsigned int*)(DataOffset + (byte)0x8));
	}
	while (true) {
		num += 1;
		DataOffset += (byte)0x18;
		if (*(unsigned int*)DataOffset == 0) {
			std::cout << "Row ID: " << std::dec << RowID << " Was Not Found" << std::endl;
			return nullptr;
		}
		else if (*(unsigned int*)DataOffset == RowID) {
			return (long long*)(EquiParamWeapon + *(unsigned int*)(DataOffset + (byte)0x8));
		}
	}
}

std::tuple<int16_t*, int16_t> EquipParamWeapon(int EquipParamWeaponRowID) {
	byte Bytes1 = (byte)0x01;
	byte Bytes2 = (byte)0x02;
	byte Bytes4 = (byte)0x04;
	byte* Row = (byte*)Parameme(EquipParamWeaponRowID);

	int32_t* behaviorVariationId = (int32_t*)Row;
	Row += Bytes4;
	int32_t* sortId = (int32_t*)Row;
	Row += Bytes4;
	int32_t* wanderingEquipId = (int32_t*)Row;
	Row += Bytes4;
	float* weight = (float*)Row;
	Row += Bytes4;
	float* weaponWeightRate = (float*)Row;
	Row += Bytes4;
	int32_t* fixPrice = (int32_t*)Row;
	Row += Bytes4;
	int32_t* basicPrice = (int32_t*)Row;
	Row += Bytes4;
	int32_t* sellValue = (int32_t*)Row;
	Row += Bytes4;
	float* correctStrength = (float*)Row;
	Row += Bytes4;
	float* correctAgility = (float*)Row;
	Row += Bytes4;
	float* corretMagic = (float*)Row;
	Row += Bytes4;
	float* corretFaith = (float*)Row;
	Row += Bytes4;
	float* physGuardCutRate = (float*)Row;
	Row += Bytes4;
	float* magGuardCutRate = (float*)Row;
	Row += Bytes4;
	float* fireGuardCutRate = (float*)Row;
	Row += Bytes4;
	float* thunGuardCutRate = (float*)Row;
	Row += Bytes4;
	int32_t* spEffectBehaviorId0 = (int32_t*)Row;
	Row += Bytes4;
	int32_t* spEffectBehaviorId1 = (int32_t*)Row;
	Row += Bytes4;
	int32_t* spEffectBehaviorId2 = (int32_t*)Row;
	Row += Bytes4;
	int32_t* residentSpEffectId0 = (int32_t*)Row;
	Row += Bytes4;
	int32_t* residentSpEffectId1 = (int32_t*)Row;
	Row += Bytes4;
	int32_t* residentSpEffectId2 = (int32_t*)Row;
	Row += Bytes4;
	int32_t* materialSetId = (int32_t*)Row;
	Row += Bytes4;
	int32_t* originEquipWep0 = (int32_t*)Row;
	Row += Bytes4;
	int32_t* originEquipWep1 = (int32_t*)Row;
	Row += Bytes4;
	int32_t* originEquipWep2 = (int32_t*)Row;
	Row += Bytes4;
	int32_t* originEquipWep3 = (int32_t*)Row;
	Row += Bytes4;
	int32_t* originEquipWep4 = (int32_t*)Row;
	Row += Bytes4;
	int32_t* originEquipWep5 = (int32_t*)Row;
	Row += Bytes4;
	int32_t* originEquipWep6 = (int32_t*)Row;
	Row += Bytes4;
	int32_t* originEquipWep7 = (int32_t*)Row;
	Row += Bytes4;
	int32_t* originEquipWep8 = (int32_t*)Row;
	Row += Bytes4;
	int32_t* originEquipWep9 = (int32_t*)Row;
	Row += Bytes4;
	int32_t* originEquipWep10 = (int32_t*)Row;
	Row += Bytes4;
	int32_t* originEquipWep11 = (int32_t*)Row;
	Row += Bytes4;
	int32_t* originEquipWep12 = (int32_t*)Row;
	Row += Bytes4;
	int32_t* originEquipWep13 = (int32_t*)Row;
	Row += Bytes4;
	int32_t* originEquipWep14 = (int32_t*)Row;
	Row += Bytes4;
	int32_t* originEquipWep15 = (int32_t*)Row;
	Row += Bytes4;
	float* antiDemonDamageRate = (float*)Row;
	Row += Bytes4;
	float* antUndeadDamageRate = (float*)Row;
	Row += Bytes4;
	float* antHollowDamageRate = (float*)Row;
	Row += Bytes4;
	float* antAbyssalDamageRate = (float*)Row;
	Row += Bytes4;
	int32_t* vagrantItemLotId = (int32_t*)Row;
	Row += Bytes4;
	int32_t* vagrantBonusEneDropItemLotId = (int32_t*)Row;
	Row += Bytes4;
	int32_t* vagrantItemEneDropItemLotId = (int32_t*)Row;
	Row += Bytes4;
	int16_t* equipModelId = (int16_t*)Row;
	Row += Bytes2;
	int16_t* iconId = (int16_t*)Row;
	Row += Bytes2;
	int16_t* durability = (int16_t*)Row;
	Row += Bytes2;
	int16_t* durailityMax = (int16_t*)Row;
	Row += Bytes2;
	int16_t* attackThrowEscape = (int16_t*)Row;
	Row += Bytes2;
	int16_t* parryDamageLife = (int16_t*)Row;
	Row += Bytes2;
	int16_t* atkBasePhysics = (int16_t*)Row;
	Row += Bytes2;
	int16_t* atkBaseMagic = (int16_t*)Row;
	Row += Bytes2;
	int16_t* atkBaseFire = (int16_t*)Row;
	Row += Bytes2;
	int16_t* atkBaseThunder = (int16_t*)Row;
	Row += Bytes2;
	int16_t* atkBaseStamina = (int16_t*)Row;
	Row += Bytes2;
	int16_t* saWeaponDamage = (int16_t*)Row;
	Row += Bytes2;
	int16_t* saDurability = (int16_t*)Row;
	Row += Bytes2;
	int16_t* guardAngle = (int16_t*)Row;
	Row += Bytes2;
	int16_t* staminaGuardDef = (int16_t*)Row;
	Row += Bytes2;
	int16_t* reinforceTypeId = (int16_t*)Row;
	Row += Bytes2;
	int16_t* trophySGradeId = (int16_t*)Row;
	Row += Bytes2;
	int16_t* trophySeqId = (int16_t*)Row;
	Row += Bytes2;
	int16_t* throwAtkRate = (int16_t*)Row;
	Row += Bytes2;
	int16_t* bowDistRate = (int16_t*)Row;
	Row += Bytes2;
	uint8_t* equipModelCategory = (uint8_t*)Row;
	Row += Bytes1;
	uint8_t* equipModelGender = (uint8_t*)Row;
	Row += Bytes1;
	uint8_t* weaponCategory = (uint8_t*)Row;
	Row += Bytes1;
	uint8_t* wepmotionCategory = (uint8_t*)Row;
	Row += Bytes1;
	uint8_t* guardmotionCategory = (uint8_t*)Row;
	Row += Bytes1;
	uint8_t* atkMaterial = (uint8_t*)Row;
	Row += Bytes1;
	uint8_t* defMaterial = (uint8_t*)Row;
	Row += Bytes1;
	uint8_t* defSfxMaterial = (uint8_t*)Row;
	Row += Bytes1;
	uint8_t* correctType = (uint8_t*)Row;
	Row += Bytes1;
	uint8_t* spAttribute = (uint8_t*)Row;
	Row += Bytes1;
	int16_t* spAtkCategory = (int16_t*)Row;
	Row += Bytes2;
	uint8_t* wepmotionOneHandId = (uint8_t*)Row;
	Row += Bytes1;
	uint8_t* wepmotionBothHandId = (uint8_t*)Row;
	Row += Bytes1;
	uint8_t* properStrength = (uint8_t*)Row;
	Row += Bytes1;
	uint8_t* properAgility = (uint8_t*)Row;
	Row += Bytes1;
	uint8_t* properMagic = (uint8_t*)Row;
	Row += Bytes1;
	uint8_t* properFaith = (uint8_t*)Row;
	Row += Bytes1;
	uint8_t* overStrength = (uint8_t*)Row;
	Row += Bytes1;
	uint8_t* attackBaseParry = (uint8_t*)Row;
	Row += Bytes1;
	uint8_t* defenseBaseParry = (uint8_t*)Row;
	Row += Bytes1;
	uint8_t* guardBaseRepel = (uint8_t*)Row;
	Row += Bytes1;
	uint8_t* attackBaseRepel = (uint8_t*)Row;
	Row += Bytes1;
	uint8_t* guardCutCancelRate = (uint8_t*)Row;
	Row += Bytes1;
	uint8_t* guardLevel = (uint8_t*)Row;
	Row += Bytes1;
	uint8_t* slashGuardCutRate = (uint8_t*)Row;
	Row += Bytes1;
	uint8_t* blowGuardCutRate = (uint8_t*)Row;
	Row += Bytes1;
	uint8_t* thrustGuardCutRate = (uint8_t*)Row;
	Row += Bytes1;
	uint8_t* poisonGuardResist = (uint8_t*)Row;
	Row += Bytes1;
	uint8_t* toxicGuardResist = (uint8_t*)Row;
	Row += Bytes1;
	uint8_t* bloodGuardResist = (uint8_t*)Row;
	Row += Bytes1;
	uint8_t* curseGuardResist = (uint8_t*)Row;
	Row += Bytes1;
	uint8_t* isDurabilityDivergence = (uint8_t*)Row;
	Row += Bytes1;

	Row += Bytes4; //Additional Offset to cut some not yet implemented fields
	Row += Bytes1;

	uint8_t* Unk6 = (uint8_t*)Row;
	Row += Bytes1;
	uint8_t* Unk7 = (uint8_t*)Row;
	Row += Bytes1;
	uint8_t* Unk8 = (uint8_t*)Row;
	Row += Bytes1;
	uint8_t* Unk9 = (uint8_t*)Row;
	Row += Bytes1;
	uint8_t* Unk10 = (uint8_t*)Row;
	Row += Bytes1;
	uint8_t* Unk11 = (uint8_t*)Row;
	Row += Bytes1;
	int32_t* Group0_AtkVfxId = (int32_t*)Row;
	Row += Bytes4;
	int32_t* Group0_DummyPolyId0 = (int32_t*)Row;
	Row += Bytes4;
	int32_t* Group0_DummyPolyId1 = (int32_t*)Row;
	Row += Bytes4;
	int32_t* Group1_AtkVfxId = (int32_t*)Row;
	Row += Bytes4;
	int32_t* Group1_DummyPolyId0 = (int32_t*)Row;
	Row += Bytes4;
	int32_t* Group1_DummyPolyId1 = (int32_t*)Row;
	Row += Bytes4;
	int32_t* Group2_AtkVfxId = (int32_t*)Row;
	Row += Bytes4;
	int32_t* Group2_DummyPolyId0 = (int32_t*)Row;
	Row += Bytes4;
	int32_t* Group2_DummyPolyId1 = (int32_t*)Row;
	Row += Bytes4;
	int32_t* Group3_AtkVfxId = (int32_t*)Row;
	Row += Bytes4;
	int32_t* Group3_DummyPolyId0 = (int32_t*)Row;
	Row += Bytes4;
	int32_t* Group3_DummyPolyId1 = (int32_t*)Row;
	Row += Bytes4;
	int32_t* Group4_AtkVfxId = (int32_t*)Row;
	Row += Bytes4;
	int32_t* Group4_DummyPolyId0 = (int32_t*)Row;
	Row += Bytes4;
	int32_t* Group4_DummyPolyId1 = (int32_t*)Row;
	Row += Bytes4;
	int32_t* Group5_AtkVfxId = (int32_t*)Row;
	Row += Bytes4;
	int32_t* Group5_DummyPolyId0 = (int32_t*)Row;
	Row += Bytes4;
	int32_t* Group5_DummyPolyId1 = (int32_t*)Row;
	Row += Bytes4;
	int32_t* Group6_AtkVfxId = (int32_t*)Row;
	Row += Bytes4;
	int32_t* Group6_DummyPolyId0 = (int32_t*)Row;
	Row += Bytes4;
	int32_t* Group6_DummyPolyId1 = (int32_t*)Row;
	Row += Bytes4;
	int32_t* Group7_AtkVfxId = (int32_t*)Row;
	Row += Bytes4;
	int32_t* Group7_DummyPolyId0 = (int32_t*)Row;
	Row += Bytes4;
	int32_t* Group7_DummyPolyId1 = (int32_t*)Row;
	Row += Bytes4;
	int16_t* materialVal0 = (int16_t*)Row;
	Row += Bytes2;
	int16_t* materialVal1 = (int16_t*)Row;
	Row += Bytes2;
	int32_t* wepAbsorpPosId = (int32_t*)Row;
	Row += Bytes4;
	float* Unk12 = (float*)Row;
	Row += Bytes4;

	Row += Bytes1; //Additional Offset to cut some not yet implemented fields

	uint8_t* Unk21 = (uint8_t*)Row;
	Row += Bytes1;
	uint8_t* Unk22 = (uint8_t*)Row;
	Row += Bytes1;
	uint8_t* Unk23 = (uint8_t*)Row;
	Row += Bytes1;
	float* Unk24 = (float*)Row;
	Row += Bytes4;
	float* Unk25 = (float*)Row;
	Row += Bytes4;
	float* darkGuardCutRate = (float*)Row;
	Row += Bytes4;
	int16_t* atkBaseDark = (int16_t*)Row;
	Row += Bytes2;
	uint8_t* Unk26 = (uint8_t*)Row;
	Row += Bytes1;
	uint8_t* AtkThrowEscapeA = (uint8_t*)Row;
	Row += Bytes1;
	int32_t* swordArt_ActId = (int32_t*)Row;
	Row += Bytes4;
	uint8_t* AtkThrowEscapeB = (uint8_t*)Row;
	Row += Bytes1;
	uint8_t* Unk30 = (uint8_t*)Row;
	Row += Bytes1;
	uint8_t* Unk31 = (uint8_t*)Row;
	Row += Bytes1;
	uint8_t* MenuAdhoc = (uint8_t*)Row;
	Row += Bytes1;
	int32_t* swordArtId = (int32_t*)Row;
	Row += Bytes4;
	float* correctLuck = (float*)Row;
	Row += Bytes4;
	int32_t* reinforceWeaponId = (int32_t*)Row;
	Row += Bytes4;
	int16_t* Unk34 = (int16_t*)Row;
	Row += Bytes2;
	int16_t* displayTypeId = (int16_t*)Row;
	Row += Bytes2;
	float* CalcCorrectVal0 = (float*)Row;
	Row += Bytes4;
	float* CalcCorrectVal1 = (float*)Row;
	Row += Bytes4;
	float* CalcCorrectVal2 = (float*)Row;
	Row += Bytes4;
	float* CalcCorrectVal3 = (float*)Row;
	Row += Bytes4;
	float* CalcCorrectVal4 = (float*)Row;
	Row += Bytes4;
	float* CalcCorrectVal5 = (float*)Row;
	Row += Bytes4;
	float* CalcCorrectVal6 = (float*)Row;
	Row += Bytes4;
	float* CalcCorrectVal7 = (float*)Row;
	Row += Bytes4;
	float* CalcCorrectVal8 = (float*)Row;
	Row += Bytes4;
	float* CalcCorrectVal9 = (float*)Row;
	Row += Bytes4;
	float* CalcCorrectVal10 = (float*)Row;
	Row += Bytes4;
	int32_t* weaponVfx0 = (int32_t*)Row;
	Row += Bytes4;
	int32_t* weaponVfx1 = (int32_t*)Row;
	Row += Bytes4;
	int32_t* weaponVfx2 = (int32_t*)Row;
	Row += Bytes4;
	int32_t* weaponVfx3 = (int32_t*)Row;
	Row += Bytes4;
	int32_t* weaponVfx4 = (int32_t*)Row;
	Row += Bytes4;
	int32_t* weaponVfx5 = (int32_t*)Row;
	Row += Bytes4;
	int32_t* weaponVfx6 = (int32_t*)Row;
	Row += Bytes4;
	int32_t* weaponVfx7 = (int32_t*)Row;
	Row += Bytes4;
	float* StaminaConsumeRate = (float*)Row;
	Row += Bytes4;
	float* Unk48 = (float*)Row;
	Row += Bytes4;
	float* Unk49 = (float*)Row;
	Row += Bytes4;
	float* Unk50 = (float*)Row;
	Row += Bytes4;
	float* Unk51 = (float*)Row;
	Row += Bytes4;
	float* Unk52 = (float*)Row;
	Row += Bytes4;
	float* Unk53 = (float*)Row;
	Row += Bytes4;
	float* Unk54 = (float*)Row;
	Row += Bytes4;
	float* Unk55 = (float*)Row;
	Row += Bytes4;
	float* Unk56 = (float*)Row;
	Row += Bytes4;
	float* Unk57 = (float*)Row;
	Row += Bytes4;
	float* Unk58 = (float*)Row;
	Row += Bytes4;
	float* Unk59 = (float*)Row;
	Row += Bytes4;
	float* Unk60 = (float*)Row;
	Row += Bytes4;
	float* attackElementCorrectId = (float*)Row;
	Row += Bytes4;
	float* shopPrice = (float*)Row;
	Row += Bytes4;
	uint8_t* Unk62 = (uint8_t*)Row;
	Row += Bytes1;
	uint8_t* MaxNum = (uint8_t*)Row;
	Row += Bytes1;

	Row += Bytes1; //Additional Offset to cut some not yet implemented fields

	uint8_t* Unk65 = (uint8_t*)Row;
	Row += Bytes1;
	int32_t* Unk66 = (int32_t*)Row;
	Row += Bytes4;
	int16_t* SpEff9600 = (int16_t*)Row;
	Row += Bytes2;

	int16_t oldSpspAtkCategory = *spAtkCategory;

	return std::make_tuple(spAtkCategory, oldSpspAtkCategory);

	std::this_thread::sleep_for(std::chrono::seconds(1000));

}

void EquipParamWeaponSwapper(int InfusionRange, int EquipParamWeaponRowID, int TriggerSpeffectID, int TargetspAtkCategory) {
	if (InfusionRange > 1500) {
		return;
		std::cout << "EquipParamWeaponSwapper() - Critical error - Specified Infusionrange for " << EquipParamWeaponRowID << " is higher than maximum implemented, 1500 is the highest\nto fix this error decrease it.";
	}

	int16_t* spAtkCategory0000 = nullptr;
	int16_t oldSpspAtkCategory0000;

	int16_t* spAtkCategory0100 = nullptr;
	int16_t oldSpspAtkCategory0100;

	int16_t* spAtkCategory0200 = nullptr;
	int16_t oldSpspAtkCategory0200;

	int16_t* spAtkCategory0300 = nullptr;
	int16_t oldSpspAtkCategory0300;

	int16_t* spAtkCategory0400 = nullptr;
	int16_t oldSpspAtkCategory0400;

	int16_t* spAtkCategory0500 = nullptr;
	int16_t oldSpspAtkCategory0500;

	int16_t* spAtkCategory0600 = nullptr;
	int16_t oldSpspAtkCategory0600;

	int16_t* spAtkCategory0700 = nullptr;
	int16_t oldSpspAtkCategory0700;

	int16_t* spAtkCategory0800 = nullptr;
	int16_t oldSpspAtkCategory0800;

	int16_t* spAtkCategory0900 = nullptr;
	int16_t oldSpspAtkCategory0900;

	int16_t* spAtkCategory1000 = nullptr;
	int16_t oldSpspAtkCategory1000;

	int16_t* spAtkCategory1100 = nullptr;
	int16_t oldSpspAtkCategory1100;

	int16_t* spAtkCategory1200 = nullptr;
	int16_t oldSpspAtkCategory1200;

	int16_t* spAtkCategory1300 = nullptr;
	int16_t oldSpspAtkCategory1300;

	int16_t* spAtkCategory1400 = nullptr;
	int16_t oldSpspAtkCategory1400;

	int16_t* spAtkCategory1500 = nullptr;
	int16_t oldSpspAtkCategory1500;

	if (InfusionRange >= 0) {
		spAtkCategory0000;
		oldSpspAtkCategory0000;
		std::tie(spAtkCategory0000, oldSpspAtkCategory0000) = EquipParamWeapon(EquipParamWeaponRowID);
	}
	if (InfusionRange >= 100) {
		spAtkCategory0100;
		oldSpspAtkCategory0100;
		std::tie(spAtkCategory0100, oldSpspAtkCategory0100) = EquipParamWeapon(EquipParamWeaponRowID + 100);
	}
	if (InfusionRange >= 200) {
		spAtkCategory0200;
		oldSpspAtkCategory0200;
		std::tie(spAtkCategory0200, oldSpspAtkCategory0200) = EquipParamWeapon(EquipParamWeaponRowID + 200);
	}
	if (InfusionRange >= 300) {
		spAtkCategory0300;
		oldSpspAtkCategory0300;
		std::tie(spAtkCategory0300, oldSpspAtkCategory0300) = EquipParamWeapon(EquipParamWeaponRowID + 300);
	}
	if (InfusionRange >= 400) {
		spAtkCategory0400;
		oldSpspAtkCategory0400;
		std::tie(spAtkCategory0400, oldSpspAtkCategory0400) = EquipParamWeapon(EquipParamWeaponRowID + 400);
	}
	if (InfusionRange >= 500) {
		spAtkCategory0500;
		oldSpspAtkCategory0500;
		std::tie(spAtkCategory0500, oldSpspAtkCategory0500) = EquipParamWeapon(EquipParamWeaponRowID + 500);
	}
	if (InfusionRange >= 600) {
		spAtkCategory0600;
		oldSpspAtkCategory0600;
		std::tie(spAtkCategory0600, oldSpspAtkCategory0600) = EquipParamWeapon(EquipParamWeaponRowID + 600);
	}
	if (InfusionRange >= 700) {
		spAtkCategory0700;
		oldSpspAtkCategory0700;
		std::tie(spAtkCategory0700, oldSpspAtkCategory0700) = EquipParamWeapon(EquipParamWeaponRowID + 700);
	}
	if (InfusionRange >= 800) {
		spAtkCategory0800;
		oldSpspAtkCategory0800;
		std::tie(spAtkCategory0800, oldSpspAtkCategory0800) = EquipParamWeapon(EquipParamWeaponRowID + 800);
	}
	if (InfusionRange >= 900) {
		spAtkCategory0900;
		oldSpspAtkCategory0900;
		std::tie(spAtkCategory0900, oldSpspAtkCategory0900) = EquipParamWeapon(EquipParamWeaponRowID + 900);
	}
	if (InfusionRange >= 1000) {
		spAtkCategory1000;
		oldSpspAtkCategory1000;
		std::tie(spAtkCategory1000, oldSpspAtkCategory1000) = EquipParamWeapon(EquipParamWeaponRowID + 1000);
	}
	if (InfusionRange >= 1100) {
		spAtkCategory1100;
		oldSpspAtkCategory1100;
		std::tie(spAtkCategory1100, oldSpspAtkCategory1100) = EquipParamWeapon(EquipParamWeaponRowID + 1100);
	}
	if (InfusionRange >= 1200) {
		spAtkCategory1200;
		oldSpspAtkCategory1200;
		std::tie(spAtkCategory1200, oldSpspAtkCategory1200) = EquipParamWeapon(EquipParamWeaponRowID + 1200);
	}
	if (InfusionRange >= 1300) {
		spAtkCategory1300;
		oldSpspAtkCategory1300;
		std::tie(spAtkCategory1300, oldSpspAtkCategory1300) = EquipParamWeapon(EquipParamWeaponRowID + 1300);
	}
	if (InfusionRange >= 1400) {
		spAtkCategory1400;
		oldSpspAtkCategory1400;
		std::tie(spAtkCategory1400, oldSpspAtkCategory1400) = EquipParamWeapon(EquipParamWeaponRowID + 1400);
	}
	if (InfusionRange == 1500) {
		spAtkCategory1500;
		oldSpspAtkCategory1500;
		std::tie(spAtkCategory1500, oldSpspAtkCategory1500) = EquipParamWeapon(EquipParamWeaponRowID + 1500);
	}

	while (true) {
		if (spAtkCategory0000 != nullptr) {
			if (*spAtkCategory0000 != TargetspAtkCategory) {
				*spAtkCategory0000 = TargetspAtkCategory;
			}
		}
		if (spAtkCategory0100 != nullptr) {
			if (*spAtkCategory0100 != TargetspAtkCategory) {
				*spAtkCategory0100 = TargetspAtkCategory;
			}
		}
		if (spAtkCategory0200 != nullptr) {
			if (*spAtkCategory0200 != TargetspAtkCategory) {
				*spAtkCategory0200 = TargetspAtkCategory;
			}
		}
		if (spAtkCategory0300 != nullptr) {
			if (*spAtkCategory0300 != TargetspAtkCategory) {
				*spAtkCategory0300 = TargetspAtkCategory;
			}
		}
		if (spAtkCategory0400 != nullptr) {
			if (*spAtkCategory0400 != TargetspAtkCategory) {
				*spAtkCategory0400 = TargetspAtkCategory;
			}
		}
		if (spAtkCategory0500 != nullptr) {
			if (*spAtkCategory0500 != TargetspAtkCategory) {
				*spAtkCategory0500 = TargetspAtkCategory;
			}
		}
		if (spAtkCategory0600 != nullptr) {
			if (*spAtkCategory0600 != TargetspAtkCategory) {
				*spAtkCategory0600 = TargetspAtkCategory;
			}
		}
		if (spAtkCategory0700 != nullptr) {
			if (*spAtkCategory0700 != TargetspAtkCategory) {
				*spAtkCategory0700 = TargetspAtkCategory;
			}
		}
		if (spAtkCategory0800 != nullptr) {
			if (*spAtkCategory0800 != TargetspAtkCategory) {
				*spAtkCategory0800 = TargetspAtkCategory;
			}
		}
		if (spAtkCategory0900 != nullptr) {
			if (*spAtkCategory0900 != TargetspAtkCategory) {
				*spAtkCategory0900 = TargetspAtkCategory;
			}
		}
		if (spAtkCategory1000 != nullptr) {
			if (*spAtkCategory1000 != TargetspAtkCategory) {
				*spAtkCategory1000 = TargetspAtkCategory;
			}
		}
		if (spAtkCategory1100 != nullptr) {
			if (*spAtkCategory1100 != TargetspAtkCategory) {
				*spAtkCategory1100 = TargetspAtkCategory;
			}
		}
		if (spAtkCategory1200 != nullptr) {
			if (*spAtkCategory1200 != TargetspAtkCategory) {
				*spAtkCategory1200 = TargetspAtkCategory;
			}
		}
		if (spAtkCategory1300 != nullptr) {
			if (*spAtkCategory1300 != TargetspAtkCategory) {
				*spAtkCategory1300 = TargetspAtkCategory;
			}
		}
		if (spAtkCategory1400 != nullptr) {
			if (*spAtkCategory1400 != TargetspAtkCategory) {
				*spAtkCategory1400 = TargetspAtkCategory;
			}
		}
		if (spAtkCategory1500 != nullptr) {
			if (*spAtkCategory1500 != TargetspAtkCategory) {
				*spAtkCategory1500 = TargetspAtkCategory;
			}
		}
		if (isSpeffectActive(TriggerSpeffectID) != true) {
			break;
		}
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
	if (spAtkCategory0000 != nullptr) {
		if (*spAtkCategory0000 != oldSpspAtkCategory0000) {
			*spAtkCategory0000 = oldSpspAtkCategory0000;
		}
	}
	if (spAtkCategory0100 != nullptr) {
		if (*spAtkCategory0100 != oldSpspAtkCategory0100) {
			*spAtkCategory0100 = oldSpspAtkCategory0100;
		}
	}
	if (spAtkCategory0200 != nullptr) {
		if (*spAtkCategory0200 != oldSpspAtkCategory0200) {
			*spAtkCategory0200 = oldSpspAtkCategory0200;
		}
	}
	if (spAtkCategory0300 != nullptr) {
		if (*spAtkCategory0300 != oldSpspAtkCategory0300) {
			*spAtkCategory0300 = oldSpspAtkCategory0300;
		}
	}
	if (spAtkCategory0400 != nullptr) {
		if (*spAtkCategory0400 != oldSpspAtkCategory0400) {
			*spAtkCategory0400 = oldSpspAtkCategory0400;
		}
	}
	if (spAtkCategory0500 != nullptr) {
		if (*spAtkCategory0500 != oldSpspAtkCategory0500) {
			*spAtkCategory0500 = oldSpspAtkCategory0500;
		}
	}
	if (spAtkCategory0600 != nullptr) {
		if (*spAtkCategory0600 != oldSpspAtkCategory0600) {
			*spAtkCategory0600 = oldSpspAtkCategory0600;
		}
	}
	if (spAtkCategory0700 != nullptr) {
		if (*spAtkCategory0700 != oldSpspAtkCategory0700) {
			*spAtkCategory0700 = oldSpspAtkCategory0700;
		}
	}
	if (spAtkCategory0800 != nullptr) {
		if (*spAtkCategory0800 != oldSpspAtkCategory0800) {
			*spAtkCategory0800 = oldSpspAtkCategory0800;
		}
	}
	if (spAtkCategory0900 != nullptr) {
		if (*spAtkCategory0900 != oldSpspAtkCategory0900) {
			*spAtkCategory0900 = oldSpspAtkCategory0900;
		}
	}
	if (spAtkCategory1000 != nullptr) {
		if (*spAtkCategory1000 != oldSpspAtkCategory1000) {
			*spAtkCategory1000 = oldSpspAtkCategory1000;
		}
	}
	if (spAtkCategory1100 != nullptr) {
		if (*spAtkCategory1100 != oldSpspAtkCategory1100) {
			*spAtkCategory1100 = oldSpspAtkCategory1100;
		}
	}
	if (spAtkCategory1200 != nullptr) {
		if (*spAtkCategory1200 != oldSpspAtkCategory1200) {
			*spAtkCategory1200 = oldSpspAtkCategory1200;
		}
	}
	if (spAtkCategory1300 != nullptr) {
		if (*spAtkCategory1300 != oldSpspAtkCategory1300) {
			*spAtkCategory1300 = oldSpspAtkCategory1300;
		}
	}
	if (spAtkCategory1400 != nullptr) {
		if (*spAtkCategory1400 != oldSpspAtkCategory1400) {
			*spAtkCategory1400 = oldSpspAtkCategory1400;
		}
	}
	if (spAtkCategory1500 != nullptr) {
		if (*spAtkCategory1500 != oldSpspAtkCategory1500) {
			*spAtkCategory1500 = oldSpspAtkCategory1500;
		}
	}

	std::this_thread::sleep_for(std::chrono::seconds(1));
}

void MainLoop() {
	while (true) {
		std::this_thread::sleep_for(std::chrono::seconds(4));
		std::cout << "MainLoop()" << std::endl;
		int* stamina = (int*)GetPointer(true, PointerPlayerStamina);
		unsigned int* maxstamina = GetPointer(true, PointerPlayerMaxStamina);
		unsigned int* fp = GetPointer(true, PointerPlayerFocusPoints);
		unsigned int* basemaxfp = GetPointer(true, PointerPlayerMaxFocusPoints);

		if (isSpeffectActive(2100) == true) {
			if (*fp < *basemaxfp) {
				FPstaminaDrain(stamina, maxstamina, fp, basemaxfp);
			}
		}
		/*if (isSpeffectActive(2100) == true and MovesetSwap1 == false) {
			std::cout << "startmemeswap";
			std::thread MovesetSwap1(EquipParamWeaponSwapper, 1500, 6100000, 2100, 193);
			MovesetSwap1.detach();
		}*/
	}
}

int StaticAddressPatcher() {

	std::cout << "StaticAddressPatcher - Start" << std::endl << std::endl;

	DWORD oldProtect;
	std::cout << "Patching... MaximumHkobjects" << std::endl;
	if (!VirtualProtect((LPVOID)0x140F09A52, 4, PAGE_EXECUTE_READWRITE, &oldProtect))
		return true;
	*(unsigned int*)0x140F09A52 = GetPrivateProfileIntW(L"Misc", L"MaximumHkobjects", 8192, L".\\HoodiePatcher.ini");
	VirtualProtect((LPVOID)0x140F09A52, 4, oldProtect, &oldProtect);
	std::cout << "MaximumHkojbects = " << *(unsigned int*)0x140F09A52 << std::endl << std::endl;

	std::cout << "Patching... EnableDebugAnimSpeedPlayer" << std::endl;
	if (!VirtualProtect((LPVOID)0x144768F85, 1, PAGE_EXECUTE_READWRITE, &oldProtect))
		return true;
	*(unsigned char*)0x144768F85 = GetPrivateProfileIntW(L"Misc", L"EnableDebugAnimSpeedPlayer", 0, L".\\HoodiePatcher.ini");
	VirtualProtect((LPVOID)0x144768F85, 1, oldProtect, &oldProtect);
	std::cout << "DebugAnimSpeedPlayer = " << *(unsigned int*)0x144768F85 << std::endl << std::endl;

	std::cout << "Patching... EnableDebugAnimSpeedEnemy" << std::endl;
	if (!VirtualProtect((LPVOID)0x144768F81, 1, PAGE_EXECUTE_READWRITE, &oldProtect))
		return true;
	*(unsigned char*)0x144768F81 = GetPrivateProfileIntW(L"Misc", L"EnableDebugAnimSpeedEnemy", 0, L".\\HoodiePatcher.ini");
	VirtualProtect((LPVOID)0x144768F81, 1, oldProtect, &oldProtect);
	std::cout << "DebugAnimSpeedEnemy = " << *(unsigned int*)0x144768F81 << std::endl << std::endl;

	std::cout << "StaticAddressPatcher - End" << std::endl << std::endl;

}

int DifficultyModule() {

	int NG0 = 0x100;
	int NG1 = 0x180;
	int NG2 = 0x200;
	int NG3 = 0x280;
	int NG4 = 0x300;
	int NG5 = 0x380;
	int NG6 = 0x400;
	int NG7 = 0x480;

	std::cout << "Difficulty Module Start" << std::endl;
	int DifficultyLevel = GetPrivateProfileIntW(L"Difficulty", L"DifficultyLevel", 0, L".\\HoodiePatcher.ini");
	std::cout << "Difficulty level = " << DifficultyLevel << std::endl << std::endl;

	std::cout << "NG0 Start" << std::endl;
	NGDifficulty(NG0, 1.0F, DifficultyLevel);
	std::cout << "NG1 Start" << std::endl;
	NGDifficulty(NG1, 1.20F, DifficultyLevel);
	std::cout << "NG2 Start" << std::endl;
	NGDifficulty(NG2, 1.30F, DifficultyLevel);
	std::cout << "NG3 Start" << std::endl;
	NGDifficulty(NG3, 1.40F, DifficultyLevel);
	std::cout << "NG4 Start" << std::endl;
	NGDifficulty(NG4, 1.50F, DifficultyLevel);
	std::cout << "NG5 Start" << std::endl;
	NGDifficulty(NG5, 1.60F, DifficultyLevel);
	std::cout << "NG6 Start" << std::endl;
	NGDifficulty(NG6, 1.70F, DifficultyLevel);
	std::cout << "NG7 Start" << std::endl;
	NGDifficulty(NG7, 1.80F, DifficultyLevel);

	std::cout << "Difficulty Module End" << std::endl << std::endl;
	return true;
}

int NGDifficulty(int RowOffset, float NGMultiplier, int DifficultyLevel) {

	unsigned int* ClearCountCorrectParam = GetPointer(true, PointerClearCountCorrectParam);

	float OffenseMultiplier = ((float)GetPrivateProfileIntW(L"Difficulty", L"EnemyOffenseMultiplier", 100, L".\\HoodiePatcher.ini") / (float)100);
	float DefenseMultiplier = ((float)GetPrivateProfileIntW(L"Difficulty", L"EnemyDefenseMultiplier", 100, L".\\HoodiePatcher.ini") / (float)100);

	byte* NG = ((byte*)ClearCountCorrectParam + RowOffset);
	byte float32 = (byte)0x4;
	std::cout << "         ClearCountCorrectParam Pointer -> " << std::hex << (unsigned int*)ClearCountCorrectParam << std::endl;
	std::cout << "                         NG Row Pointer -> " << std::hex << (unsigned int*)NG << std::endl;
	std::cout << "     Offset from ClearCountCorrectParam -> " << std::hex << ((char*)NG - (char*)ClearCountCorrectParam) << std::endl << std::endl;

	float* MaxHP = (float*)NG;
	NG += float32;
	float* MaxFP = (float*)NG;
	NG += float32;
	float* MaxStamina = (float*)NG;
	NG += float32;
	float* DmgPhys = (float*)NG;
	NG += float32;
	float* DmgSlash = (float*)NG;
	NG += float32;
	float* DmgStrike = (float*)NG;
	NG += float32;
	float* DmgThrust = (float*)NG;
	NG += float32;
	float* DmgStandard = (float*)NG;
	NG += float32;
	float* DmgMagic = (float*)NG;
	NG += float32;
	float* DmgFire = (float*)NG;
	NG += float32;
	float* DmgLightning = (float*)NG;
	NG += float32;
	float* DmgDark = (float*)NG;
	NG += float32;
	float* DefPhys = (float*)NG;
	NG += float32;
	float* DefMagic = (float*)NG;
	NG += float32;
	float* DefFire = (float*)NG;
	NG += float32;
	float* DefLightning = (float*)NG;
	NG += float32;
	float* DefDark = (float*)NG;
	NG += float32;
	float* DmgStamina = (float*)NG;
	NG += float32;
	float* SoulGainMult = (float*)NG;
	NG += float32;
	float* ResistPoison = (float*)NG;
	NG += float32;
	float* ResistToxic = (float*)NG;
	NG += float32;
	float* ResistBleed = (float*)NG;
	NG += float32;
	float* ResistCurse = (float*)NG;
	NG += float32;
	float* ResistFrost = (float*)NG;
	NG += float32;
	float* HpRecover = (float*)NG;
	NG += float32;
	float* PlayerPoiseDmgMultiplier = (float*)NG;
	NG += float32;
	float* subHpRecover = (float*)NG;

	switch (DifficultyLevel) {
	case 1: //Vanilla - NG+0 - equivalent
		*MaxHP = (float)(0.80F * DefenseMultiplier * NGMultiplier);
		*MaxFP = (float)1.0F;
		*MaxStamina = (float)(0.80F * OffenseMultiplier * NGMultiplier);
		*DmgPhys = (float)(0.80F * OffenseMultiplier * NGMultiplier);
		*DmgSlash = (float)1.0F;
		*DmgStrike = (float)1.0F;
		*DmgThrust = (float)1.0F;
		*DmgStandard = (float)(0.80F * OffenseMultiplier * NGMultiplier);
		*DmgMagic = (float)(0.80F * OffenseMultiplier * NGMultiplier);
		*DmgFire = (float)(0.80F * OffenseMultiplier * NGMultiplier);
		*DmgLightning = (float)(0.80F * OffenseMultiplier * NGMultiplier);
		*DmgDark = (float)(0.80F * OffenseMultiplier * NGMultiplier);
		*DefPhys = (float)(0.80F * DefenseMultiplier * NGMultiplier);
		*DefMagic = (float)(0.80F * DefenseMultiplier * NGMultiplier);
		*DefFire = (float)(0.80F * DefenseMultiplier * NGMultiplier);
		*DefLightning = (float)(0.80F * DefenseMultiplier * NGMultiplier);
		*DefDark = (float)(0.80F * DefenseMultiplier * NGMultiplier);
		*DmgStamina = (float)(0.80F * OffenseMultiplier * NGMultiplier);
		*SoulGainMult = (float)(0.80F * (((OffenseMultiplier + DefenseMultiplier) / 2.0F) * NGMultiplier));
		*ResistPoison = (float)(0.80F * ((DefenseMultiplier + NGMultiplier) / 2.0f));
		*ResistToxic = (float)(0.80F * ((DefenseMultiplier + NGMultiplier) / 2.0f));
		*ResistBleed = (float)(0.80F * ((DefenseMultiplier + NGMultiplier) / 2.0f));
		*ResistCurse = (float)(0.80F * ((DefenseMultiplier + NGMultiplier) / 2.0f));
		*ResistFrost = (float)(0.80F * ((DefenseMultiplier + NGMultiplier) / 2.0f));
		*HpRecover = (float)1.0F;
		*PlayerPoiseDmgMultiplier = (float)((1.2F * (1.0F / DefenseMultiplier)) * (1.0F / NGMultiplier));
		*subHpRecover = (float)1.0F;
		break;
	case 2: //Vanilla - NG+1 - equivalent
		*MaxHP = (float)(1.0F * DefenseMultiplier * NGMultiplier);
		*MaxFP = (float)1.0F;
		*MaxStamina = (float)(1.0F * OffenseMultiplier * NGMultiplier);
		*DmgPhys = (float)(1.0F * OffenseMultiplier * NGMultiplier);
		*DmgSlash = (float)1.0F;
		*DmgStrike = (float)1.0F;
		*DmgThrust = (float)1.0F;
		*DmgStandard = (float)(1.0F * OffenseMultiplier * NGMultiplier);
		*DmgMagic = (float)(1.0F * OffenseMultiplier * NGMultiplier);
		*DmgFire = (float)(1.0F * OffenseMultiplier * NGMultiplier);
		*DmgLightning = (float)(1.0F * OffenseMultiplier * NGMultiplier);
		*DmgDark = (float)(1.0F * OffenseMultiplier * NGMultiplier);
		*DefPhys = (float)(1.0F * DefenseMultiplier * NGMultiplier);
		*DefMagic = (float)(1.0F * DefenseMultiplier * NGMultiplier);
		*DefFire = (float)(1.0F * DefenseMultiplier * NGMultiplier);
		*DefLightning = (float)(1.0F * DefenseMultiplier * NGMultiplier);
		*DefDark = (float)(1.0F * DefenseMultiplier * NGMultiplier);
		*DmgStamina = (float)(1.0F * OffenseMultiplier * NGMultiplier);
		*SoulGainMult = (float)(1.0F * (((OffenseMultiplier + DefenseMultiplier) / 2.0F) * NGMultiplier));
		*ResistPoison = (float)(1.0F * ((DefenseMultiplier + NGMultiplier) / 2.0f));
		*ResistToxic = (float)(1.0F * ((DefenseMultiplier + NGMultiplier) / 2.0f));
		*ResistBleed = (float)(1.0F * ((DefenseMultiplier + NGMultiplier) / 2.0f));
		*ResistCurse = (float)(1.0F * ((DefenseMultiplier + NGMultiplier) / 2.0f));
		*ResistFrost = (float)(1.0F * ((DefenseMultiplier + NGMultiplier) / 2.0f));
		*HpRecover = (float)1.0F;
		*PlayerPoiseDmgMultiplier = (float)((1.0F * (1.0F / DefenseMultiplier)) * (1.0F / NGMultiplier));
		*subHpRecover = (float)1.0F;
		break;
	case 3: //Vanilla - NG+4 - Equivalent
		*MaxHP = (float)(1.2F * DefenseMultiplier * NGMultiplier);
		*MaxFP = (float)1.0F;
		*MaxStamina = (float)(1.2F * OffenseMultiplier * NGMultiplier);
		*DmgPhys = (float)(1.2F * OffenseMultiplier * NGMultiplier);
		*DmgSlash = (float)1.0F;
		*DmgStrike = (float)1.0F;
		*DmgThrust = (float)1.0F;
		*DmgStandard = (float)(1.2F * OffenseMultiplier * NGMultiplier);
		*DmgMagic = (float)(1.2F * OffenseMultiplier * NGMultiplier);
		*DmgFire = (float)(1.2F * OffenseMultiplier * NGMultiplier);
		*DmgLightning = (float)(1.2F * OffenseMultiplier * NGMultiplier);
		*DmgDark = (float)(1.2F * OffenseMultiplier * NGMultiplier);
		*DefPhys = (float)(1.1F * DefenseMultiplier * NGMultiplier);
		*DefMagic = (float)(1.1F * DefenseMultiplier * NGMultiplier);
		*DefFire = (float)(1.1F * DefenseMultiplier * NGMultiplier);
		*DefLightning = (float)(1.1F * DefenseMultiplier * NGMultiplier);
		*DefDark = (float)(1.1F * DefenseMultiplier * NGMultiplier);
		*DmgStamina = (float)(1.6F * OffenseMultiplier * NGMultiplier);
		*SoulGainMult = (float)(1.2F * (((OffenseMultiplier + DefenseMultiplier) / 2.0F) * NGMultiplier));
		*ResistPoison = (float)(1.045F * ((DefenseMultiplier + NGMultiplier) / 2.0f));
		*ResistToxic = (float)(1.045F * ((DefenseMultiplier + NGMultiplier) / 2.0f));
		*ResistBleed = (float)(1.045F * ((DefenseMultiplier + NGMultiplier) / 2.0f));
		*ResistCurse = (float)(1.045F * ((DefenseMultiplier + NGMultiplier) / 2.0f));
		*ResistFrost = (float)(1.045F * ((DefenseMultiplier + NGMultiplier) / 2.0f));
		*HpRecover = (float)1.0F;
		*PlayerPoiseDmgMultiplier = (float)((0.85F * (1.0F / DefenseMultiplier)) * (1.0F / NGMultiplier));
		*subHpRecover = (float)1.0F;
		break;
	case 4: //Vanilla - NG+7 - Equivalent
		*MaxHP = (float)(1.4F * DefenseMultiplier * NGMultiplier);
		*MaxFP = (float)1.0F;
		*MaxStamina = (float)(1.275F * OffenseMultiplier * NGMultiplier);
		*DmgPhys = (float)(1.450F * OffenseMultiplier * NGMultiplier);
		*DmgSlash = (float)1.0F;
		*DmgStrike = (float)1.0F;
		*DmgThrust = (float)1.0F;
		*DmgStandard = (float)(1.45F * OffenseMultiplier * NGMultiplier);
		*DmgMagic = (float)(1.45F * OffenseMultiplier * NGMultiplier);
		*DmgFire = (float)(1.45F * OffenseMultiplier * NGMultiplier);
		*DmgLightning = (float)(1.45F * OffenseMultiplier * NGMultiplier);
		*DmgDark = (float)(1.45F * OffenseMultiplier * NGMultiplier);
		*DefPhys = (float)(1.3F * DefenseMultiplier * NGMultiplier);
		*DefMagic = (float)(1.3F * DefenseMultiplier * NGMultiplier);
		*DefFire = (float)(1.3F * DefenseMultiplier * NGMultiplier);
		*DefLightning = (float)(1.3F * DefenseMultiplier * NGMultiplier);
		*DefDark = (float)(1.3F * DefenseMultiplier * NGMultiplier);
		*DmgStamina = (float)(1.9F * OffenseMultiplier * NGMultiplier);
		*SoulGainMult = (float)(1.275F * (((OffenseMultiplier + DefenseMultiplier) / 2.0F) * NGMultiplier));
		*ResistPoison = (float)(1.09F * ((DefenseMultiplier + NGMultiplier) / 2.0f));
		*ResistToxic = (float)(1.09F * ((DefenseMultiplier + NGMultiplier) / 2.0f));
		*ResistBleed = (float)(1.09F * ((DefenseMultiplier + NGMultiplier) / 2.0f));
		*ResistCurse = (float)(1.09F * ((DefenseMultiplier + NGMultiplier) / 2.0f));
		*ResistFrost = (float)(1.09F * ((DefenseMultiplier + NGMultiplier) / 2.0f));
		*HpRecover = (float)1.0F;
		*PlayerPoiseDmgMultiplier = (float)((0.6F * (1.0F / DefenseMultiplier)) * (1.0F / NGMultiplier));
		*subHpRecover = (float)1.0F;
		break;
	default:
		std::cout << "Difficulty Settings Disabled" << std::endl << std::endl;
		return false;
	}

	std::cout << "                Enemy Max HP Multiplier -> " << *MaxHP << std::endl;
	std::cout << "                Enemy Max FP Multiplier -> " << *MaxFP << std::endl;
	std::cout << "           Enemy Max Stamina Multiplier -> " << *MaxStamina << std::endl;
	std::cout << "       Enemy Physical Damage Multiplier -> " << *DmgPhys << std::endl;
	std::cout << "          Enemy Damage Slash Multiplier -> " << *DmgSlash << std::endl;
	std::cout << "          Enemy Damage Blunt Multiplier -> " << *DmgStrike << std::endl;
	std::cout << "         Enemy Damage Thrust Multiplier -> " << *DmgThrust << std::endl;
	std::cout << "       Enemy Damage Standard Multiplier -> " << *DmgStandard << std::endl;
	std::cout << "          Enemy Damage Magic Multiplier -> " << *DmgMagic << std::endl;
	std::cout << "           Enemy Damage Fire Multiplier -> " << *DmgFire << std::endl;
	std::cout << "      Enemy Damage Lightning Multiplier -> " << *DmgLightning << std::endl;
	std::cout << "           Enemy Damage Dark Multiplier -> " << *DmgDark << std::endl;
	std::cout << "      Enemy Defense Physical Multiplier -> " << *DefPhys << std::endl;
	std::cout << "         Enemy Defense Magic Multiplier -> " << *DefMagic << std::endl;
	std::cout << "          Enemy Defense Fire Multiplier -> " << *DefFire << std::endl;
	std::cout << "     Enemy Defense Lightning Multiplier -> " << *DefLightning << std::endl;
	std::cout << "          Enemy Defense Dark Multiplier -> " << *DefDark << "\n";
	std::cout << "        Enemy Damage Stamina Multiplier -> " << *DmgStamina << std::endl;
	std::cout << "            Player Gain Soul Multiplier -> " << *SoulGainMult << std::endl;
	std::cout << "     Enemy Resistance Poison Multiplier -> " << *ResistPoison << std::endl;
	std::cout << "      Enemy Resistance Toxic Multiplier -> " << *ResistToxic << std::endl;
	std::cout << "      Enemy Resistance Bleed Multiplier -> " << *ResistBleed << std::endl;
	std::cout << "      Enemy Resistance Curse Multiplier -> " << *ResistCurse << std::endl;
	std::cout << "      Enemy Resistance Frost Multiplier -> " << *ResistFrost << std::endl;
	std::cout << "           Unknown-HpRecover Multiplier -> " << *HpRecover << std::endl;
	std::cout << "Player VS Enemy Damage Poise Multiplier -> " << *PlayerPoiseDmgMultiplier << std::endl;
	std::cout << "        Unknown-subHpRecover Multiplier -> " << *subHpRecover << std::endl;

	std::cout << std::endl << std::endl;

	return true;

}

unsigned int* GetPointer(bool PointerValid, int PointerName) {
	long long Param = 0x144782838;
	long long BaseB = 0x144768E78;
	long long BaseA = 0x144740178;
	int XA = 0x1F90;
	unsigned int* Address = nullptr;
	if (PointerName == PointerPlayerStatBaseAddress) {
		while (Address == nullptr) {
			Address = mlp<unsigned int>((void*)BaseB, 0x80, XA, 0x18);
			if (PointerValid == false) { break; }
		}
	}
	else if (PointerName == PointerPlayerStamina) {
		while (Address == nullptr) {
			Address = mlp<unsigned int>((void*)BaseB, 0x80, XA, 0x18, 0xF0);
			if (PointerValid == false) { break; }
		}
	}
	else if (PointerName == PointerPlayerMaxStamina) {
		while (Address == nullptr) {
			Address = mlp<unsigned int>((void*)BaseB, 0x80, XA, 0x18, 0xF4);
			if (PointerValid == false) { break; }
		}
	}
	else if (PointerName == PointerPlayerFocusPoints) {
		while (Address == nullptr) {
			Address = mlp<unsigned int>((void*)BaseB, 0x80, XA, 0x18, 0xE4);
			if (PointerValid == false) { break; }
		}
	}
	else if (PointerName == PointerPlayerMaxFocusPoints) {
		while (Address == nullptr) {
			Address = mlp<unsigned int>((void*)BaseB, 0x80, XA, 0x18, 0xEC);
			if (PointerValid == false) { break; }
		}
	}
	else if (PointerName == PointerClearCountCorrectParam) {
		while (Address == nullptr) {
			Address = mlp<unsigned int>((void*)Param, 0x17C8, 0x68, 0x68, 0x0);
			if (PointerValid == false) { break; }
		}
	}
	else if (PointerName == PointerPlayerStaminaLevel) {
		while (Address == nullptr) {
			Address = mlp<unsigned int>((void*)BaseA, 0x10, 0x4C);
			if (PointerValid == false) { break; }
		}
	}
	else if (PointerName == PointerSpeffectListStart) {
		while (Address == nullptr) {
			Address = mlp<unsigned int>((void*)BaseA, 0x10, 0x920, 0x8);
			if (PointerValid == false) { break; }
		}
	}
	else if (PointerName == PointerPlayerSpeffect1Semaphore) {
		while (Address == nullptr) {
			Address = mlp<unsigned int>((void*)BaseA, 0x10, 0x920, 0x8, 0x60);
			if (PointerValid == false) { break; }
		}
	}
	else if (PointerName == PointerEquipParamWeapon) {
		while (Address == nullptr) {
			Address = mlp<unsigned int>((void*)Param, 0x70, 0x68, 0x68, 0x0);
			if (PointerValid == false) { break; }
		}
	}


	return Address;

}

bool isSpeffectActive(int Speffect) {
	byte* SpeffectList = (byte*)GetPointer(true, PointerSpeffectListStart);
	byte SpeffectOffset1 = (byte)0x78;
	byte SpeffectOffset2 = (byte)0x60;
	byte* SpeffectAddress = SpeffectList;
	while (*(long long*)SpeffectAddress != 0) {

		if (*(unsigned int*)(*(long long*)SpeffectAddress + (byte*)SpeffectOffset2) == Speffect) {
			return true;
		}

		SpeffectAddress = (byte*)(*(long long*)SpeffectAddress + SpeffectOffset1);

		if (*(long long*)SpeffectAddress == 0) {
			return false;
		}

	}
	return false;
}

void FPstaminaDrain(int* stamina, unsigned int* maxstamina, unsigned int* fp, unsigned int* basemaxfp) {
	std::cout << "FPstaminaDrain() - Initialize" << std::endl;
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	int fpgain = (int)std::ceil(4 + (6 * *GetPointer(true, PointerPlayerStaminaLevel) / 100));
	while (*fp < *basemaxfp) {
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		if (GetPointer(false, PointerPlayerStatBaseAddress) == nullptr) {
			std::cout << "FPstaminaDrain() - Pointer Error" << std::endl;
			break;
		}
		if ((int)((double)*stamina - ((double)*maxstamina * 0.50)) > 0) {
			*stamina = (int)((double)*stamina - ((double)*maxstamina * 0.50));
			*fp += fpgain;
		}
	}
	std::cout << "FPstaminaDrain() - End" << std::endl;
}