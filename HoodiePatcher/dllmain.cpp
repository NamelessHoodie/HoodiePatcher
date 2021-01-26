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

int NGDifficulty(int RowOffset, float NGMultiplier, int DifficultyLevel);

int DifficultyModule();

int StaticAddressPatcher();

void FPstaminaDrain(int* stamina, unsigned int* maxstamina, unsigned int* fp, unsigned int* basemaxfp);

void MainLoop();

unsigned int* GetPointer(bool PointerValid, int PointerName);

bool isSpeffectActive(int Speffect);


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

unsigned int* GetPointer(bool PointerValid, int PointerName) {
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
			Address = mlp<unsigned int>((void*)0x144782838, 0x17C8, 0x68, 0x68, 0x0);
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


	return Address;

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
	}
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

bool isSpeffectActive(int Speffect) {
	if (GetPointer(false, PointerPlayerSpeffect1Semaphore) == nullptr) {
		std::cout << "isSpeffectActive() - Pointer Error... Returning\n";
		return false;
	}
	byte* SpeffectList = (byte*)GetPointer(true, PointerSpeffectListStart);
	byte SpeffectOffset1 = (byte)0x78;
	byte SpeffectOffset2 = (byte)0x60;
	byte* SpeffectAddress = SpeffectList;
	while (*(long long*)SpeffectAddress != 0) {

		if (*(unsigned int*)(*(long long*)SpeffectAddress + (byte*)SpeffectOffset2) == Speffect) {
			std::cout << "isSpeffectActive() - Speffect: " << std::dec << Speffect << " found" << std::endl;
			return true;
		}

		SpeffectAddress = (byte*)(*(long long*)SpeffectAddress + SpeffectOffset1);

		if (*(long long*)SpeffectAddress == 0) {
			return false;
		}

	}
	return false;
}