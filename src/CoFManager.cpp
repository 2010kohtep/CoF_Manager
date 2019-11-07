#include "precompiled.h"

void FindModule(const char *name, HMODULE &base, void *&end, unsigned int &size)
{
	base = nullptr;
	end = nullptr;
	size = 0;


	base = GetModuleHandleA(name);
	if (!base)
		return;
	
	size = GetModuleSize(base);
	end = Transpose(base, size - 1);
}

void CreateCoFDataStructure()
{
	gCoFData.version = 1;

	gCoFData.pcl_enginefuncs = pEngine;
	gCoFData.pcl_funcs = pClient;
	gCoFData.psv_enginefuncs = pServer;

	gCoFData.p_entityInterface = pgEntityInterface;

	gCoFData.pcl_move = gclmove;
	gCoFData.psv_player = gsv_player;

	gCoFData.gGlobals = gGlobals;

	gCoFData.pcls = gpcls;
	gCoFData.psvc = nullptr;
}

void hkHudFrame(double time);

void InitMainStructs()
{
	pEngine = (cl_enginefunc_t *)Transpose(gHLBase, 0x001A9A90);
	pServer = (enginefuncs_t *)Transpose(gHLBase, 0x001CED18);
	pClient = (cldll_func_t *)Transpose(gHLBase, 0x012B9BE0);
	pgEntityInterface = (DLL_FUNCTIONS *)Transpose(gHLBase, 0x0080D8C0);

	gclmove = (playermove_t *)Transpose(gHLBase, 0x0107F7A0);
	gsv_player = (edict_t **)Transpose(gHLBase, 0x0080E07C);
	gpcls = (client_static_t *)Transpose(gHLBase, 0x010CF280);
	gpsvc = nullptr;

	memcpy(&Client, pClient, sizeof(cldll_func_t));

	pClient->pHudFrame = hkHudFrame;
}

void InitAdditionalStructs()
{
	gGlobals = (globalvars_t **)Transpose(gModBase, 0x002196B0);
}

void hkHudFrame(double time)
{
	pClient->pHudFrame = Client.pHudFrame;

	FindModule("client.dll", gCLBase, gCLEnd, gCLSize);
	FindModule("hl.dll", gModBase, gModEnd, gModSize);

	InitAdditionalStructs();
	CreateCoFDataStructure();

	for (auto &&m : gCofModules)
	{
		if (!m->pInit)
		{
			pEngine->Con_Printf("WARNING! Module %s does not contain initialization function.\n", m->pszName);
			continue;
		}

		if (!m->pPluginInfo)
		{
			pEngine->Con_Printf("WARNING! Module %s does not contain plugin info function.\n", m->pszName);
			continue;
		}

		plugin_info_t *info;

		m->pPluginInfo(&info);

		if (m->pGetGameVars)
			m->pGetGameVars(&gCoFData);

		if (!m->pInit(COFMGR_VERSION_MAJOR, COFMGR_VERSION_MINOR))
		{
			pEngine->Con_Printf("WARNING! Initialization failed for plugin %s.\n", m->pszName);
			continue;
		}

		pEngine->Con_Printf("%s loaded. ", info->pszName);
		if (!info->nVerMajor && !info->nVerMinor)
			pEngine->Con_Printf("Build: %d\n", info->nBuild);
		else
			pEngine->Con_Printf("Version: %d.%d.%d\n", info->nVerMajor, info->nVerMinor, info->nBuild);
		
		m->bInited = true;
	}

	Client.pHudFrame(time);
}

template <typename T>
void GetProcAddress(T &pVar, HMODULE hModule, LPCSTR lpProcName)
{
	pVar = (T)GetProcAddress(hModule, lpProcName);
}

void LoadPlugins()
{
	char szPluginsDir[MAX_PATH];
	char szPluginsTmp[MAX_PATH];

	sprintf(szPluginsDir, "%s\\cryoffear\\addons", gszExeDir);
	sprintf(szPluginsTmp, "%s\\*.dll", szPluginsDir);

	WIN32_FIND_DATA file = { 0 };
	void *hFile = FindFirstFileA(szPluginsTmp, &file);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		do
		{
			char szPluginName[MAX_PATH];

			sprintf(szPluginName, "%s\\%s", szPluginsDir, file.cFileName);
			HMODULE hModule = LoadLibraryA(szPluginName);

			if (!hModule)
			{
				pEngine->Con_Printf("WARNING! Could not load CoF module %s\n", szPluginName);
				continue;
			}

			auto mod = new cof_module_t;

			mod->pszName = _strdup(file.cFileName);
			mod->bInited = false;
			GetProcAddress(mod->pInit, hModule, "Init");
			GetProcAddress(mod->pPluginInfo, hModule, "GetPluginInfo");
			GetProcAddress(mod->pGetGameVars, hModule, "GetGameVars");

			gCofModules.push_back(mod);
		} while (FindNextFileA(hFile, &file) != 0);

		FindClose(hFile);
	}
}

BOOL WINAPI DllMain(_In_ HINSTANCE hinstDLL, _In_ DWORD fdwReason, _In_ LPVOID lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		GetModuleFileNameA(NULL, gszExeDir, sizeof(gszExeDir));
		auto psz = strrchr(gszExeDir, '\\');
		if (psz) *psz = '\0';

		FindModule("hw.dll", gHLBase, gHLEnd, gHLSize);

		InitMainStructs();

		LoadPlugins();
	}

	return TRUE;
}