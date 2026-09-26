
// pixiuDlg.cpp: 实现文件
//

#include "pch.h"
#include "myafx.h"
#include "pixiu.h"
#include "pixiuDlg.h"
#include "afxdialogex.h"
#include "dlg/LoginDlg.h"
#include "dlg/FundSettingDlg.h"

#include "notify/UIPublisher.h"
#include "model/dealing/ExchangeRequest.h"
#include "model/factory/InstrumentFactory.h"
#include "runner/Framework.h"
#include "strategy/StrategyFactory.h"
#include "strategy/mct/MCTStrategy.h"



// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx {
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX) {
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX) {
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

IMPLEMENT_DYNAMIC(CColorBtn, CButton)
BEGIN_MESSAGE_MAP(CColorBtn, CButton)
	ON_WM_DRAWITEM()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

// CpixiuDlg 对话框
CpixiuDlg::CpixiuDlg(Config& config, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_PIXIU_DIALOG, pParent), m_config(config), m_framework(Framework::Instance()), m_businessPtr(NULL)
	, m_bOpenBtn(RGB(255, 106, 106))		// IndianRed
	, m_sOpenBtn(RGB(114, 238, 144))		// LimeGreen
	, m_bOffsetBtn(RGB(255, 106, 106))		// IndianRed
	, m_sOffsetBtn(RGB(114, 238, 144))		// Chartreuse
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_platformIDMap[ConnPlatform::CTP] = IDC_FUTURES_LOGIN_BTN;
	m_platformIDMap[ConnPlatform::HFT] = IDC_FUTURES_LOGIN_BTN;
	m_platformIDMap[ConnPlatform::HKEX_ESUNNY] = IDC_OVERSEA_LOGIN_BTN;
	m_platformIDMap[ConnPlatform::SGE] = IDC_DEFERSPOT_LOGIN_BTN;

	m_statusInfoMap[ExchangeStatus::BEFORE_TRADING] = "开盘前";
	m_statusInfoMap[ExchangeStatus::NO_TRADING] = "非交易";
	m_statusInfoMap[ExchangeStatus::CONTINOUS] = "连续交易";
	m_statusInfoMap[ExchangeStatus::AUCTION_ORDERING] = "竞价报单";
	m_statusInfoMap[ExchangeStatus::AUCTION_BALANCE] = "竞价平衡";
	m_statusInfoMap[ExchangeStatus::AUCTION_MATCH] = "竞价撮合";
	m_statusInfoMap[ExchangeStatus::DELIVERY_APPEND] = "交割申报";
	m_statusInfoMap[ExchangeStatus::DELIVERY_END] = "交割结束";
	m_statusInfoMap[ExchangeStatus::MID_APPEND] = "中立仓申报";
	m_statusInfoMap[ExchangeStatus::MID_BALANCE] = "中立仓平衡";
	m_statusInfoMap[ExchangeStatus::CLOSED] = "收盘";
	m_statusInfoMap[ExchangeStatus::UNKNOW] = "未知状态";
}

void CpixiuDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STRATEGY_TAB, m_dialogTabCtrl);
	DDX_Control(pDX, IDC_DEALING_LIST, m_dealingList);
	DDX_Control(pDX, IDC_POSITION_LIST, m_positionList);
	DDX_Control(pDX, IDC_INSTRUMENT_COMBO, m_instrumentCombo);
	DDX_Control(pDX, IDC_AMT_EDIT, m_amtEdit);
	DDX_Control(pDX, IDC_PRICE_EDIT, m_priceEdit);
	DDX_Control(pDX, IDC_PRICE_SPIN, m_priceSpin);
	DDX_Control(pDX, IDC_BUY_SPIN, m_buyOffsetSpin);
	DDX_Control(pDX, IDC_SELL_SPIN, m_sellOffsetSpin);
	DDX_Control(pDX, IDC_BOPEN_BTN, m_bOpenBtn);
	DDX_Control(pDX, IDC_SOPEN_BTN, m_sOpenBtn);
	DDX_Control(pDX, IDC_BOFFSET_BTN, m_bOffsetBtn);
	DDX_Control(pDX, IDC_SOFFSET_BTN, m_sOffsetBtn);
	DDX_Control(pDX, IDC_SOFFSET_BTN, m_sOffsetBtn);
	DDX_Control(pDX, IDC_PLATFORM_COMBO, m_platformCombo);
	DDX_Control(pDX, IDC_LOG_LIST, m_logList);
}

BEGIN_MESSAGE_MAP(CpixiuDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_CLOSE()
	ON_WM_CTLCOLOR()
	ON_WM_QUERYDRAGICON()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_SET_FUND_BTN, &CpixiuDlg::OnBnClickedSetFundsBtn)
	ON_BN_CLICKED(IDC_FUTURE_LOGIN_BTN, &CpixiuDlg::OnBnClickedFuturesLoginBtn)
	ON_BN_CLICKED(IDC_DEFERSPOT_LOGIN_BTN, &CpixiuDlg::OnBnClickedDeferSpotLoginBtn)
	ON_BN_CLICKED(IDC_OVERSEA_LOGIN_BTN, &CpixiuDlg::OnBnClickedOverseaLoginBtn)
	ON_BN_CLICKED(IDC_BOPEN_BTN, &CpixiuDlg::OnBnClickedBopenBtn)
	ON_BN_CLICKED(IDC_SOPEN_BTN, &CpixiuDlg::OnBnClickedSopenBtn)
	ON_BN_CLICKED(IDC_BOFFSET_BTN, &CpixiuDlg::OnBnClickedBoffsetBtn)
	ON_BN_CLICKED(IDC_SOFFSET_BTN, &CpixiuDlg::OnBnClickedSoffsetBtn)
	ON_BN_CLICKED(IDC_SHFE_RADIO, &CpixiuDlg::OnBnClickedShfeRadio)
	ON_BN_CLICKED(IDC_DCE_RADIO, &CpixiuDlg::OnBnClickedDceRadio)
	ON_BN_CLICKED(IDC_CZCE_RADIO, &CpixiuDlg::OnBnClickedCzceRadio)
	ON_BN_CLICKED(IDC_CFFEX_RADIO, &CpixiuDlg::OnBnClickedCffexRadio)
	ON_BN_CLICKED(IDC_SGE_RADIO, &CpixiuDlg::OnBnClickedSgeRadio)
	ON_BN_CLICKED(IDC_HKEX_RADIO, &CpixiuDlg::OnBnClickedHkexRadio)
	ON_NOTIFY(NM_DBLCLK, IDC_DEALING_LIST, &CpixiuDlg::OnNMDblclkDealingList)
	ON_NOTIFY(NM_DBLCLK, IDC_POSITION_LIST, &CpixiuDlg::OnNMDblclkPositionList)
	ON_NOTIFY(UDN_DELTAPOS, IDC_PRICE_SPIN, &CpixiuDlg::OnDeltaposPriceSpin)
	ON_NOTIFY(UDN_DELTAPOS, IDC_BUY_SPIN, &CpixiuDlg::OnDeltaposBuySpin)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SELL_SPIN, &CpixiuDlg::OnDeltaposSellSpin)
	ON_CBN_SELCHANGE(IDC_PLATFORM_COMBO, &CpixiuDlg::OnCbnSelchangePlatformCombo)
	ON_CBN_SELCHANGE(IDC_INSTRUMENT_COMBO, &CpixiuDlg::OnCbnSelchangeInstrumentCombo)
	ON_BN_CLICKED(IDC_QRY_POSITION_BTN, &CpixiuDlg::OnBnClickedQryPositionBtn)
END_MESSAGE_MAP()

