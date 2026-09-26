// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once
#pragma warning( disable : 5053 )
#include "targetver.h"

/////////////////////////////////////////////////////////////////////////
///Common
/////////////////////////////////////////////////////////////////////////
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
#include <string>
#include <sstream>
#include <fstream>
#include <stack>
using namespace std;

/////////////////////////////////////////////////////////////////////////
///Windows所需
/////////////////////////////////////////////////////////////////////////
#include <windows.h>
#include <io.h>
#include <process.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/timeb.h>
#include <winsock.h>
#include <fcntl.h>
#include "conio.h"
#include "Logger.h"

/////////////////////////////////////////////////////////////////////////
///定义标准长度的类型
/////////////////////////////////////////////////////////////////////////
typedef char	INT1;
typedef short	INT2;
typedef int		INT4;
typedef __int64	UF_INT8;
typedef float	REAL4;
typedef double	REAL8;
typedef unsigned char	BYTE;
typedef unsigned short	WORD;

#if defined (_MSC_VER) && (_MSC_VER >= 1020)
#pragma  warning(disable:4819 4996)
#else
typedef unsigned int	DWORD;
#endif


/////////////////////////////////////////////////////////////////////////
///字节序调整
/////////////////////////////////////////////////////////////////////////
inline WORD ChangeEndian(WORD value)
{
	return	((value&0x00ff)<<8) | ((value&0xff00)>>8);
}
inline DWORD ChangeEndian(DWORD value)
{
	return	((value&0x000000ff)<<24) | ((value&0x0000ff00)<<8) | ((value&0x00ff0000)>>8) | ((value&0xff000000)>>24);
}
inline UF_INT8 ChangeEndian(UF_INT8 value)
{
	UF_INT8 temp = value;
	char* pSource = (char*)&value;
	char* pTarget = (char*)&temp;
	pTarget[7] = pSource[0];
	pTarget[6] = pSource[1];
	pTarget[5] = pSource[2];
	pTarget[4] = pSource[3];
	pTarget[3] = pSource[4];
	pTarget[2] = pSource[5];
	pTarget[1] = pSource[6];
	pTarget[0] = pSource[7];
	return temp;
}
#ifndef _BIG_ENDIAN_
#define CHANGE_ENDIAN(value)	value = ::ChangeEndian(value)
#else
#define CHANGE_ENDIAN(value)
#endif


/////////////////////////////////////////////////////////////////////////
///处理文件
/////////////////////////////////////////////////////////////////////////
///在Windows下，我们使用_stat来获取文件状态,使用_unlink来删除文件
#define struct_stat struct _stat
#define STAT	_stat
#define UNLINK _unlink
#define OPEN_FOR_READ (O_BINARY|O_RDONLY)
#define OPEN_FOR_WRITE (O_BINARY|O_RDWR)
#define OPEN_FOR_CREATE (O_BINARY|O_RDWR|O_CREAT|O_TRUNC)

/////////////////////////////////////////////////////////////////////////
///处理共享内存
/////////////////////////////////////////////////////////////////////////
#define ALLOC_SHARE_MEMORY(ptr_addr,id,size)				\
{								\
	HANDLE hfile,hmap;					\
	char buffer[80];					\
	sprintf(buffer,"\\mapping%d.mem",id);			\
	hfile=CreateFile(buffer,GENERIC_READ|GENERIC_WRITE,	\
	FILE_SHARE_READ|FILE_SHARE_WRITE,		\
	NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);	\
	if (hfile==INVALID_HANDLE_VALUE)			\
{							\
	*ptr_addr=NULL;					\
}							\
		else							\
{							\
	hmap=CreateFileMapping(hfile,NULL,		\
	PAGE_READWRITE,0,size,NULL);		\
	if (hmap==NULL)					\
{						\
	*ptr_addr=NULL;				\
}						\
			else						\
{						\
	*ptr_addr=(char *)MapViewOfFile(hmap,	\
	FILE_MAP_WRITE,0,0,size);	\
}						\
}							\
}
#define REUSE_SHARE_MEMORY(ptr_addr,id,size)	ALLOC_SHARE_MEMORY(ptr_addr,id,size)

