#include "precompiled.h"

void *LocateInterface(const char *pszName)
{
	return gPlugins.FindPlugin(pszName);
}