// CpixiuDlg 消息处理程序
void CpixiuDlg::OnOK() {
}
void CpixiuDlg::OnCancel() {
	CWnd* enableWnds[] = { &m_bOpenBtn, &m_sOpenBtn, &m_bOffsetBtn, &m_sOffsetBtn, &m_buyOffsetSpin, &m_sellOffsetSpin };
	CommonFunction::SHowWindows(enableWnds, CommonFunction::ArraySize(enableWnds), TRUE);
}
BOOL CpixiuDlg::OnInitDialog() {
	CDialogEx::OnInitDialog();
	// 将“关于...”菜单项添加到系统菜单中。
	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr) {
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty()) {
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。 当应用程序主窗口不是对话框时，框架将自动执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标
	{
		GetWindowRect(&m_windowRect);		//获取窗口大小
		m_oldPoint.x = m_windowRect.right - m_windowRect.left;
		m_oldPoint.y = m_windowRect.bottom - m_windowRect.top;
		m_initPoint.x = m_oldPoint.x;
		m_initPoint.y = m_oldPoint.y;
	}	//for size

	InitStrategyPaneSet();
	InitCtrls();
	AddMTCBusiness();

	this->SetTimer(10, 500, NULL);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CpixiuDlg::OnSysCommand(UINT nID, LPARAM lParam) {
	if ((nID & 0xFFF0) == IDM_ABOUTBOX) {
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	} else {
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码来绘制该图标。  
// 对于使用文档/视图模型的 MFC 应用程序，这将由框架自动完成。
void CpixiuDlg::OnPaint() {
	if (IsIconic()) {
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	} else {
		CDialogEx::OnPaint();
	}
}
// 当用户拖动最小化窗口时系统调用此函数取得光标显示
HCURSOR CpixiuDlg::OnQueryDragIcon() {
	return static_cast<HCURSOR>(m_hIcon);
}
HBRUSH CpixiuDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) {
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  在此更改 DC 的任何特性
	if (CTLCOLOR_STATIC == nCtlColor) {
		//***********************交易状态***********************//
		if (IDC_FUTURE_STATUS==pWnd->GetDlgCtrlID() || IDC_DEFERSPOT_STATUS==pWnd->GetDlgCtrlID() || IDC_OVERSEA_STATUS==pWnd->GetDlgCtrlID()) {
			pDC->SetTextColor(RGB(0, 0, 255));
		}
		//***********************行情链接***********************//
		if (IDC_FUTURE_QUOTE==pWnd->GetDlgCtrlID() || IDC_DEFERSPOT_QUOTE==pWnd->GetDlgCtrlID() || IDC_OVERSEA_QUOTE==pWnd->GetDlgCtrlID()) {
			ChangeCtlColor(pDC, pWnd->GetDlgCtrlID());
		}
		//***********************交易链接***********************//
		if (IDC_FUTURE_TRADE==pWnd->GetDlgCtrlID() || IDC_DEFERSPOT_TRADE==pWnd->GetDlgCtrlID() || IDC_OVERSEA_TRADE==pWnd->GetDlgCtrlID()) {
			ChangeCtlColor(pDC, pWnd->GetDlgCtrlID());
		}
		//***********************行情字体***********************//
		if (IDC_ASK_STATIC==pWnd->GetDlgCtrlID() || IDC_ASK_AMT_STATIC==pWnd->GetDlgCtrlID()) {
			pDC->SetBkColor(RGB(114, 238, 144));
		}
		if (IDC_BID_STATIC==pWnd->GetDlgCtrlID() || IDC_BID_AMT_STATIC==pWnd->GetDlgCtrlID()) {
			pDC->SetBkColor(RGB(255, 106, 106));
		}
	}
	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}
void CpixiuDlg::OnDeltaposPriceSpin(NMHDR* pNMHDR, LRESULT* pResult) {
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);

	if (-1 == m_instrumentCombo.GetItemData(m_instrumentCombo.GetCurSel())) {
		return;
	}
	InstID instID = (InstID)m_instrumentCombo.GetItemData(m_instrumentCombo.GetCurSel());
	money unitPrice = InstrumentFactory::GetInstruments().Get(instID).GetUnitPrice();
	money orderPrice = CommonFunction::WindowText2Double(m_priceEdit);
	if (pNMUpDown->iDelta < 0 && orderPrice > unitPrice) {
		orderPrice -= unitPrice;
	}
	if (pNMUpDown->iDelta > 0) {
		orderPrice += unitPrice;
	}
	CommonFunction::Double2WindowText(m_priceEdit, orderPrice.value(), CommonFunction::GetPricePrecision(unitPrice));
	*pResult = 0;
}
void CpixiuDlg::OnDeltaposBuySpin(NMHDR* pNMHDR, LRESULT* pResult) {
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	*pResult = 0;
	
	if (pNMUpDown->iDelta < 0) {
		if (IDC_HKEX_RADIO == GetCheckedRadioButton(IDC_SHFE_RADIO, IDC_HKEX_RADIO)) {		
			SetDlgItemText(IDC_BOFFSET_BTN, _T("双买"));
		} else {
			SetDlgItemText(IDC_BOFFSET_BTN, _T("买平"));
		}
	}
	if (pNMUpDown->iDelta > 0) {
		if (IDC_HKEX_RADIO == GetCheckedRadioButton(IDC_SHFE_RADIO, IDC_HKEX_RADIO)) {
			SetDlgItemText(IDC_BOFFSET_BTN, _T("双买"));
		} else {
			SetDlgItemText(IDC_BOFFSET_BTN, _T("平今"));
		}
	}
}
void CpixiuDlg::OnDeltaposSellSpin(NMHDR* pNMHDR, LRESULT* pResult) {
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	*pResult = 0;
		
	if (pNMUpDown->iDelta < 0) {
		if (IDC_HKEX_RADIO == GetCheckedRadioButton(IDC_SHFE_RADIO, IDC_HKEX_RADIO)) {
			SetDlgItemText(IDC_SOFFSET_BTN, _T("双卖"));
		} else {
			SetDlgItemText(IDC_SOFFSET_BTN, _T("卖平"));
		}
	}
	if (pNMUpDown->iDelta > 0) {
		if (IDC_HKEX_RADIO == GetCheckedRadioButton(IDC_SHFE_RADIO, IDC_HKEX_RADIO)) {
			SetDlgItemText(IDC_SOFFSET_BTN, _T("双卖"));
		} else {
			SetDlgItemText(IDC_SOFFSET_BTN, _T("平今"));
		}
	}
}

void CpixiuDlg::OnClose() {
	if (IDYES == MessageBox(_T("确定要退出么？系统将暂停所有策略！"), AfxGetApp()->m_pszAppName, MB_YESNO | MB_ICONQUESTION)) {
		if (nullptr != m_neturalDlg) {
			m_neturalDlg->PauseAll();
		}
		if (nullptr != m_grabDlg) {
			m_grabDlg->PauseAll();
		}
		if (nullptr != m_spreadDlg) {
			m_spreadDlg->PauseAll();
		}
		Sleep(500);
		CDialogEx::OnCancel();
	}
}
void CpixiuDlg::OnSize(UINT nType, int cx, int cy) {
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: 在此处添加消息处理程序代码
	if (SIZE_RESTORED == nType || SIZE_MAXIMIZED == nType) {
		POINT newPoint;
		CRect clientRect;
		GetClientRect(&clientRect);
		newPoint.x = clientRect.right - clientRect.left;
		newPoint.y = clientRect.bottom - clientRect.top;

		float fsp[2];	//获取控件与窗口相对位置的比值
		fsp[0] = (float)newPoint.x / m_oldPoint.x;
		fsp[1] = (float)newPoint.y / m_oldPoint.y;

		int ctrlId;
		CRect Rect;
		CPoint OldTLPoint, NewTLPoint; //左上角  
		CPoint OldBRPoint, NewBRPoint; //右下角  
		HWND  hwndChild = ::GetWindow(m_hWnd, GW_CHILD);
		while (hwndChild) {
			ctrlId = ::GetDlgCtrlID(hwndChild);//取得ID
			GetDlgItem(ctrlId)->GetWindowRect(Rect);
			ScreenToClient(Rect);
			OldTLPoint = Rect.TopLeft();
			NewTLPoint.x = long(OldTLPoint.x * fsp[0]);
			NewTLPoint.y = long(OldTLPoint.y * fsp[1]);
			OldBRPoint = Rect.BottomRight();
			NewBRPoint.x = long(OldBRPoint.x * fsp[0]);
			NewBRPoint.y = long(OldBRPoint.y * fsp[1]);
			if (IDC_INSTRUMENT_COMBO == ctrlId) {
				NewBRPoint.y += 150;
			}
			Rect.SetRect(NewTLPoint, NewBRPoint);
			GetDlgItem(ctrlId)->MoveWindow(Rect, TRUE);
			hwndChild = ::GetWindow(hwndChild, GW_HWNDNEXT);
		}
		m_oldPoint = newPoint;
	}
}

void CpixiuDlg::OnTimer(UINT_PTR nIDEvent) {
	Data data;
	UIPublisher& publisher = m_framework.GetUIPublisher();
	while (publisher.Pop(data)) {
		OnNotify(data);
	}

	UpdateQuote();
	CDialogEx::OnTimer(nIDEvent);
}
void CpixiuDlg::OnCbnSelchangePlatformCombo() {
	ConnPlatform futurePlatform = ConnPlatform::CTP;
	for each (ConnConfig config in m_config.ConnConfigs) {
		if (config.Platform != ConnPlatform::SGE) {
			futurePlatform = config.Platform;
		}
	}	// 找到期货对应的Platform，现货只有SGE

	CString platformName;
	m_platformCombo.GetWindowTextW(platformName);
	if (0 == platformName.Compare(_T("现货"))) {
		m_framework.GetUIPublisher().QryFund(ConnPlatform::SGE);		
		GetDlgItem(IDC_SET_FUND_BTN)->EnableWindow(TRUE);		// 只有金交所才能设置可用资金
	}
	else {
// 		m_framework.GetUIPublisher().QryFund(futurePlatform);	// 期货资金自动查询
		GetDlgItem(IDC_SET_FUND_BTN)->EnableWindow(FALSE);
	}
}
void CpixiuDlg::OnCbnSelchangeInstrumentCombo() {
	InstID instID = (InstID)m_instrumentCombo.GetItemData(m_instrumentCombo.GetCurSel());
	Instrument instrument = InstrumentFactory::GetInstruments().Get(instID);
	BOOL isVisible = instrument.GetVariety().IsSpot() ? FALSE : TRUE;

	m_bOffsetBtn.ShowWindow(isVisible);
	m_sOffsetBtn.ShowWindow(isVisible);
	GetDlgItem(IDC_PRICE_EDIT)->SetWindowText(_T("0"));
}

void CpixiuDlg::OnBnClickedSetFundsBtn() {
	FundSettingDlg dlg(ConnPlatform::SGE);	// 只有金交所才能设置可用资金
	if (IDOK == dlg.DoModal()) {
		pos::GlobalPositionStore::Instance().Account().SetBalance(dlg.GetPlatformAvailable(ConnPlatform::SGE));
		auto const& account = pos::GlobalPositionStore::Instance().Account();
		CommonFunction::Double2WindowText(*GetDlgItem(IDC_AVAILABLE_STATIC), account.Available.value());
	}
}
void CpixiuDlg::OnBnClickedFuturesLoginBtn() {
	USES_CONVERSION;
	for each (ConnConfig config in m_config.ConnConfigs) {
		if (config.Platform == ConnPlatform::CTP) {
			Credential& futuresCredential = config.Credential;
			LoginDlg loginDlg(futuresCredential);
			if (IDOK == loginDlg.DoModal()) {		
				config.Credential.Password = CT2A(loginDlg.GetPassword());
				OnBnClickedShfeRadio();
				m_framework.Login(config.Platform, CT2A(loginDlg.GetPassword()));
				this->SetWindowText(A2W(config.Credential.UserID.c_str()));
			}
		}		
	}
}
void CpixiuDlg::OnBnClickedDeferSpotLoginBtn() {
	::AfxMessageBox(_T("现货自动登录!"));
// 	Credential futuresCredential;
// 	LoginDlg loginDlg(futuresCredential);
// 	if (IDOK == loginDlg.DoModal()) {
// 		OnBnClickedSgeRadio();
// 		m_framework.Login(ConnPlatform::SGE, CT2A(loginDlg.GetPassword()));
// 	}
}
void CpixiuDlg::OnBnClickedOverseaLoginBtn() {
	USES_CONVERSION;
	for each (ConnConfig config in m_config.ConnConfigs) {
		if (config.Platform == ConnPlatform::HKEX_ESUNNY) {
			Credential& overseaCredential = config.Credential;
			LoginDlg loginDlg(overseaCredential);
			if (IDOK == loginDlg.DoModal()) {
				config.Credential.Password = CT2A(loginDlg.GetPassword());
				OnBnClickedHkexRadio();
				m_framework.Login(config.Platform, CT2A(loginDlg.GetPassword()));
				this->SetWindowText(A2W(overseaCredential.UserID.c_str()));
			}
		}
	}
}

void CpixiuDlg::OnBnClickedShfeRadio() {
	ShowOpenBtn(TRUE);
	ShowOffsetBtn(TRUE);
	InitComboData(Exchange::SHFE);
	SwitchExchRadio(IDC_SHFE_RADIO);
	m_instrumentCombo.SetFocus();
}
void CpixiuDlg::OnBnClickedDceRadio() {
	ShowOpenBtn(TRUE);
	ShowOffsetBtn(FALSE);
	InitComboData(Exchange::DCE);
	SwitchExchRadio(IDC_DCE_RADIO);
	m_instrumentCombo.SetFocus();
}
void CpixiuDlg::OnBnClickedCzceRadio() {
	ShowOpenBtn(TRUE);
	ShowOffsetBtn(FALSE);
	InitComboData(Exchange::CZCE);
	SwitchExchRadio(IDC_CZCE_RADIO);
	m_instrumentCombo.SetFocus();
}
void CpixiuDlg::OnBnClickedCffexRadio() {
	ShowOpenBtn(TRUE);
	ShowOffsetBtn(FALSE);
	InitComboData(Exchange::CFFEX);
	SwitchExchRadio(IDC_CFFEX_RADIO);
	m_instrumentCombo.SetFocus();
}
void CpixiuDlg::OnBnClickedSgeRadio() {
	ShowOpenBtn(TRUE);
	ShowOffsetBtn(FALSE);
	InitComboData(Exchange::SGE);
	SwitchExchRadio(IDC_SGE_RADIO);
	m_instrumentCombo.SetFocus();
}
void CpixiuDlg::OnBnClickedHkexRadio() {
	ShowOpenBtn(FALSE);
	ShowOffsetBtn(FALSE, true);
	InitComboData(Exchange::HKEX);
	SwitchExchRadio(IDC_HKEX_RADIO);
	m_instrumentCombo.SetFocus();
}

void CpixiuDlg::OnBnClickedBopenBtn() {
	if (ParameterValidate()) {
		mct::MCTOrder buyOpenOrder{
			(InstID)m_instrumentCombo.GetItemData(m_instrumentCombo.GetCurSel()), BuyOpenNormal, 
			CommonFunction::WindowText2UINT16(m_amtEdit), CommonFunction::WindowText2Double(m_priceEdit)
		};

		mct::MCTCommand orderCmd(mct::MCTCommand::CommandType::Order, buyOpenOrder);
		StrategyCommand strategyCmd;
		strategyCmd.Type = StrategyCommand::MANUAL_COMMAND;
		memcpy(strategyCmd.Data, &orderCmd, sizeof(mct::MCTCommand));
		strategyCmd.Business = &*m_businessPtr;
		m_framework.GetUIPublisher().Push(strategyCmd);
	}
}
void CpixiuDlg::OnBnClickedSopenBtn() {
	if (ParameterValidate()) {
		mct::MCTOrder sellOpenOrder{
			(InstID)m_instrumentCombo.GetItemData(m_instrumentCombo.GetCurSel()), SellOpenNormal,
			CommonFunction::WindowText2UINT16(m_amtEdit), CommonFunction::WindowText2Double(m_priceEdit)
		};

		mct::MCTCommand orderCmd(mct::MCTCommand::CommandType::Order, sellOpenOrder);
		StrategyCommand strategyCmd;
		strategyCmd.Type = StrategyCommand::MANUAL_COMMAND;
		memcpy(strategyCmd.Data, &orderCmd, sizeof(mct::MCTCommand));
		strategyCmd.Business = &*m_businessPtr;
		m_framework.GetUIPublisher().Push(strategyCmd);
	}
}
void CpixiuDlg::OnBnClickedBoffsetBtn() {
	if (ParameterValidate()) {		
		CString btnTxt;
		GetDlgItemText(IDC_BOFFSET_BTN, btnTxt);
		OrderCommand buyOffsetCmd = btnTxt.Compare(_T("平今")) ? BuyOffsetNormal : BuyOffsetTodayNormal;
		mct::MCTOrder buyOrder{
			(InstID)m_instrumentCombo.GetItemData(m_instrumentCombo.GetCurSel()), buyOffsetCmd,
			CommonFunction::WindowText2UINT16(m_amtEdit), CommonFunction::WindowText2Double(m_priceEdit)
		};

		mct::MCTCommand orderCmd(mct::MCTCommand::CommandType::Order, buyOrder);
		StrategyCommand strategyCmd;
		strategyCmd.Type = StrategyCommand::MANUAL_COMMAND;
		memcpy(strategyCmd.Data, &orderCmd, sizeof(mct::MCTCommand));
		strategyCmd.Business = &*m_businessPtr;
		m_framework.GetUIPublisher().Push(strategyCmd);
	}
}
void CpixiuDlg::OnBnClickedSoffsetBtn() {
	if (ParameterValidate()) {
		CString btnTxt;
		GetDlgItemText(IDC_SOFFSET_BTN, btnTxt);
		OrderCommand sellOffsetCmd = btnTxt.Compare(_T("平今")) ? SellOffsetNormal : SellOffsetTodayNormal;
		mct::MCTOrder sellOrder{
			(InstID)m_instrumentCombo.GetItemData(m_instrumentCombo.GetCurSel()), sellOffsetCmd,
			CommonFunction::WindowText2UINT16(m_amtEdit), CommonFunction::WindowText2Double(m_priceEdit)
		};

		mct::MCTCommand orderCmd(mct::MCTCommand::CommandType::Order, sellOrder);
		StrategyCommand strategyCmd;
		strategyCmd.Type = StrategyCommand::MANUAL_COMMAND;
		memcpy(strategyCmd.Data, &orderCmd, sizeof(mct::MCTCommand));
		strategyCmd.Business = &*m_businessPtr;
		m_framework.GetUIPublisher().Push(strategyCmd);
	}
}

void CpixiuDlg::OnBnClickedQryPositionBtn() {
	m_positionList.DeleteAllItems();
	for (auto platform : m_loginedPlatform) {
		m_framework.GetUIPublisher().QryPosition(platform);
	}	// 对于已经登录的平台都发起查询
}

void CpixiuDlg::OnNMDblclkDealingList(NMHDR* pNMHDR, LRESULT* pResult) {
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	int nItem = pNMItemActivate->iItem;
	if (nItem >= 0 && nItem < m_dealingList.GetItemCount()) {
		USES_CONVERSION;
		CString systemID = m_dealingList.GetItemText(nItem, 0);
		CString instCode = m_dealingList.GetItemText(nItem, 1);
		CString price = m_dealingList.GetItemText(nItem, 4);
		CString localID = m_dealingList.GetItemText(nItem, 9);

		SystemNoT systemNo(W2A(systemID));
		InstID instID = GetInstID(instCode);
		money orderPrice = atof(W2A(price));
		LocalOrderID localId = atoi(W2A(localID));

		Instrument instrument = InstrumentFactory::GetInstruments().Get(instID);
		TSGEProductClassType productClass = instrument.GetVariety().IsSpot() ? SGE_OC_Spot : SGE_OC_Defer;
		if (orderPrice < 1) {
			productClass = (m_currentStatus > ExchangeStatus::DELIVERY_END) ? SGE_OC_Neutral : SGE_OC_Delivery;
		}	// 委托价为0必定是：中立仓或交割申报

		OrderCancel actionCommand(instrument.GetExchange(), systemNo, instrument.GetInstCode(), instID, localId, LocalIdACC::NextNormal(), productClass);
		mct::MCTCancel mctCancel{ actionCommand };
		mct::MCTCommand cancelCmd(mct::MCTCommand::CommandType::Cancel, mctCancel);
	
		StrategyCommand strategyCmd;
		strategyCmd.Type = StrategyCommand::MANUAL_COMMAND;
		memcpy(strategyCmd.Data, &cancelCmd, sizeof(mct::MCTCommand));
		strategyCmd.Business = &*m_businessPtr;
		m_framework.GetUIPublisher().Push(strategyCmd);
		return;
	}
	*pResult = 0;
}
void CpixiuDlg::OnNMDblclkPositionList(NMHDR* pNMHDR, LRESULT* pResult) {
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	int nItem = pNMItemActivate->iItem;
	if (nItem >= 0 && nItem < m_positionList.GetItemCount()) {
		USES_CONVERSION;
		CString instCode = m_positionList.GetItemText(nItem, 0);
		CString direction = m_positionList.GetItemText(nItem, 1);
		CString totalQuantity = m_positionList.GetItemText(nItem, 2);
		CString todayQuantity = m_positionList.GetItemText(nItem, 3);

		int totalQty = atoi(W2A(totalQuantity));		// 总持仓
		int todayQty = atoi(W2A(todayQuantity));		// 净持仓
		int offsetAmt = (0 == todayQty) ? totalQty : todayQty;		//优先平今仓
		GetDlgItem(IDC_AMT_EDIT)->SetWindowText(A2W(fmt::format("{}", offsetAmt).c_str()));
		
		//选中对应Radio
		CButton* radioBtns[] = { (CButton*)GetDlgItem(IDC_SHFE_RADIO),(CButton*)GetDlgItem(IDC_CFFEX_RADIO),(CButton*)GetDlgItem(IDC_CZCE_RADIO),(CButton*)GetDlgItem(IDC_DCE_RADIO),(CButton*)GetDlgItem(IDC_SGE_RADIO),(CButton*)GetDlgItem(IDC_HKEX_RADIO) };
		for each (CButton * checkBtn in radioBtns) {
			checkBtn->SetCheck(FALSE);
		}

		int instID = GetInstID(instCode);
		Instrument instrument = InstrumentFactory::GetInstruments().Get(instID);
		switch (instrument.GetExchange()) {
		case Exchange::SHFE: {
			((CButton*)GetDlgItem(IDC_SHFE_RADIO))->SetCheck(TRUE);
			OnBnClickedShfeRadio();
			if (offsetAmt == todayQty) {
				UINT nID = (0 == direction.Compare(_T("买"))) ? IDC_SOFFSET_BTN : IDC_BOFFSET_BTN;
				SetDlgItemText(nID, _T("平今"));
			}
			break;
		}
		case Exchange::DCE: {
			((CButton*)GetDlgItem(IDC_SHFE_RADIO))->SetCheck(TRUE);
			OnBnClickedDceRadio();
			break;
		}
		case Exchange::CZCE: {
			((CButton*)GetDlgItem(IDC_SHFE_RADIO))->SetCheck(TRUE);
			OnBnClickedCzceRadio();
			break;
		}
		case Exchange::CFFEX: {
			((CButton*)GetDlgItem(IDC_SHFE_RADIO))->SetCheck(TRUE);
			OnBnClickedCffexRadio();
			break;
		}
		case Exchange::SGE: {
			((CButton*)GetDlgItem(IDC_SHFE_RADIO))->SetCheck(TRUE);
			OnBnClickedSgeRadio();
			break;
		}
		case Exchange::HKEX: {
			((CButton*)GetDlgItem(IDC_SHFE_RADIO))->SetCheck(TRUE);
			OnBnClickedHkexRadio();
			break;	
		}
		}

		// 选中对应Combox
		for (int i = 0; i < m_instrumentCombo.GetCount(); ++i) {
			CString comboText;
			m_instrumentCombo.GetLBText(i, comboText);
			if (0 == comboText.Compare(instCode)) {
				m_instrumentCombo.SetCurSel(i);
				break;
			}
		}

		// 为了防止下错单，再把其他按钮隐藏
		auto* pHideWind = (0 == direction.Compare(_T("买"))) ? &m_bOffsetBtn : &m_sOffsetBtn;
		auto* pHideSpin = (0 == direction.Compare(_T("买"))) ? &m_buyOffsetSpin : &m_sellOffsetSpin;
		CWnd* enableWnds[] = { &m_bOpenBtn, &m_sOpenBtn, pHideWind, pHideSpin };
		CommonFunction::SHowWindows(enableWnds, CommonFunction::ArraySize(enableWnds), FALSE);

		QuoteService& quoteService = WritableQuoteService();
		money offsetPrice = (0 == direction.Compare(_T("买"))) ? quoteService.GetSellPrice(instID) : quoteService.GetBuyPrice(instID);
		int precision = CommonFunction::GetPricePrecision(instrument.GetUnitPrice());
		CommonFunction::Double2WindowText(*GetDlgItem(IDC_PRICE_EDIT), offsetPrice.value(), precision);
		GetDlgItem(IDC_PRICE_EDIT)->SetFocus(); 
		return;
	}
	*pResult = 0;
}

void CpixiuDlg::InitStrategyPaneSet() {
	CDialog* pDialog = NULL;

	for (auto panel : m_config.Rule.ValidPanel) {
		if (0 == panel.compare("SPREAD")) {
			m_spreadDlg = new HFSpreadDlg();
			pDialog = m_spreadDlg;
			m_dialogTabCtrl.AddTabPane(_T("HFSpread"), IDD_HF_SPREAD_DLG, pDialog);
		}
		if (0 == panel.compare("GRAB")) {
			m_grabDlg = new SuperGrabDlg();
			pDialog = m_grabDlg;
			m_dialogTabCtrl.AddTabPane(_T("SuperGrab"), IDD_TG_GRAB_DLG, pDialog);
		}
		if (0 == panel.compare("HFT")) {
			m_neturalDlg = new HFNeturalDlg();
			pDialog = m_neturalDlg;
			m_dialogTabCtrl.AddTabPane(_T("HFNetural"), IDD_HF_NETURAL_DLG, pDialog);
		}
	}
		
	m_dialogTabCtrl.SetDefaultPane(0);
}
void CpixiuDlg::InitCtrls() {
	m_platformCombo.SetCurSel(0);
	m_priceSpin.SetBuddy(GetDlgItem(IDC_PRICE_EDIT));
	m_priceSpin.SetRange(-10, 10);

	for each (ConnConfig const& connConfig  in m_config.ConnConfigs) {
		UINT nID = m_platformIDMap[connConfig.Platform];
		if (IDC_FUTURES_LOGIN_BTN == nID) {			
			int index = m_platformCombo.AddString(_T("期货"));
			m_platformCombo.SetItemData(index, nID);
		}
		else if (IDC_DEFERSPOT_LOGIN_BTN == nID) {
			int index = m_platformCombo.AddString(_T("现货"));
			m_platformCombo.SetItemData(index, nID);
		}
		else if (IDC_OVERSEA_LOGIN_BTN == nID) {
			int index = m_platformCombo.AddString(_T("海外"));
			m_platformCombo.SetItemData(index, nID);
		}
	}
	m_platformCombo.SetCurSel(0);

	InitDealingList();
	InitPositionList();

	m_logList.InsertColumn(0, _T("日期"), LVCFMT_RIGHT, 100);
	m_logList.InsertColumn(1, _T("信息"), LVCFMT_LEFT, 1650);
	m_logList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP);
}
void CpixiuDlg::InitDealingList() {
	int column = 0;
	m_dealingList.InsertColumn(column++, _T("系统号"), LVCFMT_CENTER, 120);	// 230
	m_dealingList.InsertColumn(column++, _T("合约代码"), LVCFMT_CENTER, 100);
	m_dealingList.InsertColumn(column++, _T("买卖"), LVCFMT_CENTER, 60);
	m_dealingList.InsertColumn(column++, _T("开平"), LVCFMT_CENTER, 60);
	m_dealingList.InsertColumn(column++, _T("委托价"), LVCFMT_RIGHT, 120);
	m_dealingList.InsertColumn(column++, _T("委托"), LVCFMT_CENTER, 60);
	m_dealingList.InsertColumn(column++, _T("成交"), LVCFMT_CENTER, 60);
	m_dealingList.InsertColumn(column++, _T("成交价"), LVCFMT_LEFT, 120);
	m_dealingList.InsertColumn(column++, _T("剩余"), LVCFMT_CENTER, 60);
	m_dealingList.InsertColumn(column++, _T("委托号"), LVCFMT_RIGHT, 120);	// 100
	m_dealingList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP);
