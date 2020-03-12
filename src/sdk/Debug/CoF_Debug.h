#pragma once

class CDebug : IDebug, IPlugin
{
private:
	bool m_Enable;
	int m_nCurrentLine;

private:
	virtual void OnFrame(double time) override;

public:
	CDebug();

	virtual const char *GetPluginName();
	virtual IBase *GetPluginFactory();

	virtual void Toggle(bool value);
	virtual void NPrint(const char *fmt, ...);
	virtual void NPrint();
};