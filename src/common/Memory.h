#pragma once

enum
{
	PF_NONE = 0,
	PF_BACK = 1 << 0,
	PF_IGNORE_FF = 1 << 1,
	PF_IGNORE_00 = 1 << 2,
	PF_UNSAFE = 1 << 3,
};

enum
{
	WF_NONE = 0,
	WF_NO_PROTECT = 1 << 0,
};

inline void *Transpose(void *addr, int offset);
inline void *Absolute(void *addr);

inline void *Relative(void *base, void *func);

inline bool Bounds(void *addr, void *bottom, void *top);

template <typename T>
bool Check(void *addr, T value, int offset = 0)
{
	if (!addr)
		return false;

	return *(T *)&((unsigned char *)addr)[offset] == value;
}

bool ValidateMemory(void *addr);
bool IsExecutable(void *addr);

HMODULE GetBaseAddr(void *addr);
unsigned int GetModuleSize(HMODULE mod);

/********** Detouring **********/

void WriteFunc(void *addr, void *func, unsigned char opcode);
void WriteCall(void *addr, void *func);
void WriteJump(void *addr, void *func);

void *HookRegular(void *addr, void *func, bool as_call);
void *HookWinAPI(void *addr, void *func);
void *HookImport(HMODULE mod, const char *func_name, void *func_addr);
void *HookExport(HMODULE mod, const char *func_name, void *func_addr);

bool RestoreWinAPI(void *addr);

int HookRefAddr(void *addr, void *new_addr, void *start, void *end, unsigned char opcode);
int HookRefCall(void *addr, void *new_addr, void *start, void *end);
int HookRefJump(void *addr, void *new_addr, void *start, void *end);

/********** Searching **********/

template <typename T>
void *Find(void *start, void *left, void *right, T value, int flags = 0)
{
	if (!start || !left || !right)
		return nullptr;

	if ((flags & PF_UNSAFE) != 0)
		right = Transpose(right, sizeof(T));

	bool back = (flags & PF_BACK);

	for (;;)
	{
		if (!Bounds(start, left, right))
			return nullptr;

		switch (sizeof(T))
		{
		case 1: case 2: case 4: case 8:
			if (*(T *)start == value)
				return start;
			break;

		default:
			if (!memcmp(start, &value, sizeof(T)))
				return start;
			break;
		}

		start = back ? Transpose(start, -1) : Transpose(start, 1);
	}

	return nullptr;
}

void *FindPattern(void *start, void *left, void *right, void *value, unsigned int size, int flags = 0);
void *FindRelative(void *start, void *left, void *right, short opcode, int index = 0, bool back = false);
void *FindReference(void *start, void *left, void *right, void *ref_addr, short opcode, bool back = false);

/********** Reading **********/

template <typename T>
T Read(void *addr)
{
	T ret;

	if (!addr)
	{
		memset(&ret, 0, sizeof(T));
		return ret;
	}

	return *(T *)addr;
}

bool ReadPattern(void *addr, int size, void *dest);

/********** Writing **********/

void WritePattern(void *addr, int size, void *buf, int flags = 0);

template <typename T>
void Write(void *addr, T value, int flags = 0)
{
	if (!addr)
		return;

	WritePattern(addr, sizeof(T), &value, flags);
}

template<typename T, size_t N> 
void WritePattern(void *address, const T(&value)[N], int offset = 0)
{
	WritePattern(address, N, (void *)value);
}

void WriteRelative(void *addr, void *base, void *value, int flags = 0);
void Fill(void *addr, unsigned char data, int size);

/********** Other **********/

HANDLE BeginThread(void *addr, void *arg);