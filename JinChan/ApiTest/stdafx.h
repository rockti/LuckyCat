// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once
#pragma warning( disable : 5053 )
#include "targetver.h"

#include <windows.h>

/////////////////////////////////////////////////////////////////////////
///WindowsËùÐè
/////////////////////////////////////////////////////////////////////////
#include <io.h>
#include <process.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/timeb.h>
#include <winsock.h>
#include <fcntl.h>
#include "conio.h"

#include <atlbase.h>
#include <atlstr.h>
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit
#include <tchar.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include <errno.h>
#include <memory.h>
#include <assert.h>
#include <time.h>
#include <math.h>
#include <float.h>
#include <stdio.h>
#include <tchar.h>
// TODO: reference additional headers your program requires here

/////////////////////////////////////////////////////////////////////////
///STL
/////////////////////////////////////////////////////////////////////////
#pragma warning(disable:4786)
#include <algorithm>
#include <map>
#include <set>
#include <list>
#include <deque>
#include <vector>
#include <queue>
#include <stack>
#include <sstream>
#include <fstream>
#include <iostream>
#include <cmath>
#include <functional>
#include <numeric>
using namespace std;

#include "Logger.h"
