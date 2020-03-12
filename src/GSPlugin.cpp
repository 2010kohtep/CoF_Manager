#include "precompiled.h"

CPlugins gPlugins;

void CPlugins::RegisterPlugin(IPlugin *obj)
{
	m_Plugins.push_back(obj);
}

CPlugins::CPlugins()
{
	m_Plugins = std::vector<IPlugin *>();
}

void CPlugins::DoFrame(double time)
{
	for (auto &&obj : m_Plugins)
	{
		obj->OnFrame(time);
	}
}

IPlugin::IPlugin()
{
	gPlugins.RegisterPlugin(this);
}