// 	m_dealingList.InsertItem(0, _T("OA2019071200000000110"));
}
void CpixiuDlg::InitPositionList() {
	int column = 0;
	m_positionList.InsertColumn(column++, _T("合约代码"), LVCFMT_RIGHT, 100);
	m_positionList.InsertColumn(column++, _T("买卖"), LVCFMT_CENTER, 60);
	m_positionList.InsertColumn(column++, _T("总仓"), LVCFMT_RIGHT, 65);
	m_positionList.InsertColumn(column++, _T("今仓"), LVCFMT_RIGHT, 65);		// 需用平仓指令的持仓
	m_positionList.InsertColumn(column++, _T("冻结"), LVCFMT_RIGHT, 65);
	m_positionList.InsertColumn(column++, _T("均价"), LVCFMT_RIGHT, 80);
	m_positionList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP | LVS_NOCOLUMNHEADER);
}
void CpixiuDlg::InitComboData(Exchange exchange) {
	m_instrumentCombo.ResetContent();
	Instruments const& instruments = InstrumentFactory::GetInstruments();
	for each (Instrument const& inst in instruments.GetAll()) {
		if (inst.GetExchange() == exchange) {
			if (0 == inst.GetVariety().GetCode().compare("CUS"))	// 不支持CUS人工交易
				continue;
			CString contractCode(inst.GetInstCode().Str());
			int index = m_instrumentCombo.AddString(contractCode);
			m_instrumentCombo.SetItemData(index, inst.GetID());
		}
	}
	m_instrumentCombo.SetCurSel(0);
}
void CpixiuDlg::AddMTCBusiness() {
	m_businessPtr = StrategyFactory::NewMCT(this,m_framework.GetStrategyPublisher());
	m_framework.GetUIPublisher().AddBusiness(&*m_businessPtr);
}

