#include "precompiled.h"

BOOL WINAPI DllMain(_In_ HINSTANCE hinstDLL, _In_ DWORD fdwReason, _In_ LPVOID lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		GetModuleFileNameA(NULL, gszExeDir, sizeof(gszExeDir));
		auto psz = strrchr(gszExeDir, '\\');
		if (psz) *psz = '\0';

		FindModule("hw.dll", gHLBase, gHLEnd, gHLSize);

		InitManager();
	}

	return TRUE;
}