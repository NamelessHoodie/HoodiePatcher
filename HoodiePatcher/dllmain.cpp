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

int NGDifficulty(int RowOffset, float NGMultiplier, int DifficultyLevel);

int DifficultyModule();

int StaticAddressPatcher();

void FPstaminaDrain();

void StaminaHalfCut();


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

	FPstaminaDrain();

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

	while (true) {
		unsigned int* ClearCountCorrectParam = mlp<unsigned int>((void*)0x144782838, 0x17C8, 0x68, 0x68, 0x0);
		if (ClearCountCorrectParam != nullptr) {

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
	}
}

int NGDifficulty(int RowOffset, float NGMultiplier, int DifficultyLevel) {

	unsigned int* ClearCountCorrectParam = mlp<unsigned int>((void*)0x144782838, 0x17C8, 0x68, 0x68, 0x0);

	float OffenseMultiplier = ((float)GetPrivateProfileIntW(L"Difficulty", L"EnemyOffenseMultiplier", 100, L".\\HoodiePatcher.ini") / (float)100);
	float DefenseMultiplier = ((float)GetPrivateProfileIntW(L"Difficulty", L"EnemyDefenseMultiplier", 100, L".\\HoodiePatcher.ini") / (float)100);

	char* NG = ((char*)ClearCountCorrectParam + RowOffset);
	std::cout << "         ClearCountCorrectParam Pointer -> " << std::hex << (unsigned int*)ClearCountCorrectParam << std::endl;
	std::cout << "                         NG Row Pointer -> " << std::hex << (unsigned int*)NG << std::endl;
	std::cout << "     Offset from ClearCountCorrectParam -> " << std::hex << ((char*)NG - (char*)ClearCountCorrectParam) << std::endl << std::endl;

	char* MaxHP = ((char*)NG + 0x0);
	char* MaxFP = ((char*)NG + 0x4);
	char* MaxStamina = ((char*)NG + 0x8);
	char* DmgPhys = ((char*)NG + 0xC);
	char* DmgSlash = ((char*)NG + 0x10);
	char* DmgStrike = ((char*)NG + 0x14);
	char* DmgThrust = ((char*)NG + 0x18);
	char* DmgStandard = ((char*)NG + 0x1C);
	char* DmgMagic = ((char*)NG + 0x20);
	char* DmgFire = ((char*)NG + 0x24);
	char* DmgLightning = ((char*)NG + 0x28);
	char* DmgDark = ((char*)NG + 0x2C);
	char* DefPhys = ((char*)NG + 0x30);
	char* DefMagic = ((char*)NG + 0x34);
	char* DefFire = ((char*)NG + 0x38);
	char* DefLightning = ((char*)NG + 0x3C);
	char* DefDark = ((char*)NG + 0x40);
	char* DmgStamina = ((char*)NG + 0x44);
	char* SoulGainMult = ((char*)NG + 0x48);
	char* ResistPoison = ((char*)NG + 0x4C);
	char* ResistToxic = ((char*)NG + 0x50);
	char* ResistBleed = ((char*)NG + 0x54);
	char* ResistCurse = ((char*)NG + 0x58);
	char* ResistFrost = ((char*)NG + 0x5C);
	char* HpRecover = ((char*)NG + 0x60);
	char* PlayerPoiseDmgMultiplier = ((char*)NG + 0x64);
	char* subHpRecover = ((char*)NG + 0x68);

	switch (DifficultyLevel) {
	case 1: //Vanilla - NG+0 - equivalent
		*(float*)MaxHP = (float)(0.80F * DefenseMultiplier * NGMultiplier);
		*(float*)MaxFP = (float)1.0F;
		*(float*)MaxStamina = (float)(0.80F * OffenseMultiplier * NGMultiplier);
		*(float*)DmgPhys = (float)(0.80F * OffenseMultiplier * NGMultiplier);
		*(float*)DmgSlash = (float)1.0F;
		*(float*)DmgStrike = (float)1.0F;
		*(float*)DmgThrust = (float)1.0F;
		*(float*)DmgStandard = (float)(0.80F * OffenseMultiplier * NGMultiplier);
		*(float*)DmgMagic = (float)(0.80F * OffenseMultiplier * NGMultiplier);
		*(float*)DmgFire = (float)(0.80F * OffenseMultiplier * NGMultiplier);
		*(float*)DmgLightning = (float)(0.80F * OffenseMultiplier * NGMultiplier);
		*(float*)DmgDark = (float)(0.80F * OffenseMultiplier * NGMultiplier);
		*(float*)DefPhys = (float)(0.80F * DefenseMultiplier * NGMultiplier);
		*(float*)DefMagic = (float)(0.80F * DefenseMultiplier * NGMultiplier);
		*(float*)DefFire = (float)(0.80F * DefenseMultiplier * NGMultiplier);
		*(float*)DefLightning = (float)(0.80F * DefenseMultiplier * NGMultiplier);
		*(float*)DefDark = (float)(0.80F * DefenseMultiplier * NGMultiplier);
		*(float*)DmgStamina = (float)(0.80F * OffenseMultiplier * NGMultiplier);
		*(float*)SoulGainMult = (float)(0.80F * (((OffenseMultiplier + DefenseMultiplier) / 2.0F) * NGMultiplier));
		*(float*)ResistPoison = (float)(0.80F * ((DefenseMultiplier + NGMultiplier) / 2.0f));
		*(float*)ResistToxic = (float)(0.80F * ((DefenseMultiplier + NGMultiplier) / 2.0f));
		*(float*)ResistBleed = (float)(0.80F * ((DefenseMultiplier + NGMultiplier) / 2.0f));
		*(float*)ResistCurse = (float)(0.80F * ((DefenseMultiplier + NGMultiplier) / 2.0f));
		*(float*)ResistFrost = (float)(0.80F * ((DefenseMultiplier + NGMultiplier) / 2.0f));
		*(float*)HpRecover = (float)1.0F;
		*(float*)PlayerPoiseDmgMultiplier = (float)((1.2F * (1.0F / DefenseMultiplier)) * (1.0F / NGMultiplier));
		*(float*)subHpRecover = (float)1.0F;
		break;
	case 2: //Vanilla - NG+1 - equivalent
		*(float*)MaxHP = (float)(1.0F * DefenseMultiplier * NGMultiplier);
		*(float*)MaxFP = (float)1.0F;
		*(float*)MaxStamina = (float)(1.0F * OffenseMultiplier * NGMultiplier);
		*(float*)DmgPhys = (float)(1.0F * OffenseMultiplier * NGMultiplier);
		*(float*)DmgSlash = (float)1.0F;
		*(float*)DmgStrike = (float)1.0F;
		*(float*)DmgThrust = (float)1.0F;
		*(float*)DmgStandard = (float)(1.0F * OffenseMultiplier * NGMultiplier);
		*(float*)DmgMagic = (float)(1.0F * OffenseMultiplier * NGMultiplier);
		*(float*)DmgFire = (float)(1.0F * OffenseMultiplier * NGMultiplier);
		*(float*)DmgLightning = (float)(1.0F * OffenseMultiplier * NGMultiplier);
		*(float*)DmgDark = (float)(1.0F * OffenseMultiplier * NGMultiplier);
		*(float*)DefPhys = (float)(1.0F * DefenseMultiplier * NGMultiplier);
		*(float*)DefMagic = (float)(1.0F * DefenseMultiplier * NGMultiplier);
		*(float*)DefFire = (float)(1.0F * DefenseMultiplier * NGMultiplier);
		*(float*)DefLightning = (float)(1.0F * DefenseMultiplier * NGMultiplier);
		*(float*)DefDark = (float)(1.0F * DefenseMultiplier * NGMultiplier);
		*(float*)DmgStamina = (float)(1.0F * OffenseMultiplier * NGMultiplier);
		*(float*)SoulGainMult = (float)(1.0F * (((OffenseMultiplier + DefenseMultiplier) / 2.0F) * NGMultiplier));
		*(float*)ResistPoison = (float)(1.0F * ((DefenseMultiplier + NGMultiplier) / 2.0f));
		*(float*)ResistToxic = (float)(1.0F * ((DefenseMultiplier + NGMultiplier) / 2.0f));
		*(float*)ResistBleed = (float)(1.0F * ((DefenseMultiplier + NGMultiplier) / 2.0f));
		*(float*)ResistCurse = (float)(1.0F * ((DefenseMultiplier + NGMultiplier) / 2.0f));
		*(float*)ResistFrost = (float)(1.0F * ((DefenseMultiplier + NGMultiplier) / 2.0f));
		*(float*)HpRecover = (float)1.0F;
		*(float*)PlayerPoiseDmgMultiplier = (float)((1.0F * (1.0F / DefenseMultiplier)) * (1.0F / NGMultiplier));
		*(float*)subHpRecover = (float)1.0F;
		break;
	case 3: //Vanilla - NG+4 - Equivalent
		*(float*)MaxHP = (float)(1.2F * DefenseMultiplier * NGMultiplier);
		*(float*)MaxFP = (float)1.0F;
		*(float*)MaxStamina = (float)(1.2F * OffenseMultiplier * NGMultiplier);
		*(float*)DmgPhys = (float)(1.2F * OffenseMultiplier * NGMultiplier);
		*(float*)DmgSlash = (float)1.0F;
		*(float*)DmgStrike = (float)1.0F;
		*(float*)DmgThrust = (float)1.0F;
		*(float*)DmgStandard = (float)(1.2F * OffenseMultiplier * NGMultiplier);
		*(float*)DmgMagic = (float)(1.2F * OffenseMultiplier * NGMultiplier);
		*(float*)DmgFire = (float)(1.2F * OffenseMultiplier * NGMultiplier);
		*(float*)DmgLightning = (float)(1.2F * OffenseMultiplier * NGMultiplier);
		*(float*)DmgDark = (float)(1.2F * OffenseMultiplier * NGMultiplier);
		*(float*)DefPhys = (float)(1.1F * DefenseMultiplier * NGMultiplier);
		*(float*)DefMagic = (float)(1.1F * DefenseMultiplier * NGMultiplier);
		*(float*)DefFire = (float)(1.1F * DefenseMultiplier * NGMultiplier);
		*(float*)DefLightning = (float)(1.1F * DefenseMultiplier * NGMultiplier);
		*(float*)DefDark = (float)(1.1F * DefenseMultiplier * NGMultiplier);
		*(float*)DmgStamina = (float)(1.6F * OffenseMultiplier * NGMultiplier);
		*(float*)SoulGainMult = (float)(1.2F * (((OffenseMultiplier + DefenseMultiplier) / 2.0F) * NGMultiplier));
		*(float*)ResistPoison = (float)(1.045F * ((DefenseMultiplier + NGMultiplier) / 2.0f));
		*(float*)ResistToxic = (float)(1.045F * ((DefenseMultiplier + NGMultiplier) / 2.0f));
		*(float*)ResistBleed = (float)(1.045F * ((DefenseMultiplier + NGMultiplier) / 2.0f));
		*(float*)ResistCurse = (float)(1.045F * ((DefenseMultiplier + NGMultiplier) / 2.0f));
		*(float*)ResistFrost = (float)(1.045F * ((DefenseMultiplier + NGMultiplier) / 2.0f));
		*(float*)HpRecover = (float)1.0F;
		*(float*)PlayerPoiseDmgMultiplier = (float)((0.85F * (1.0F / DefenseMultiplier)) * (1.0F / NGMultiplier));
		*(float*)subHpRecover = (float)1.0F;
		break;
	case 4: //Vanilla - NG+7 - Equivalent
		*(float*)MaxHP = (float)(1.4F * DefenseMultiplier * NGMultiplier);
		*(float*)MaxFP = (float)1.0F;
		*(float*)MaxStamina = (float)(1.275F * OffenseMultiplier * NGMultiplier);
		*(float*)DmgPhys = (float)(1.450F * OffenseMultiplier * NGMultiplier);
		*(float*)DmgSlash = (float)1.0F;
		*(float*)DmgStrike = (float)1.0F;
		*(float*)DmgThrust = (float)1.0F;
		*(float*)DmgStandard = (float)(1.45F * OffenseMultiplier * NGMultiplier);
		*(float*)DmgMagic = (float)(1.45F * OffenseMultiplier * NGMultiplier);
		*(float*)DmgFire = (float)(1.45F * OffenseMultiplier * NGMultiplier);
		*(float*)DmgLightning = (float)(1.45F * OffenseMultiplier * NGMultiplier);
		*(float*)DmgDark = (float)(1.45F * OffenseMultiplier * NGMultiplier);
		*(float*)DefPhys = (float)(1.3F * DefenseMultiplier * NGMultiplier);
		*(float*)DefMagic = (float)(1.3F * DefenseMultiplier * NGMultiplier);
		*(float*)DefFire = (float)(1.3F * DefenseMultiplier * NGMultiplier);
		*(float*)DefLightning = (float)(1.3F * DefenseMultiplier * NGMultiplier);
		*(float*)DefDark = (float)(1.3F * DefenseMultiplier * NGMultiplier);
		*(float*)DmgStamina = (float)(1.9F * OffenseMultiplier * NGMultiplier);
		*(float*)SoulGainMult = (float)(1.275F * (((OffenseMultiplier + DefenseMultiplier) / 2.0F) * NGMultiplier));
		*(float*)ResistPoison = (float)(1.09F * ((DefenseMultiplier + NGMultiplier) / 2.0f));
		*(float*)ResistToxic = (float)(1.09F * ((DefenseMultiplier + NGMultiplier) / 2.0f));
		*(float*)ResistBleed = (float)(1.09F * ((DefenseMultiplier + NGMultiplier) / 2.0f));
		*(float*)ResistCurse = (float)(1.09F * ((DefenseMultiplier + NGMultiplier) / 2.0f));
		*(float*)ResistFrost = (float)(1.09F * ((DefenseMultiplier + NGMultiplier) / 2.0f));
		*(float*)HpRecover = (float)1.0F;
		*(float*)PlayerPoiseDmgMultiplier = (float)((0.6F * (1.0F / DefenseMultiplier)) * (1.0F / NGMultiplier));
		*(float*)subHpRecover = (float)1.0F;
		break;
	default:
		std::cout << "Difficulty Settings Disabled" << std::endl << std::endl;
		return false;
	}

	std::cout << "                Enemy Max HP Multiplier -> " << *(float*)MaxHP << std::endl;
	std::cout << "                Enemy Max FP Multiplier -> " << *(float*)MaxFP << std::endl;
	std::cout << "           Enemy Max Stamina Multiplier -> " << *(float*)MaxStamina << std::endl;
	std::cout << "       Enemy Physical Damage Multiplier -> " << *(float*)DmgPhys << std::endl;
	std::cout << "          Enemy Damage Slash Multiplier -> " << *(float*)DmgSlash << std::endl;
	std::cout << "          Enemy Damage Blunt Multiplier -> " << *(float*)DmgStrike << std::endl;
	std::cout << "         Enemy Damage Thrust Multiplier -> " << *(float*)DmgThrust << std::endl;
	std::cout << "       Enemy Damage Standard Multiplier -> " << *(float*)DmgStandard << std::endl;
	std::cout << "          Enemy Damage Magic Multiplier -> " << *(float*)DmgMagic << std::endl;
	std::cout << "           Enemy Damage Fire Multiplier -> " << *(float*)DmgFire << std::endl;
	std::cout << "      Enemy Damage Lightning Multiplier -> " << *(float*)DmgLightning << std::endl;
	std::cout << "           Enemy Damage Dark Multiplier -> " << *(float*)DmgDark << std::endl;
	std::cout << "      Enemy Defense Physical Multiplier -> " << *(float*)DefPhys << std::endl;
	std::cout << "         Enemy Defense Magic Multiplier -> " << *(float*)DefMagic << std::endl;
	std::cout << "          Enemy Defense Fire Multiplier -> " << *(float*)DefFire << std::endl;
	std::cout << "     Enemy Defense Lightning Multiplier -> " << *(float*)DefLightning << std::endl;
	std::cout << "          Enemy Defense Dark Multiplier -> " << *(float*)DefDark << "\n";
	std::cout << "        Enemy Damage Stamina Multiplier -> " << *(float*)DmgStamina << std::endl;
	std::cout << "            Player Gain Soul Multiplier -> " << *(float*)SoulGainMult << std::endl;
	std::cout << "     Enemy Resistance Poison Multiplier -> " << *(float*)ResistPoison << std::endl;
	std::cout << "      Enemy Resistance Toxic Multiplier -> " << *(float*)ResistToxic << std::endl;
	std::cout << "      Enemy Resistance Bleed Multiplier -> " << *(float*)ResistBleed << std::endl;
	std::cout << "      Enemy Resistance Curse Multiplier -> " << *(float*)ResistCurse << std::endl;
	std::cout << "      Enemy Resistance Frost Multiplier -> " << *(float*)ResistFrost << std::endl;
	std::cout << "           Unknown-HpRecover Multiplier -> " << *(float*)HpRecover << std::endl;
	std::cout << "Player VS Enemy Damage Poise Multiplier -> " << *(float*)PlayerPoiseDmgMultiplier << std::endl;
	std::cout << "        Unknown-subHpRecover Multiplier -> " << *(float*)subHpRecover << std::endl;

	std::cout << std::endl << std::endl;

	return true;

}