void CpixiuDlg::OnNotify(Data const& data) {
	switch (data.Type) {
	case DataType::UIMessage: {
		CString msg((LPSTR)data.Data);	//or use CA2T function
		OnInfoNotification(msg);
	}
	case DataType::Exchange: {
		ExchangeNotification notify = *(ExchangeNotification*)data.Data;
		OnExchangeNotification(notify);
		break;
	}
	case DataType::QryPosition: {
		PositionNotification notify = *(PositionNotification*)data.Data;
		OnPositionNotification(notify);
		break;
	}
	case DataType::UpdateAvailable: {
		PlatformAccount notify = *(PlatformAccount*)data.Data;
		OnAccountUpdate(notify);
		break;
	}
	case DataType::Trade: {
		OrderNotification tradeNotification = *(OrderNotification*)data.Data;
		OnTradeDetail(tradeNotification);
		break;
	}
	case DataType::Dealing: {
		Dealing dealing = *(Dealing*)data.Data;
		OnDealing(dealing);
		break;
	}
	case DataType::BusinessUpdate: {
		SnapshotBase const& update = *(SnapshotBase*)data.Data;
		if (update.UIUpdaterPtr != nullptr) {
			update.UIUpdaterPtr->Update(data);
		}
		break;
	}
	default:
		throw ILLEGAL_MESSAGE_TYPE;
	}
}

