#pragma once

#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <stdio.h>
#include <vector>

#include "GSPlugin.h"

#include "shared/hlsdk/dlls/extdll.h"

#include "shared/hlsdk/cl_dll/wrect.h"
#include "shared/hlsdk/cl_dll/cl_dll.h"

#include "shared/hlsdk/common/const.h"
#include "shared/hlsdk/engine/APIProxy.h"
#include "shared/hlsdk/engine/eiface.h"

#include "shared/hlsdk/pm_shared/pm_defs.h"
#include "shared/hlsdk/extended/basetypes.h"
#include "shared/hlsdk/engine/server_static.h"

#include "public/opcode_len_calc.h"

#include "common/Memory.h"

struct client_static_t
{
	cactive_t state;

	// TODO: Complete struct
};

#include "CoFSDK.h"

#include "sdk/Debug/CoF_Debug.h"

#include "Global.h"

#include "manager/Init.h"
#include "manager/Export.h"