void FPstaminaDrain() {
	std::cout << "FPstaminaDrain() - Start" << std::endl;
	std::thread StaminaDebuff(StaminaHalfCut);
	int Second = 1000;
	long long BaseB = 0x144768E78;
	int XA = 0x1F90;
	while (true) {
		int* stamina = mlp<int>((void*)BaseB, 0x80, XA, 0x18, 0xF0);
		unsigned int* maxstamina = mlp<unsigned int>((void*)BaseB, 0x80, XA, 0x18, 0xF4);
		unsigned int* fp = mlp<unsigned int>((void*)BaseB, 0x80, XA, 0x18, 0xE4);
		unsigned int* maxfp = mlp<unsigned int>((void*)BaseB, 0x80, XA, 0x18, 0xE8);
		unsigned int* basemaxfp = mlp<unsigned int>((void*)BaseB, 0x80, XA, 0x18, 0xEC);
		unsigned int* HeroBaseAddress = mlp<unsigned int>((void*)BaseB, 0x80, XA, 0x18);
		if (HeroBaseAddress != nullptr) {
			if (stamina != nullptr) {
				if (*fp < *basemaxfp) {
					if ((int)((double)*stamina - ((double)*maxstamina * 0.40)) > 0) {
						std::cout << "looping stamina drain fpgib" << std::endl;
						*stamina = (int)((double)*stamina - ((double)*maxstamina * 0.40));
						*fp += (byte)5;
					}
				}
			}
		}
	}
}

void StaminaHalfCut() {
	std::cout << "StaminaHalfCut() - Start" << std::endl;
	long long BaseB = 0x144768E78;
	int XA = 0x1F90;
	while (true) {
		unsigned int* HeroBaseAddress = mlp<unsigned int>((void*)BaseB, 0x80, XA, 0x18);
		int* stamina = mlp<int>((void*)BaseB, 0x80, XA, 0x18, 0xF0);
		int* maxstamina = mlp<int>((void*)BaseB, 0x80, XA, 0x18, 0xF4);
		if (HeroBaseAddress != nullptr) {
			if (stamina != nullptr) {
				if (*stamina > (*maxstamina / 2)) {
					*stamina = (*maxstamina / 2);
				}
			}
		}
	}
}