char const* GetOperationDescription(Operation type) {
	switch (type) {
	case Operation::OPEN:	return "开仓";
	case Operation::OFFSET:	return "平仓";
	case Operation::OFFSET_TODAY:	return "平今";
	default:	return "-";
	}
}
void CpixiuDlg::OnInfoNotification(CString const& info) {
	CTime logTime(CTime::GetCurrentTime());
	m_logList.InsertItem(0, logTime.Format("%H:%M:%S"));
	m_logList.SetItemText(0, 1, info);

	UINT itemCount = m_logList.GetItemCount();
	if (itemCount > 0) {
		m_logList.EnsureVisible(0, FALSE);
	}
	if (itemCount > 6) {
		m_logList.DeleteItem(itemCount - 1);
	}
}
void CpixiuDlg::OnExchangeNotification(ExchangeNotification const& notification) {
	UINT quoteID = IDC_FUTURE_QUOTE;
	UINT tradeID = IDC_FUTURE_TRADE;
	UINT statusID = IDC_FUTURE_STATUS;
	switch (notification.Platform) {
	case ConnPlatform::CTP:
	case ConnPlatform::HFT:
		break;
	case ConnPlatform::SGE:
		quoteID = IDC_DEFERSPOT_QUOTE;
		tradeID = IDC_DEFERSPOT_TRADE;
		statusID = IDC_DEFERSPOT_STATUS;
		break;
	default:
		quoteID = IDC_OVERSEA_QUOTE;
		tradeID = IDC_OVERSEA_TRADE;
		statusID = IDC_OVERSEA_STATUS;
	}
	NotifyByPlatform(quoteID, tradeID, statusID, notification);
}
void CpixiuDlg::NotifyByPlatform(UINT quoteID, UINT tradeID, UINT statusID, ExchangeNotification const& notification) {	
	switch (notification.Type) {
	case ExchangeNotification::QUOTE_CONNECTED: {
		GetDlgItem(quoteID)->SetWindowText(_T("已连接"));
		break;
	}
	case ExchangeNotification::QUOTE_LOGINED: {
		GetDlgItem(quoteID)->SetWindowText(_T("已登录"));
		break;
	}
	case ExchangeNotification::TRADER_CONNECTED: {
		GetDlgItem(tradeID)->SetWindowText(_T("已连接"));
		if (ConnPlatform::CTP == notification.Platform || ConnPlatform::HFT == notification.Platform) {
			GetDlgItem(IDC_FUTURES_LOGIN_BTN)->EnableWindow(TRUE);
		}
		else if (ConnPlatform::SGE == notification.Platform) {
			GetDlgItem(IDC_DEFERSPOT_LOGIN_BTN)->EnableWindow(TRUE);
		}
		else if (ConnPlatform::HKEX_ESUNNY == notification.Platform) {
			GetDlgItem(IDC_OVERSEA_LOGIN_BTN)->EnableWindow(TRUE);
		}
		break;
	}
	case ExchangeNotification::TRADER_LOGINED: {
		GetDlgItem(tradeID)->SetWindowText(_T("已登录"));
		m_loginedPlatform.insert(notification.Platform);
		if (ConnPlatform::SGE == notification.Platform) {
			GetDlgItem(IDC_SGE_RADIO)->EnableWindow(TRUE);
			m_platformCombo.SetCurSel(1);	// 现货
			OnCbnSelchangePlatformCombo();
		}
		else if (ConnPlatform::HFT == notification.Platform) {
			GetDlgItem(IDC_SHFE_RADIO)->EnableWindow(TRUE);			// 中信上期所专用版本：120550255	autowin_plus_1.0	156eb98f3268601bd4773372203b7e1c
// 			GetDlgItem(IDC_CFFEX_RADIO)->EnableWindow(TRUE);		// 东证中金所专用版本：88937011	client_autowin_1.3.3.24		SE5C6H1P1WYPMVPE
			m_platformCombo.SetCurSel(0);	// 期货
		}	// 易达只支能持上海或中金
		else if (ConnPlatform::CTP == notification.Platform) {
			GetDlgItem(IDC_CFFEX_RADIO)->EnableWindow(TRUE);
			GetDlgItem(IDC_SHFE_RADIO)->EnableWindow(TRUE);
			GetDlgItem(IDC_DCE_RADIO)->EnableWindow(TRUE);
			GetDlgItem(IDC_CZCE_RADIO)->EnableWindow(TRUE);
			m_platformCombo.SetCurSel(0);
		}	// CTP支持所有期货品种
		else if (ConnPlatform::HKEX_ESUNNY == notification.Platform) {
			GetDlgItem(IDC_HKEX_RADIO)->EnableWindow(TRUE);
		}
		break;
	}
	case ExchangeNotification::QUOTE_DISCONNECTED: {
		GetDlgItem(quoteID)->SetWindowText(_T("已断开"));
		break;
	}
	case ExchangeNotification::TRADER_DISCONNECTED: {
		GetDlgItem(tradeID)->SetWindowText(_T("已断开"));
		if (ConnPlatform::SGE == notification.Platform) {
			GetDlgItem(IDC_SGE_RADIO)->EnableWindow(FALSE);
		}
		else if (ConnPlatform::HFT == notification.Platform) {
			GetDlgItem(IDC_SHFE_RADIO)->EnableWindow(FALSE);
			// 			GetDlgItem(IDC_CFFEX_RADIO)->EnableWindow(TRUE);
		}	// 易达只支持上海
		else if (ConnPlatform::CTP == notification.Platform) {
			GetDlgItem(IDC_CFFEX_RADIO)->EnableWindow(FALSE);
			GetDlgItem(IDC_SHFE_RADIO)->EnableWindow(FALSE);
			GetDlgItem(IDC_DCE_RADIO)->EnableWindow(FALSE);
			GetDlgItem(IDC_CZCE_RADIO)->EnableWindow(FALSE);
		}	// CTP支持所有期货品种
		else if (ConnPlatform::HKEX_ESUNNY == notification.Platform) {
			GetDlgItem(IDC_HKEX_RADIO)->EnableWindow(FALSE);
		}
		break;
	}
	case ExchangeNotification::CODE_LOGIN: {
		for each (ConnConfig config in m_config.ConnConfigs) {
			if (config.Platform == ConnPlatform::HKEX_ESUNNY) {
				Credential& overseaCredential = config.Credential;
				LoginDlg loginDlg(overseaCredential);
				if (IDOK == loginDlg.DoModal()) {
					config.Credential.Password = CT2A(loginDlg.GetPassword());
					OnBnClickedHkexRadio();
					m_framework.SetVertificateCode(config.Platform, CT2A(loginDlg.GetPassword()));
				}
			}
			else {
// 				GetDlgItem(IDC_SET_FUND_BTN)->EnableWindow(TRUE);
			}
		}
		break;
	}
	case ExchangeNotification::EXCHANGE_STATUS: {
		m_currentStatus = notification.Status;
		CString currentStatus;
		GetDlgItem(statusID)->GetWindowText(currentStatus);
		CString statusDesc(m_statusInfoMap[m_currentStatus].c_str());
		if (statusDesc != currentStatus) {
			GetDlgItem(statusID)->SetWindowText(statusDesc);
		}
		break;
	}
	default:
		break;
	}
}
void CpixiuDlg::OnAccountUpdate(PlatformAccount const& notification) {
	CString platformDesc = _T("期货");
	if (notification.Platform == ConnPlatform::SGE) {
		platformDesc = _T("现货");
	} 
	
	auto const& account = pos::GlobalPositionStore::Instance().Account();
	UINT nID = UINT(m_platformCombo.GetItemData(m_platformCombo.GetCurSel()));
	CommonFunction::Double2WindowText(*GetDlgItem(IDC_CAPITAL_STATIC), account.Balance.value());
	CommonFunction::Double2WindowText(*GetDlgItem(IDC_OCCUPY_STATIC), account.Margin.value());
	CommonFunction::Double2WindowText(*GetDlgItem(IDC_FEE_STATIC), account.Fee.value());
	CommonFunction::Double2WindowText(*GetDlgItem(IDC_FREEZE_STATIC), (account.FrozenFee + account.FrozenMargin).value());
	CommonFunction::Double2WindowText(*GetDlgItem(IDC_AVAILABLE_STATIC), account.Available.value());
}
void CpixiuDlg::OnPositionNotification(PositionNotification const& notification) {	
	m_framework.GetUIPublisher().SubscribeQuote(notification.InstID);	//有持仓的合约，直接订阅行情
	UpdatePositionList(notification);
}
void CpixiuDlg::OnDealing(Dealing const& dealing) {
	CString systemNo(dealing.GetSystemNo().Str());
	if (systemNo.IsEmpty()) {
		return;
	} 
	else {
		systemNo.TrimLeft();
	}
	
	Instrument instrument = InstrumentFactory::GetInstruments().Get(dealing.GetInstID());
	char const* priceFormat = CommonFunction::GetPriceFormat(instrument.GetUnitPrice());

// 	Log::Info(fmt::format("UIOnDealing, SysID[{}] LocalId[{}] [{} {}: {:.3f} {}] Status[{}]"
// 		, dealing.GetSystemNo().Str(), dealing.GetLocalId(), Desc(dealing.GetDirection()), Desc(dealing.GetOperation())
// 		, dealing.GetOrderPrice(), dealing.GetOrderQuantity(), dealing.GetStatus()));

	int pos = 0;
	if (dealing.IsOrdered() || dealing.IsPartial()) {
		USES_CONVERSION;
		int remainQuantity = dealing.GetRemainQuantity();
		if (!FindDealingFromList(systemNo, pos)) {
			m_dealingList.InsertItem(pos, systemNo);
			UINT columnPos = 1;
			m_dealingList.SetItemText(pos, columnPos++, A2W(instrument.GetInstCode().Str()));
			std::string direction = dealing.GetDirection()== Direction::BUY ? "买" : "卖";
			m_dealingList.SetItemText(pos, columnPos++, A2W(direction.c_str()));			
			m_dealingList.SetItemText(pos, columnPos++, A2W(GetOperationDescription(dealing.GetOperation())));
			m_dealingList.SetItemText(pos, columnPos++, A2W(fmt::format(priceFormat, dealing.GetOrderPrice().value()).c_str()));// 委托价			
			m_dealingList.SetItemText(pos, columnPos++, A2W(fmt::format("{}", dealing.GetOrderQuantity()).c_str()));			// 委托量
			m_dealingList.SetItemText(pos, columnPos++, A2W(fmt::format("{}", dealing.GetQuantityTraded()).c_str()));			// 成交量
			m_dealingList.SetItemText(pos, columnPos++, A2W(fmt::format(priceFormat, dealing.GetDealPrice().value()).c_str()));	// 成交价
			m_dealingList.SetItemText(pos, columnPos++, A2W(fmt::format("{}", dealing.GetRemainQuantity()).c_str()));			// 剩余量
			m_dealingList.SetItemText(pos, columnPos++, A2W(fmt::format("{}", dealing.GetLocalId()).c_str()));					// 本地委托号
		} 
		else {
			UINT columnPos = 6;
			// SGE取GetDealQuantity，其他取GetQuantityTraded
			int dealQuantity = IsSGE(dealing.GetExchange()) ? dealing.GetDealQuantity() : dealing.GetQuantityTraded();
			m_dealingList.SetItemText(pos, columnPos++, A2W(fmt::format("{}", dealQuantity).c_str()));							// 成交量
			m_dealingList.SetItemText(pos, columnPos++, A2W(fmt::format(priceFormat, dealing.GetDealPrice().value()).c_str()));	// 成交价
			m_dealingList.SetItemText(pos, columnPos++, A2W(fmt::format("{}", dealing.GetRemainQuantity()).c_str()));			// 剩余量
		}
	} 

	if (dealing.IsFinished() || dealing.GetRemainQuantity() <= 0) {
		if (FindDealingFromList(systemNo, pos)) {
			m_dealingList.DeleteItem(pos);
		}
	}
}
bool CpixiuDlg::FindDealingFromList(CString const& systemNo, int& pos) {
	int itemCount = m_dealingList.GetItemCount();
	for (int currentPos = 0; currentPos < itemCount; ++currentPos) {
		CString systemID = m_dealingList.GetItemText(currentPos, 0);
		if (0 == systemID.Compare(systemNo)) {
			pos = currentPos;
			return true;
		}
	}
	return false;
}
bool CpixiuDlg::FindPositionFromList(InstID instID, Direction direction, int& pos) {
	USES_CONVERSION;
	int itemCount = m_positionList.GetItemCount();
	Exchange exchange = InstrumentFactory::GetInstruments().Get(instID).GetExchange();
	for (int currentPos=0; currentPos<itemCount; ++currentPos) {
		CString instCode = m_positionList.GetItemText(currentPos, 0);		
		if (0 == instCode.Compare(A2W(InstrumentFactory::GetInstruments().Get(instID).GetInstCode().Str()))) {
			if (Exchange::HKEX == exchange) {
				pos = currentPos;
				return true;
			} 
			else {
				CString dirText = m_positionList.GetItemText(currentPos, 1);
				Direction dir = (0 == dirText.Compare(_T("买"))) ? Direction::BUY : Direction::SELL;
				if (direction == dir) {
					pos = currentPos;
					return true;
				}
			}
		}
	}
	return false;
}
void CpixiuDlg::OnTradeDetail(OrderNotification const& notification) {
// 	Log::Info(fmt::format("UIOnTradeDetail, SysID[{}] LocalId[{}] Inst[{}] [{} {}: {:.3f} {}] Turnover[{:.3f}]"
// 		, notification.SystemNo, notification.LocalId, notification.InstID, Desc(notification.Direction), Desc(notification.Operation)
// 		, notification.DealPrice, notification.DealQuantity, notification.DealPrice * notification.DealQuantity));

	int pos = 0;
	InstID instID = notification.InstID;
	Instrument instrument = InstrumentFactory::GetInstruments().Get(instID);

	Direction direction = notification.Direction;
	if (Operation::OPEN != notification.Operation) {
		direction = OppositeDirection(direction);
	}	// 平仓操作 买卖方向相反

	USES_CONVERSION;
	if (!FindPositionFromList(instID, direction, pos)) {
		char const* dirDescribe = (direction == Direction::BUY) ? "买" : "卖";
		m_positionList.InsertItem(pos, A2W(instrument.GetInstCode().Str()));
		m_positionList.SetItemText(pos, 1, A2W(dirDescribe));
		m_positionList.SetItemText(pos, 2, A2W(fmt::format("{}", notification.DealQuantity).c_str()));
		int todayDealQuantity = (IsSHFE(instID) || IsINE(instID)) ? notification.DealQuantity : 0;
		m_positionList.SetItemText(pos, 3, A2W(fmt::format("{}", todayDealQuantity).c_str()));	// 上期所合约还要增加今仓标注
		m_positionList.SetItemText(pos, 4, A2W(fmt::format("{}", 0).c_str()));
		m_positionList.SetItemText(pos, 5, A2W(fmt::format("{:.3f}", notification.DealPrice).c_str()));
		return;
	}
	else {
		CString totalQuantity = m_positionList.GetItemText(pos, 2);							// 总仓
		CString averagePrice = m_positionList.GetItemText(pos, 5);							// 均价
		int totalQty = atoi(W2A(totalQuantity));
		money avgPrice = atof(W2A(averagePrice));
		money turnOver = totalQty * avgPrice;
		money oriTurnover = turnOver;

		if (Operation::OPEN == notification.Operation) {
			totalQty += notification.DealQuantity;
			m_positionList.SetItemText(pos, 2, A2W(fmt::format("{}", totalQty).c_str()));
			if (IsSHFE(instID) || IsINE(instID)) {
				CString todayQuantity = m_positionList.GetItemText(pos, 3);					// 今仓
				int todayQty = atoi(W2A(todayQuantity));
				m_positionList.SetItemText(pos, 3, A2W(fmt::format("{}", todayQty + notification.DealQuantity).c_str()));
			}	// 上期所合约还要修改今仓标注

// 			turnOver += notification.DealQuantity * notification.DealPrice;
			avgPrice = turnOver / totalQty;
// 			Log::Info(fmt::format("Turnover[{:.3f}]->[{:.3f}], avg[{:.3f}]", oriTurnover, turnOver, avgPrice));
			m_positionList.SetItemText(pos, 5, A2W(fmt::format("{:.3f}", avgPrice).c_str()));
		}	// 开仓
		else {
			totalQty -= notification.DealQuantity;
			if (totalQty <= 0) {
				m_positionList.DeleteItem(pos);
				return;
			}	// 无剩余持仓删除记录

			m_positionList.SetItemText(pos, 2, A2W(fmt::format("{}", totalQty).c_str()));	// 总仓
			if (Operation::OFFSET_TODAY == notification.Operation) {
				CString todayQuantity = m_positionList.GetItemText(pos, 3);					// 今仓
				int todayQty = atoi(W2A(todayQuantity));
				m_positionList.SetItemText(pos, 3, A2W(fmt::format("{}", todayQty - notification.DealQuantity).c_str()));
			}	// 上期所合约还要修改今仓标注

			CString frozenQuantity = m_positionList.GetItemText(pos, 4);	// 冻结
			int frozenQty = atoi(W2A(frozenQuantity));
			if (frozenQty > 0) {
				m_positionList.SetItemText(pos, 4, A2W(fmt::format("{}", frozenQty - notification.DealQuantity).c_str()));
			}

// 			turnOver -= notification.DealQuantity * notification.DealPrice;
			avgPrice = turnOver / totalQty;
// 			Log::Info(fmt::format("Turnover[{:.3f}]->[{:.3f}], avg[{:.3f}]", oriTurnover, turnOver, avgPrice));
			m_positionList.SetItemText(pos, 5, A2W(fmt::format("{:.3f}", avgPrice).c_str()));
		}	// 平仓
	}
}
void CpixiuDlg::UpdatePositionList(PositionNotification notification) {
	int pos = 0;
	Instrument instrument = InstrumentFactory::GetInstruments().Get(notification.InstID);
	USES_CONVERSION;
	if (Direction::BUY == notification.Direction) {
		if (!FindPositionFromList(notification.InstID, Direction::BUY, pos)) {
			m_positionList.InsertItem(pos, A2W(instrument.GetInstCode().Str()));
			m_positionList.SetItemText(pos, 1, _T("买"));
			m_positionList.SetItemText(pos, 2, A2W(fmt::format("{}", notification.Quantity).c_str()));
			m_positionList.SetItemText(pos, 3, A2W(fmt::format("{}", notification.TodayQty).c_str()));
			m_positionList.SetItemText(pos, 4, A2W(fmt::format("{}", notification.Frozen).c_str()));
			m_positionList.SetItemText(pos, 5, A2W(fmt::format("{:.3f}", notification.Price.value()).c_str()));
		}
	}
	else {
		if (!FindPositionFromList(notification.InstID, Direction::SELL, pos)) {
			m_positionList.InsertItem(pos, A2W(instrument.GetInstCode().Str()));
			m_positionList.SetItemText(pos, 1, _T("卖"));
			m_positionList.SetItemText(pos, 2, A2W(fmt::format("{}", notification.Quantity).c_str()));
			m_positionList.SetItemText(pos, 3, A2W(fmt::format("{}", notification.TodayQty).c_str()));
			m_positionList.SetItemText(pos, 4, A2W(fmt::format("{}", notification.Frozen).c_str()));
			m_positionList.SetItemText(pos, 5, A2W(fmt::format("{:.3f}", notification.Price.value()).c_str()));
		}
	}
}

