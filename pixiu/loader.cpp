#include "pch.h"


#include <DbgHelp.h>
#pragma comment(lib,"dbghelp.lib")

using namespace std;

struct IATRESULTS {
	enum class FAILUREREASON {
		SUCCESS = 0,
		OTHER = 1,
		NOTFOUND = 2,
		CANNOTPATCH = 3,
	};
	struct FUNCTIONINFO {
		string name;
		size_t ord = 0;
		FAILUREREASON f = FAILUREREASON::SUCCESS;
	};
	struct MODULEINFO {
		string name;
		HINSTANCE handle = 0;
		FAILUREREASON f = FAILUREREASON::SUCCESS;
		vector<FUNCTIONINFO> functions;
	};

	vector<MODULEINFO> modules;
};

void ParseIAT(HINSTANCE h, IATRESULTS& res) {
	// Get IAT size
	DWORD ulsize = 0;
	PIMAGE_IMPORT_DESCRIPTOR pImportDesc = (PIMAGE_IMPORT_DESCRIPTOR)ImageDirectoryEntryToData(h, TRUE, IMAGE_DIRECTORY_ENTRY_IMPORT, &ulsize);
	if (!pImportDesc)
		return;

	// Loop names
	for (; pImportDesc->Name; pImportDesc++) {
		PSTR pszModName = (PSTR)((PBYTE)h + pImportDesc->Name);
		if (!pszModName)
			break;

		IATRESULTS::MODULEINFO m;

		m.name = pszModName;

		HINSTANCE hImportDLL = LoadLibraryA(pszModName);
		if (!hImportDLL) {
			m.f = IATRESULTS::FAILUREREASON::NOTFOUND;
			res.modules.push_back(m);
			continue;
		}
		m.handle = hImportDLL;
		m.f = IATRESULTS::FAILUREREASON::SUCCESS;

		// Get caller's import address table (IAT) for the callee's functions
		PIMAGE_THUNK_DATA pThunk = (PIMAGE_THUNK_DATA)
			((PBYTE)h + pImportDesc->FirstThunk);

		// Replace current function address with new function address
		for (; pThunk->u1.Function; pThunk++) {
			IATRESULTS::FUNCTIONINFO fu;

			FARPROC pfnNew = 0;
			size_t rva = 0;
#ifdef _WIN64
			if (pThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG64)
#else
			if (pThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG32)
#endif
			{
				// Ordinal
#ifdef _WIN64
				size_t ord = IMAGE_ORDINAL64(pThunk->u1.Ordinal);
#else
				size_t ord = IMAGE_ORDINAL32(pThunk->u1.Ordinal);
#endif

				fu.ord = ord;
				m.functions.push_back(fu);
				PROC* ppfn = (PROC*)& pThunk->u1.Function;
				if (!ppfn) {
					fu.f = IATRESULTS::FAILUREREASON::NOTFOUND;
					m.functions.push_back(fu);
					continue;
				}
				rva = (size_t)pThunk;

				char fe[100] = { 0 };
				sprintf_s(fe, 100, "#%zu", ord);
				pfnNew = GetProcAddress(hImportDLL, (LPCSTR)ord);
				if (!pfnNew) {
					fu.f = IATRESULTS::FAILUREREASON::NOTFOUND;
					m.functions.push_back(fu);
					continue;
				}
			} else {
				// Get the address of the function address
				PROC* ppfn = (PROC*)& pThunk->u1.Function;
				if (!ppfn) {
					fu.f = IATRESULTS::FAILUREREASON::NOTFOUND;
					m.functions.push_back(fu);
					continue;
				}
				rva = (size_t)pThunk;
				PSTR fName = (PSTR)h;
				fName += pThunk->u1.Function;
				fName += 2;
				if (!fName)
					break;
				fu.name = fName;
				pfnNew = GetProcAddress(hImportDLL, fName);
				if (!pfnNew) {
					fu.f = IATRESULTS::FAILUREREASON::NOTFOUND;
					m.functions.push_back(fu);
					continue;
				}
			}

			// Patch it now...
			auto hp = GetCurrentProcess();

			if (!WriteProcessMemory(hp, (LPVOID*)rva, &pfnNew, sizeof(pfnNew), NULL) && (ERROR_NOACCESS == GetLastError())) {
				DWORD dwOldProtect;
				if (VirtualProtect((LPVOID)rva, sizeof(pfnNew), PAGE_WRITECOPY, &dwOldProtect)) {
					if (!WriteProcessMemory(GetCurrentProcess(), (LPVOID*)rva, &pfnNew,
						sizeof(pfnNew), NULL)) {
						fu.f = IATRESULTS::FAILUREREASON::CANNOTPATCH;
						continue;
					}
					if (!VirtualProtect((LPVOID)rva, sizeof(pfnNew), dwOldProtect,
						&dwOldProtect)) {
						fu.f = IATRESULTS::FAILUREREASON::CANNOTPATCH;
						continue;
					}
				}
			}
			m.functions.push_back(fu);
		}
		res.modules.push_back(m);
	}
}

//#define STATICLOAD


#ifdef STATICLOAD
extern "C"	void __stdcall e0(HANDLE);
extern "C"	void __stdcall e1();
#pragma comment(lib,"loader.lib")
#endif

int validate() {

	HINSTANCE hL = LoadLibrary(L"loader.exe");
	if (!hL) {
		exit(0);
		return 0;
	}
	IATRESULTS res;
	ParseIAT(hL, res);

	HANDLE hEv = CreateEvent(0, 0, 0, 0);
	void(__stdcall * e)(HANDLE) = (void(__stdcall*)(HANDLE)) GetProcAddress(hL, "e0");
	if (!e) {
		FreeLibrary(hL);
		exit(0);
		return 0;
	}
	std::thread t([&](HANDLE h) {
		e(h);
		}
	, hEv);
	t.detach();
	WaitForSingleObject(hEv, INFINITE);

	void(__stdcall * x)(int b) = (void(__stdcall*)(int b)) GetProcAddress(hL, "e1");
	if (!x) {
		FreeLibrary(hL);
		exit(0);
		return 0;
	}
	x(20190901);
	FreeLibrary(hL);
	return 1;
}