/////////////////////////////////////////////////////////////////////////
///处理线程
/////////////////////////////////////////////////////////////////////////
#define CREATE_PTHREAD(ptrThreadHandle,ptrThreadId, ptrFunc, ptrArgument)   ( (*ptrThreadHandle = CreateThread(NULL,0, (LPTHREAD_START_ROUTINE)ptrFunc,ptrArgument,0,ptrThreadId))==NULL?0:1)
#define TheadEntry DWORD WINAPI _ThreadEntry(LPVOID pParam)

//**********************************************************************
//处理临界区的区别
//**********************************************************************
#define CRITICAL_VAR		CRITICAL_SECTION
#define INIT_CRITICAL_VAR(v)	InitializeCriticalSection(&(v))
#define ENTER_CRITICAL(v)	EnterCriticalSection(&(v))
#define LEAVE_CRITICAL(v)	LeaveCriticalSection(&(v))
#define DELETE_CRITICAL_VAR(v)	DeleteCriticalSection(&(v))

/////////////////////////////////////////////////////////////////////////
///在windows上生成dll
/////////////////////////////////////////////////////////////////////////
#define DLL_EXPORT __declspec(dllexport)

/////////////////////////////////////////////////////////////////////////
///处理精确时间
/////////////////////////////////////////////////////////////////////////
#define GET_ACCURATE_SEC_TIME(v) 			\
{					\
	SYSTEMTIME t;			\
	GetLocalTime(&t);		\
	v=t.wHour*3600+		\
	t.wMinute*60+	\
	t.wSecond;		\
}

#define GET_ACCURATE_TIME(v) 			\
	{					\
	SYSTEMTIME t;			\
	GetLocalTime(&t);		\
	v=t.wHour*3600000+		\
	t.wMinute*60000+	\
	t.wSecond*1000+		\
	t.wMilliseconds;	\
}

#define GET_ACCURATE_USEC_TIME(v) 			\
	{					\
	SYSTEMTIME t;			\
	GetLocalTime(&t);		\
	v=t.wHour*3600000000+		\
	t.wMinute*60000000+	\
	t.wSecond*1000000+		\
	t.wMilliseconds*1000;	\
	}

#define DOUBLE_NaN DBL_MAX

///在Windows下，我们需要重新定义MAX和MIN宏
#ifndef MAX
#define MAX(a,b) max(a,b)
#endif
#ifndef MIN
#define MIN(a,b) min(a,b)
#endif


//windows下的不同函数的定义
//#define isnan _isnan
//注意window和linux下面的不同，这里规定使用时都要初始化为0
#define snprintf _snprintf

#define sh_open(a,b,c)  _sopen(a,b|O_BINARY,c,S_IREAD | S_IWRITE)

/////////////////////////////////////////////////////////////////////////
///Difference for fpos_t
/////////////////////////////////////////////////////////////////////////
#define FPOS_SET(fposValue,longValue)	(fposValue)=(longValue)
#define FPOS_GET(fposValue)	(fposValue)

/////////////////////////////////////////////////////////////////////////
///处理运行时异常
/////////////////////////////////////////////////////////////////////////
#define RAISE_RUNTIME_ERROR(msg) {											\
	printf("RuntimeError:%s in line %d of file %s\n",msg,__LINE__,__FILE__);\
	fflush(stdout);															\
	char *__pNull = NULL; }

#define RUNTIME_INFO_REPORT(msg) {			\
	printf("%s \n",msg);		\
	char *__pNull = NULL; }

#define PATH_SPLIT '\\'
#define ALL_SPLITS "\\/$"
#define MAX_PATH_LEN 200
void convertPath(char* target, char const* source);
FILE* mfopen(char const* filename, char const* mode);

/*Global exception list*/
#define ParameterError				1	//参数不符合要求
#define RuntimeError			    2	//运行时错误

struct ltstr
{
	bool operator()(const char* s1, const char* s2) const
	{
		return strcmp(s1, s2) < 0;
	}
};
