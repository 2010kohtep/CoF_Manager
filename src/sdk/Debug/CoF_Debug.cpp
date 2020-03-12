#include "precompiled.h"

CDebug gMgrDebug;

void CDebug::OnFrame(double time)
{
	m_nCurrentLine = 0;
}

CDebug::CDebug()
{
	m_Enable = false;
	m_nCurrentLine = 0;
}

const char *CDebug::GetPluginName()
{
	return "IDebug001";
}

IBase *CDebug::GetPluginFactory()
{
	return dynamic_cast<IBase *>(this);
}

void CDebug::Toggle(bool value)
{
	m_Enable = value;
}

void CDebug::NPrint(const char *fmt, ...)
{
	if (!m_Enable)
		return;

	char buf[1024];
	va_list args;
	va_start(args, fmt);
	vsprintf_s(buf, fmt, args);
	va_end(args);

	pEngine->Con_NPrintf(m_nCurrentLine++, buf);
}

void CDebug::NPrint()
{
	if (!m_Enable)
		return;

	m_nCurrentLine++;
}