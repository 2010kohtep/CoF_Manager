#include "precompiled.h"

inline LPVOID HeapAlloc(SIZE_T dwBytes)
{
	return HeapAlloc(GetProcessHeap(), 0, dwBytes);
}

inline BOOL HeapFree(LPVOID lpMem)
{
	return HeapFree(GetProcessHeap(), 0, lpMem);
}

void *GetExecMem(unsigned int size)
{
	void *ret;

	ret = HeapAlloc(size);
	if (!ret)
		return nullptr;

	DWORD oldProtect;
	if (!VirtualProtect(ret, size, PAGE_EXECUTE_READWRITE, &oldProtect))
	{
		HeapFree(ret);
		return nullptr;
	}

	return ret;
}

int GetInstructionLength(void *code)
{
	return InstructionLength(code);
}

/********** Basic Memory **********/

inline void *Transpose(void *addr, int offset)
{
	return (void *)(int(addr) + offset);
}

inline void *Absolute(void *addr)
{
	if (!addr)
		return nullptr;

	return Transpose(addr, *(int *)addr + sizeof(void *));
}

inline void *Relative(void *base, void *func)
{
	if (!base || !func)
		return nullptr;

	return Transpose(func, -int(base) - sizeof(void *));
}

inline bool Bounds(void *addr, void *bottom, void *top)
{
	return (uintptr_t(addr) >= uintptr_t(bottom)) && (uintptr_t(addr) <= uintptr_t(top));
}

bool ValidateMemory(void *addr)
{
	if (!addr)
		return false;

	MEMORY_BASIC_INFORMATION mem;
	if (VirtualQuery(addr, &mem, sizeof(mem)) != sizeof(mem))
		return false;

	if (!mem.Protect || mem.Protect == PAGE_NOACCESS)
		return false;

	return true;
}

bool IsExecutable(void *addr)
{
	MEMORY_BASIC_INFORMATION mem;
	if (VirtualQuery(addr, &mem, sizeof(mem)) != sizeof(mem))
		return false;

	if (!mem.Protect || mem.Protect == PAGE_NOACCESS)
		return false;

	switch (mem.Protect)
	{
	case PAGE_EXECUTE: 
	case PAGE_EXECUTE_READ: 
	case PAGE_EXECUTE_READWRITE: 
	case PAGE_EXECUTE_WRITECOPY:
		return true;

	default:
		return false;
	}
}

HMODULE GetBaseAddr(void *addr)
{
	constexpr auto flags = GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
		GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT;

	HMODULE ret;
	if (!GetModuleHandleExA(flags, LPCSTR(addr), &ret))
		return 0;

	return ret;
}

unsigned int GetModuleSize(HMODULE mod)
{
	if (!mod)
		return 0;

	auto dos_header = (PIMAGE_DOS_HEADER)mod;
	auto nt_header = (PIMAGE_NT_HEADERS)((int)dos_header + dos_header->e_lfanew);
	return nt_header->OptionalHeader.SizeOfImage;
}

/********** Detouring **********/

void WriteFunc(void *addr, void *func, unsigned char opcode)
{
	if (!addr || !func)
		return;

	Write<unsigned char>(addr, opcode);
	WriteRelative(Transpose(addr, 1), Transpose(addr, 1), func);
}

void WriteCall(void *addr, void *func)
{
	WriteFunc(addr, func, 0xE8);
}

void WriteJump(void *addr, void *func)
{
	WriteFunc(addr, func, 0xE9);
}

void *HookRegular(void *addr, void *func, bool as_call)
{
	if (!addr || !func)
		return nullptr;
	
	int code_size = 0;
	int size;

	while (code_size < 5)
	{
		size = GetInstructionLength(Transpose(addr, code_size));
		if (!size)
			return nullptr;

		code_size += size;
	}

	void *trampoline = GetExecMem(1 + 4 + code_size);
	if (!trampoline)
		return nullptr;

	memcpy(trampoline, addr, code_size);

	WriteFunc(Transpose(trampoline, code_size), Transpose(addr, code_size), 0xE9);
	WriteFunc(addr, func, as_call ? 0xE8 : 0xE9);

	if (code_size > 5)
	{
		Fill(Transpose(addr, 5), 0x90, code_size - 5);
	}

	return trampoline;
}

void *HookWinAPI(void *addr, void *func)
{
	if (!addr || !func)
		return nullptr;

	if (Check<short>(addr, 0x25FF))
	{
		addr = *(void **)Transpose(addr, 2);
		addr = *(void **)addr;
	}

	if (*(unsigned int *)addr != 0x8B55FF8B) // winapi header check
		return nullptr;

	void *ret;

	WriteJump(addr, func);
	ret = GetExecMem(8);
	*(unsigned int *)ret = 0x90E58955;
	WriteJump(Transpose(ret, 3), Transpose(addr, 5));

	return ret;
}

void *HookImport(HMODULE mod, const char *func_name, void *func_addr)
{
	return nullptr;
}

void *HookExport(HMODULE mod, const char *func_name, void *func_addr)
{
	return nullptr;
}

bool RestoreWinAPI(void *addr)
{
	if (!addr)
		return false;

	void *trampoline;

	trampoline = addr;

	addr = Transpose(addr, 4);
	addr = Absolute(addr);
	addr = Transpose(addr, -5);

	WritePattern(addr, { 0x8B, 0xFF, 0x55, 0x8B, 0xEC });

	if (!HeapFree(trampoline))
		return false;

	return true;
}

