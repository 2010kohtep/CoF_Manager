#pragma once

struct IBase;

struct IPlugin
{
	IPlugin();

	virtual const char *GetPluginName() abstract;
	virtual IBase *GetPluginFactory() abstract;

	virtual void OnFrame(double time) {}
};

struct CPlugins
{
private:
	friend IPlugin;

	std::vector<IPlugin *> m_Plugins;

private:
	void RegisterPlugin(IPlugin *obj);

public:
	CPlugins();

	IBase *FindPlugin(const char *pszName)
	{
		for (auto &&plugin : m_Plugins)
		{
			if (!_stricmp(pszName, plugin->GetPluginName()))
				return plugin->GetPluginFactory();
		}

		return nullptr;
	}

	void DoFrame(double time);
};

extern CPlugins gPlugins;