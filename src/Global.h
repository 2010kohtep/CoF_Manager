#pragma once

#define GLOBAL __declspec(selectany)

#define COFMGR_VERSION_MAJOR 0
#define COFMGR_VERSION_MINOR 1

GLOBAL cl_enginefunc_t *pEngine;
GLOBAL cldll_func_t *pClient;
GLOBAL cldll_func_t Client;
GLOBAL enginefuncs_t *pServer;
GLOBAL DLL_FUNCTIONS *pgEntityInterface;

GLOBAL playermove_t *gclmove;
GLOBAL edict_t **gsv_player;
GLOBAL globalvars_t **gGlobals;

GLOBAL client_static_t *gpcls;
GLOBAL server_static_t *gpsvc;

GLOBAL char gszExeDir[MAX_PATH];

// Modules

GLOBAL HMODULE gHLBase;
GLOBAL void *gHLEnd;
GLOBAL unsigned int gHLSize;

GLOBAL HMODULE gModBase;
GLOBAL void *gModEnd;
GLOBAL unsigned int gModSize;

GLOBAL HMODULE gCLBase;
GLOBAL void *gCLEnd;
GLOBAL unsigned int gCLSize;

GLOBAL HMODULE gGUIBase;
GLOBAL void *gGUIEnd;
GLOBAL unsigned int gGUISize;

struct cof_module_t
{
	const char *pszName;
	bool bInited;

	pfnInit_t pInit;
	pfnPluginInfo_t pPluginInfo;
	pfnGetGameVars pGetGameVars;
};

GLOBAL std::vector<cof_module_t *> gCofModules;

GLOBAL cof_data_t gCoFData;