int HookRefAddr(void *addr, void *new_addr, void *start, void *end, unsigned char opcode)
{
	if (!addr || !new_addr || !start || !end)
		return 0;

	int op_size = opcode > 0 ? 1 : 0;

	int ret = 0;
	void *p = start;
	end = Transpose(end, -5);

	if (op_size != 0)
	{
		for (;;)
		{
			p = FindRelative(p, start, end, opcode);
			if (!p)
				return ret;

			if (Absolute(Transpose(p, 1)) == addr)
			{
				WriteFunc(p, new_addr, opcode);
				ret++;
			}

			p = Transpose(p, op_size + sizeof(addr));
		}
	}
	else
	{
		p = Find<void *>(p, start, end, new_addr);
		if (!p)
			return ret;

		Write<void *>(p, new_addr);
		p = Transpose(p, sizeof(addr));
		ret++;
	}

	return ret;
}

int HookRefCall(void *addr, void *new_addr, void *start, void *end)
{
	return HookRefAddr(addr, new_addr, start, end, 0xE8);
}

int HookRefJump(void *addr, void *new_addr, void *start, void *end)
{
	return HookRefAddr(addr, new_addr, start, end, 0xE9);
}

/********** Searching **********/

bool CompareMemory(void *p1, void *p2, int len, bool ignore_ff, bool ignore_00)
{
	if (!p1 || !p2)
		return false;

	if (p1 == p2)
		return true;

	void *end;

	end = Transpose(p2, len);

	for (;;)
	{
		if (p2 == end)
			break;

		unsigned char b = *(unsigned char *)p2;

		// TODO: __try __finally?

		if ((ignore_ff && b == 0xFF) || (ignore_00 && b == 0x00))
		{
			p1 = Transpose(p1, 1);
			p2 = Transpose(p2, 1);
			continue;
		}

		if (*(unsigned char *)p1 != *(unsigned char *)p2)
			return false;

		p1 = Transpose(p1, 1);
		p2 = Transpose(p2, 1);
	}

	return true;
}

void *FindPattern(void *start, void *left, void *right, void *value, unsigned int size, int flags)
{
	if (!left || !right)
		return nullptr;

	if (!value || !size)
		return nullptr;

	if ((flags & PF_UNSAFE) != 0)
		right = Transpose(right, -5);

	bool back = (flags & PF_BACK);

	void *ret = start;
	for (;;)
	{
		if (!Bounds(ret, left, right))
			return nullptr;

		if (*(unsigned char *)ret == *(unsigned char *)value)
		{
			if (CompareMemory(ret, value, size, (flags & PF_IGNORE_FF), (flags & PF_IGNORE_00)))
				return ret;
		}

		ret = back ? Transpose(ret, -1) : Transpose(ret, 1);
	}
}

void *FindRelative(void *start, void *left, void *right, short opcode, int index, bool back)
{
	for (;;)
	{
		start = FindReference(start, left, right, nullptr, opcode, back);

		if (!start)
			return start;

		if (index <= 0)
			return start;

		index--;
	}
}

void *FindReference(void *start, void *left, void *right, void *ref_addr, short opcode, bool back)
{
	if (!left || !right || uintptr_t(left) > uintptr_t(right) || !start)
		return nullptr;

	bool is_opcode = opcode != 0;
	bool is_two_bytes = is_opcode && (opcode > 255);
	int flags = back ? PF_BACK : 0;
	void *ret = start;

	right = Transpose(right, -int(sizeof(void *)));

	for (;;)
	{
		if (!Bounds(ret, left, right))
			return nullptr;

		void *f;

		if (is_opcode)
		{
			ret = is_two_bytes ?
				Find<short>(ret, left, right, short(opcode), flags) :
				Find<char>(ret, left, right, char(opcode), flags);

			if (!ret)
				return nullptr;

			f = ret;
			f = Transpose(f, 1);
			f = Absolute(f);
		}
		else
		{
			f = *(void **)ret;
		}

		if (!Bounds(f, left, right))
		{
			ret = back ? Transpose(ret, -1) : Transpose(ret, 1);
			continue;
		}

		if (!ref_addr || ref_addr == f)
			return ret;

		ret = back ? Transpose(ret, -1) : Transpose(ret, 1);
	}

	return nullptr;
}

/********** Reading **********/

bool ReadPattern(void *addr, int size, void *dest)
{
	if (!addr || !size || !dest)
		return false;

	memcpy(dest, addr, size);
	return true;
}

/********** Writing **********/

void WritePattern(void *addr, int size, void *buf, int flags)
{
	if (!addr || !size || !buf)
		return;

	DWORD oldProtect;

	if (!(flags & WF_NO_PROTECT))
	{
		VirtualProtect(addr, size, PAGE_EXECUTE_READWRITE, &oldProtect);
	}

	memcpy(addr, buf, size);

	if (!(flags & WF_NO_PROTECT))
	{
		VirtualProtect(addr, size, oldProtect, &oldProtect);
	}
}

void WriteRelative(void *addr, void *base, void *value, int flags)
{
	Write<void *>(addr, Relative(base, value));
}

void Fill(void *addr, unsigned char data, int size)
{
	if (!addr || !size)
		return;

	DWORD oldProtect;
	VirtualProtect(addr, size, PAGE_EXECUTE_READWRITE, &oldProtect);
	memset(addr, data, size);
	VirtualProtect(addr, size, oldProtect, &oldProtect);
}

/********** Other **********/

HANDLE BeginThread(void *addr, void *arg)
{
	return CreateThread(NULL, 0, LPTHREAD_START_ROUTINE(addr), arg, 0, NULL);
}