void CpixiuDlg::ChangeCtlColor(CDC* pDc, int ctrlID) {
	CString text;
	GetDlgItem(ctrlID)->GetWindowText(text);
	if (0 == text.Compare(_T("已断开"))) {
		pDc->SetTextColor(RGB(255, 0, 0));
	}
	else if (0 == text.Compare(_T("已连接"))) {
		pDc->SetTextColor(RGB(0, 255, 0));
	}
	else {
		pDc->SetTextColor(RGB(0, 0, 255));
	}
}
void CpixiuDlg::ShowOpenBtn(BOOL bEnable) {
	if (TRUE == bEnable) {
		SetDlgItemText(IDC_BOPEN_BTN, _T("买开"));
		SetDlgItemText(IDC_SOPEN_BTN, _T("卖开"));
	} else {
		SetDlgItemText(IDC_BOPEN_BTN, _T("买"));
		SetDlgItemText(IDC_SOPEN_BTN, _T("卖"));
	}
}
void CpixiuDlg::ShowOffsetBtn(BOOL enableSpin, BOOL isHK) {
	m_buyOffsetSpin.ShowWindow(enableSpin);
	m_sellOffsetSpin.ShowWindow(enableSpin);
	if (TRUE == isHK) {
		m_bOffsetBtn.ShowWindow(!isHK);
		m_sOffsetBtn.ShowWindow(!isHK);
	} else {
		SetDlgItemText(IDC_BOFFSET_BTN, _T("买平"));
		SetDlgItemText(IDC_SOFFSET_BTN, _T("卖平"));
	}
}
void CpixiuDlg::SwitchExchRadio(UINT RadioID) {
	CButton* radioGroup[] = { (CButton*)GetDlgItem(IDC_SHFE_RADIO),(CButton*)GetDlgItem(IDC_DCE_RADIO),(CButton*)GetDlgItem(IDC_CZCE_RADIO),(CButton*)GetDlgItem(IDC_CFFEX_RADIO),(CButton*)GetDlgItem(IDC_SGE_RADIO),(CButton*)GetDlgItem(IDC_HKEX_RADIO) };
	for each (auto radio in radioGroup) {
		if (radio->GetDlgCtrlID() == RadioID) {
			radio->SetCheck(TRUE);
		} else {
			radio->SetCheck(FALSE);
		}
	}
}

