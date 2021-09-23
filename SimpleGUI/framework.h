#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // Windows ヘッダーからほとんど使用されていない部分を除外する
// Windows ヘッダー ファイル
#include <windows.h>
#include <windowsx.h>
#include <CommCtrl.h>
#include <Shlwapi.h>

// C ランタイム ヘッダー ファイル
#include <crtdbg.h>
#include <memory.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <string>
#include <format>
#include <functional>
#include <map>
#include <list>

#ifdef _DEBUG
#  define DEBUG_NEW new(_CLIENT_BLOCK,__FILE__,__LINE__)
#endif