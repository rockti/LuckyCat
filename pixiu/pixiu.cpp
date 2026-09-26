
// pixiu.cpp: 定义应用程序的类行为。
//

#include "pch.h"
#include "myafx.h"
#include "pixiu.h"
#include "dlg/pixiuDlg.h"
#include "dlg/RegisterDlg.h"
#include <thread>

#include "utils/Log.h"
#include "runner/Framework.h"
#include "config/Config.h"
#include <atlconv.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


extern "C" void wWinMainCRTStartup();
extern "C" void __stdcall e0(HANDLE hE) {
	wWinMainCRTStartup();
}


#ifdef _WIN64
//#pragma comment(linker, "/EXPORT:WinMainCRTStartup=WinMainCRTStartup")
#pragma comment(linker, "/EXPORT:e0=e0")
#endif

// CpixiuApp

BEGIN_MESSAGE_MAP(CpixiuApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CpixiuApp 构造

CpixiuApp::CpixiuApp()
{
	// 支持重新启动管理器
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}


// 唯一的 CpixiuApp 对象

CpixiuApp theApp;

int validate();

// #include <model/Timer.h>

double g_stdDev = 0;
double g_sumValue = 0;
double g_avgValue = 0;
double g_variance = 0;
size_t g_sampleSize = 0;
std::vector<double> g_sampleVec;

void CalcSTDEV(double sample) {
	g_sampleVec.push_back(sample);
	g_sampleSize = g_sampleVec.size();
	g_sumValue += sample;
	g_avgValue = g_sumValue / g_sampleSize;

	double accum = 0.0;
	auto pos = g_sampleVec.cbegin();
	auto end = g_sampleVec.cend();
	for (; pos != end; ++pos) {
		accum += (*pos - g_avgValue) * (*pos - g_avgValue);
	}
	double variance = accum / g_sampleSize;
	g_stdDev = sqrt(variance);
}

// CpixiuApp 初始化
BOOL CpixiuApp::InitInstance()
{
	// 如果一个运行在 Windows XP 上的应用程序清单指定要
	// 使用 ComCtl32.dll 版本 6 或更高版本来启用可视化方式，
	//则需要 InitCommonControlsEx()。  否则，将无法创建窗口。
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 将它设置为包括所有要在应用程序中使用的
	// 公共控件类。
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	AfxEnableControlContainer();

	// 创建 shell 管理器，以防对话框包含
	// 任何 shell 树视图控件或 shell 列表视图控件。
	CShellManager *pShellManager = new CShellManager;

	// 激活“Windows Native”视觉管理器，以便在 MFC 控件中启用主题
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// 标准初始化
	// 如果未使用这些功能并希望减小
	// 最终可执行文件的大小，则应移除下列
	// 不需要的特定初始化例程
	// 更改用于存储设置的注册表项
	// TODO: 应适当修改该字符串，
	// 例如修改为公司或组织名
	SetRegistryKey(_T("应用程序向导生成的本地应用程序"));

// 	validate();

// 	if (!LicenseChecker::Validate("pi.xiu")) {
// 		exit(1);
// 	}

 	Log::Init("64X", "pixiu.log");
	Log::Info("=================================================================================");
	Log::Info("Start");

	// test wap
// 	money unitPrice = 0.01;
// 
// 	money sellPrice = 419.65;
// 	int sellVol = 3;
// 	money buyPrice = 419.58;
// 	int buyVol = 5;
// 
// 	money quarterUnit = (sellPrice - buyPrice) / 3;
// 
// 	auto sellWeight = sellPrice * buyVol;
// 	auto buyWeight = buyPrice * sellVol;
// 	int totalVol = sellVol + buyVol;
// 
// 	int dir = 99;
// 	double wap = double(sellWeight + buyWeight) / totalVol;
// 	if (wap + quarterUnit > sellPrice) {
// 		dir = 1;
// 	}
// 	else if (wap - quarterUnit < buyPrice) {
// 		dir = 0;
// 	}
// 
// 	int a = 1;


	//test area
// 	money unitPrice = 0.02;
// 	int totalQty = 0;
// 	TickT totalTick = 0;
// 	money reverse1 = 404.9;
// 	int qty1 = 1;
// 	money reverse2 = 404.84;
// 	int qty2 = 1;
// 
// 	Direction reverseDir = Direction::SELL;
// 
// 	totalTick = priceToTick(reverse1, unitPrice);
// 	totalTick += priceToTick(reverse2, unitPrice);
// 	totalQty += qty1;
// 	totalQty += qty2;
// 	double tmpTick = double(totalTick) / totalQty;
// 	TickT avgTick = (Direction::SELL == reverseDir) ? TickT(ceil(tmpTick)) : TickT(floor(tmpTick));
// 	money avgPrice = tickToPrice(avgTick, unitPrice);

// 	int a = 1;
// 	Log::Error(fmt::format("AvgReverse willl be[{:02d}]", a));


	//Load config...
	ConfigLoader configLoader;
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);
	if (__argc < 2) {
		configLoader.Load("pixiu.prod.toml");
	} 
	else {
		USES_CONVERSION;
		auto confFile = __wargv[1];
		configLoader.Load(W2A(confFile));
	}

	Framework& framework = Framework::Instance();
	framework.Start(configLoader.Get());

	CpixiuDlg dlg(configLoader.Get());
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK) {
		// TODO: 在此放置处理何时用
		//  “确定”来关闭对话框的代码
	}
	else if (nResponse == IDCANCEL) {
		// TODO: 在此放置处理何时用
		//  “取消”来关闭对话框的代码
	}
	else if (nResponse == -1) {
		TRACE(traceAppMsg, 0, "警告: 对话框创建失败，应用程序将意外终止。\n");
		TRACE(traceAppMsg, 0, "警告: 如果您在对话框上使用 MFC 控件，则无法 #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS。\n");
	}

	framework.Stop();
	framework.Join();

	// 删除上面创建的 shell 管理器。
	if (pShellManager != nullptr) {
		delete pShellManager;
	}

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
	ControlBarCleanUp();
#endif

	Log::Info("PiXiu--END");
	Log::Shutdown();
	// 由于对话框已关闭，所以将返回 FALSE 以便退出应用程序，
	//  而不是启动应用程序的消息泵。
	return FALSE;
}

