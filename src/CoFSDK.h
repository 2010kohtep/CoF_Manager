#pragma once

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

using pfnInit_t = bool(*)(int nVerMajor, int nVerMinor);
using pfnPluginInfo_t = void(*)(plugin_info_t **info);
using pfnGetGameVars = void(*)(cof_data_t *cofdata);