bool CpixiuDlg::ParameterValidate() {
	if (-1 == m_instrumentCombo.GetItemData(m_instrumentCombo.GetCurSel())) {
		::AfxMessageBox(_T("请选择合约!"));
		m_instrumentCombo.SetFocus();
		return false;
	}

	int validAmt = 100;
	InstID instID = (InstID)m_instrumentCombo.GetItemData(m_instrumentCombo.GetCurSel());
	if (IsSGE(instID)) {
		validAmt = 2000;
	}
	else if (IsCFFEX(instID)) {
		validAmt = 50;
	}
	if (CommonFunction::WindowText2UINT16(m_amtEdit) < 1 || CommonFunction::WindowText2UINT16(m_amtEdit) > validAmt) {
		::AfxMessageBox(_T("手动委托手数不合法"));
		m_amtEdit.SetFocus();
		return false;
	}
	if (DoubleComparer::less_equal(CommonFunction::WindowText2Double(m_priceEdit), 0)) {
		::AfxMessageBox(_T("委托价格必须大于0!"));
		m_priceEdit.SetFocus();
		return false;
	}
	return true;
}
InstID CpixiuDlg::GetInstID(CString instCode) {
	USES_CONVERSION;
	Instruments const& instruments = InstrumentFactory::GetInstruments();
	for each (Instrument const& inst in instruments.GetAll()) {
		if (0 == instCode.Compare(A2W(inst.GetInstCode().Str()))) {
			return inst.GetID();
		}
	}
	return INVALID_INSTRUMENT_ID;
}
void CpixiuDlg::UpdateQuote() {
	USES_CONVERSION;
	if (-1 != m_instrumentCombo.GetItemData(m_instrumentCombo.GetCurSel())) {
		InstID instID = (InstID)m_instrumentCombo.GetItemData(m_instrumentCombo.GetCurSel());
		Quote const& quote = WritableQuoteService().GetQuote(instID);
		
		char const* priceFormat = CommonFunction::GetPriceFormat(quote.GetInstrument().GetUnitPrice());
		GetDlgItem(IDC_ASK_STATIC)->SetWindowText(A2W(fmt::format(priceFormat, quote.GetSell().GetPrice().value()).c_str()));
		GetDlgItem(IDC_BID_STATIC)->SetWindowText(A2W(fmt::format(priceFormat, quote.GetBuy().GetPrice().value()).c_str()));
		GetDlgItem(IDC_ASK_AMT_STATIC)->SetWindowText(A2W(fmt::format("{}", quote.GetSell().GetQuantity()).c_str()));
		GetDlgItem(IDC_BID_AMT_STATIC)->SetWindowText(A2W(fmt::format("{}", quote.GetBuy().GetQuantity()).c_str()));
	}
}

void CpixiuDlg::Update(Data const& data) {
}


