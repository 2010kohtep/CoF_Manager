#pragma once

#pragma region Basic Types

struct plugin_info_t
{
	const char *pszName;
	const char *pszDescription;
	const char *pszContact;
	int nVerMajor;
	int nVerMinor;
	int nBuild;
};

struct cof_data_t
{
	int version;

	cl_enginefunc_t *pcl_enginefuncs;
	cldll_func_t *pcl_funcs;
	enginefuncs_t *psv_enginefuncs;

	DLL_FUNCTIONS *p_entityInterface;

	playermove_t *pcl_move;
	edict_t **psv_player;

	globalvars_t **gGlobals;

	client_static_t *pcls;
	server_static_t *psvc;
};

using pfnInit_t = bool(*)(HMODULE hManager, int nVerMajor, int nVerMinor);
using pfnPluginInfo_t = void(*)(plugin_info_t **info);
using pfnGetGameVars_t = void(*)(cof_data_t *cofdata);

#pragma endregion

#pragma region Interfaces

struct IBase
{
	virtual ~IBase() {}

	virtual int GetVersion() { return 0; };
};

using pfnLocateInterface_t = IBase * (*)(const char *pszName);

struct IDebug : IBase
{
	virtual int GetVersion() { return 1; }

	virtual void Toggle(bool value) abstract;

	virtual void NPrint(const char *fmt, ...) abstract;
	virtual void NPrint() abstract;
};

#pragma endregion