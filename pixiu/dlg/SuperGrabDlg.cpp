// SuperGrab.cpp: 实现文件
//

#include "pch.h"
#include "pixiu.h"
#include "SuperGrabDlg.h"
#include "afxdialogex.h"

#include "model/factory/InstrumentFactory.h"
#include "notify/UIPublisher.h"
#include "utils/Ctrl.h"
#include "runner/Framework.h"
#include "strategy/StrategyFactory.h"
#include "strategy/Business.h"

// SuperGrab 对话框


IMPLEMENT_DYNAMIC(SuperGrabDlg, CDialogEx)

using namespace tg::grab;
SuperGrabDlg::SuperGrabDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_TG_GRAB_DLG, pParent)
// 	, m_notifyColumn(0), m_arguments{ GBOrder{ 0 }, GBOrder{}/*默认值是INVALID*/, m_startTime, 500/*SGE委托间隔*/, 1000/*起始频率*/, 12, 220} {
// 	, m_notifyColumn(0), m_arguments{ GBOrder{ 0 }, GBOrder{}/*默认值是INVALID*/, m_startTime, 800/*CFFEX委托间隔*/, 2000/*起始频率*/, 25, 350} {
	, m_notifyColumn(0), m_arguments{ GBOrder{ 0 }, GBOrder{}/*默认值是INVALID*/, m_startTime, 500/*SHFE委托间隔*/, 2000/*起始频率*/, 10, 330 } {
	::GetLocalTime(&m_startTime);
	m_startTime.wHour = 9;
	m_startTime.wMinute = 00;
	m_startTime.wSecond = 00;
	m_startTime.wMilliseconds = 158;
}

SuperGrabDlg::~SuperGrabDlg() {
}

void SuperGrabDlg::DoDataExchange(CDataExchange* pDX) {
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_combo1);
	DDX_Control(pDX, IDC_GRAB_BUSINESS_LIST, m_businessList);
	DDX_Control(pDX, IDC_COMBO2, m_combo2);
	DDX_Control(pDX, IDC_OP_PRICE1, m_opPrice1);
	DDX_Control(pDX, IDC_OP_PRICE2, m_opPrice2);
	DDX_Control(pDX, IDC_ST_PRICE1, m_stPrice1);
	DDX_Control(pDX, IDC_ST_PRICE2, m_stPrice2);
	DDX_Control(pDX, IDC_OP_QTY1, m_openQty1);
	DDX_Control(pDX, IDC_OP_QTY2, m_openQty2);
	DDX_Control(pDX, IDC_OF_PRICE1, m_ofPrice1);
	DDX_Control(pDX, IDC_OF_PRICE2, m_ofPrice2);
	DDX_Control(pDX, IDC_HD_QTY1, m_holdQty1);
	DDX_Control(pDX, IDC_HD_QTY2, m_holdQty2);
	DDX_Control(pDX, IDC_PERIOD_EDIT, m_period);
	DDX_Control(pDX, IDC_EXPERIOD_EDIT, m_exPeriod);
	DDX_Control(pDX, IDC_AWAKE_AMT, m_awakeAmt);
	DDX_Control(pDX, IDC_TOTAL_AMT, m_totalAmt);
	DDX_Control(pDX, IDC_HOUR_COMBO, m_hourCombo);
	DDX_Control(pDX, IDC_MINUTE_COMBO, m_minuteCombo);
	DDX_Control(pDX, IDC_SECOND_COMBO, m_secondCombo);
	DDX_Control(pDX, IDC_CHECK1, m_instCheck);
	DDX_Control(pDX, IDC_OF_PRICE_CHECK1, m_holdCheck1);
	DDX_Control(pDX, IDC_OF_PRICE_CHECK2, m_holdCheck2);
	DDX_Control(pDX, IDC_MICRO_SECOND, m_msEdit);
}

BEGIN_MESSAGE_MAP(SuperGrabDlg, CDialogEx)
	ON_BN_CLICKED(IDC_ADD_GRAB, &SuperGrabDlg::OnBnClickedAddGrab)
	ON_BN_CLICKED(IDC_MDF_GRAB, &SuperGrabDlg::OnBnClickedMdfGrab)
	ON_CBN_SELCHANGE(IDC_COMBO1, &SuperGrabDlg::OnCbnSelchangeCombo1)
	ON_WM_SIZE()
	ON_NOTIFY(LVN_KEYDOWN, IDC_GRAB_BUSINESS_LIST, &SuperGrabDlg::OnLvnKeydownGrabStrategyList)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_GRAB_BUSINESS_LIST, &SuperGrabDlg::OnLvnItemchangedGrabStrategyList)
	ON_BN_CLICKED(IDC_CHECK1, &SuperGrabDlg::OnBnClickedInstCheck)
	ON_BN_CLICKED(IDC_OF_PRICE_CHECK1, &SuperGrabDlg::OnBnClickedHoldCheck1)
	ON_BN_CLICKED(IDC_OF_PRICE_CHECK2, &SuperGrabDlg::OnBnClickedHoldCheck2)
END_MESSAGE_MAP()


// SuperGrab 消息处理程序
void SuperGrabDlg::OnOK() {
}
void SuperGrabDlg::OnCancel() {
	BOOL bSwitch = m_combo1.IsWindowEnabled();
	m_combo1.EnableWindow(!bSwitch);
	m_instCheck.EnableWindow(!bSwitch);
}
BOOL SuperGrabDlg::OnInitDialog() {
	CDialogEx::OnInitDialog();

	// Set up the tooltip
	m_toolTips.Create(this);
	m_toolTips.AddTool(GetDlgItem(IDC_MICRO_SECOND), _T("抢中立仓时表示延迟后的提前时间"));
	m_toolTips.Activate(TRUE);

	InitCtrls();
	InitListCtrls();

	{
		GetWindowRect(&m_windowRect);		//获取窗口大小
		m_oldPoint.x = m_windowRect.right - m_windowRect.left;
		m_oldPoint.y = m_windowRect.bottom - m_windowRect.top;
	}	//for size

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}
BOOL SuperGrabDlg::PreTranslateMessage(MSG* pMsg) {
	if (NULL != m_toolTips) {
		m_toolTips.RelayEvent(pMsg);
	}
	return __super::PreTranslateMessage(pMsg);
}


void SuperGrabDlg::OnSize(UINT nType, int cx, int cy) {
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
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
			if (IDC_COMBO1 == ctrlId || IDC_COMBO2 == ctrlId || IDC_HOUR_COMBO == ctrlId || IDC_MINUTE_COMBO == ctrlId || IDC_SECOND_COMBO == ctrlId) {
				NewBRPoint.y += 80;
			}
			Rect.SetRect(NewTLPoint, NewBRPoint);
			GetDlgItem(ctrlId)->MoveWindow(Rect, TRUE);
			hwndChild = ::GetWindow(hwndChild, GW_HWNDNEXT);
		}
		m_oldPoint = newPoint;
	}
}
void SuperGrabDlg::OnBnClickedAddGrab() {
	Update2Arguments();						// 从界面取参数到m_arguments;
	if (ParameterValidate()) {
		if (m_businessList.GetItemCount() > 0) {
			Instrument inst = InstrumentFactory::GetInstruments().Get(m_arguments.Order.Inst);
			CString desc(fmt::format("增加新合约[{}]会导致之前策略暂停，是否继续?", inst.GetInstCode().Str()).c_str());			
			if (IDNO == MessageBox(desc, AfxGetApp()->m_pszAppName, MB_YESNO | MB_ICONQUESTION)) {
				return;
			}
			for (int i = 0; i <= m_businessList.GetItemCount(); ++i) {
				StrategyCommand command;
				command.Type = StrategyCommand::PAUSE;
				command.BusinessID = i;
				command.Business = nullptr;
				Framework& framework = Framework::Instance();
				framework.GetUIPublisher().Push(command);
			}
		}
		Framework& framework = Framework::Instance();
		auto business = AddBusiness(m_arguments);
		framework.GetUIPublisher().AddBusiness(&*business);
		m_businesses.push_back(business);

		InsertStrategyList(business->GetId(), m_arguments);
		Update2Ctrls(m_arguments);	// 界面更新为最新设置的参数
	}
}
void SuperGrabDlg::OnBnClickedMdfGrab() {
	int index = m_businessList.GetSelectionMark();
	if (-1 == index) return;

	if (ParameterValidate()) {
		Update2Arguments();			// 从界面取参数到arguments;

		StrategyCommand command;
		command.Type = StrategyCommand::UPDATE_ARGUMENT;
		int businessID = int(m_businessList.GetItemData(index));
		command.Business = nullptr;
		command.BusinessID = businessID;
		Arguments& argument = *(Arguments*)command.Data;
		argument = m_arguments;
		Framework::Instance().GetUIPublisher().Push(command);

		UpdateStrategyList(index, m_arguments);
		Update2Ctrls(m_arguments);	// 界面更新为最新修改的参数
	}
}
void SuperGrabDlg::OnBnClickedInstCheck() {
	BOOL isChecked = (m_instCheck.GetCheck() == BST_CHECKED) ? TRUE : FALSE;
	CWnd* enableWnds[] = { &m_combo2, &m_opPrice2, &m_stPrice2, &m_openQty2, &m_holdCheck2, &m_ofPrice2, &m_holdQty2 };
	CommonFunction::EnableWindows(enableWnds, CommonFunction::ArraySize(enableWnds), isChecked);
	m_holdCheck2.SetCheck(FALSE);
	m_holdQty2.EnableWindow(FALSE);

	if (FALSE == isChecked) {	// 合约2界面参数全部初始化成0
		CommonFunction::Double2WindowText(m_opPrice2, 0.0, 0);
		CommonFunction::Double2WindowText(m_stPrice2, 0.0, 0);
		CommonFunction::Double2WindowText(m_ofPrice2, 0.0, 0);
		CommonFunction::Int2WindowText(m_openQty2, 0);
		CommonFunction::Int2WindowText(m_holdQty2, 0);
	}
}
void SuperGrabDlg::OnBnClickedHoldCheck1() {
	BOOL isEnable = (m_holdCheck1.GetCheck() == BST_CHECKED) ? TRUE : FALSE;
	m_holdQty1.EnableWindow(isEnable);
	if (FALSE == isEnable) {
		CommonFunction::Int2WindowText(m_holdQty1, 0);
	}
}
void SuperGrabDlg::OnBnClickedHoldCheck2() {	
	BOOL isEnable = (m_holdCheck2.GetCheck() == BST_CHECKED) ? TRUE : FALSE;
	m_holdQty2.EnableWindow(isEnable);
	if (FALSE == isEnable) {
		CommonFunction::Int2WindowText(m_holdQty2, 0);
	}
}
void SuperGrabDlg::OnCbnSelchangeCombo1() {
	InitCombo2Data((int)m_combo1.GetItemData(m_combo1.GetCurSel()));
}
void SuperGrabDlg::OnLvnKeydownGrabStrategyList(NMHDR* pNMHDR, LRESULT* pResult) {
	LPNMLVKEYDOWN pLVKeyDow = reinterpret_cast<LPNMLVKEYDOWN>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	int index = m_businessList.GetSelectionMark();
	if (-1 == index)
		return;
	ShortcutsHandler(index, pLVKeyDow->wVKey);

	*pResult = 0;
}
void SuperGrabDlg::OnLvnItemchangedGrabStrategyList(NMHDR* pNMHDR, LRESULT* pResult) {
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	if ((pNMLV->uChanged & LVIF_STATE) && (pNMLV->uNewState & LVNI_SELECTED)) {
		int businessID = int(m_businessList.GetItemData(pNMLV->iItem));
		if (businessID > 0) {
			int itemIndex = m_itemIndexMap[businessID];
			m_arguments = m_gbArgumentsMap.at(itemIndex);
			Update2Ctrls(m_arguments);
			CWnd* enableWnds[] = { &m_combo1, &m_combo2, &m_instCheck};	// 合约名称和数量都不可修改
			CommonFunction::EnableWindows(enableWnds, CommonFunction::ArraySize(enableWnds), FALSE);
		}
	}
	*pResult = 0;
}

void SuperGrabDlg::InitCtrls() {
	InitInstCombos();						// 初始化合约Combo
	InitStartTimeCtrls(m_startTime);		// 初始化时间Combo
	m_combo1.EnableWindow(TRUE);
	m_holdCheck1.SetCheck(BST_UNCHECKED);	// 默认无保留持仓
	OnBnClickedHoldCheck1();

	Instruments const& instruments = InstrumentFactory::GetInstruments();	
	int precision1 = CommonFunction::GetPricePrecision(instruments.Get(m_arguments.Order.Inst).GetUnitPrice());
	CommonFunction::Double2WindowText(m_opPrice1, m_arguments.Order.OpenPrice.value(), precision1);
	CommonFunction::Double2WindowText(m_stPrice1, m_arguments.Order.StopPrice.value(), precision1);
	CommonFunction::Double2WindowText(m_ofPrice1, m_arguments.Order.OffsetPrice.value(), precision1);
	CommonFunction::Int2WindowText(m_openQty1, m_arguments.Order.OpenQty);
	CommonFunction::Int2WindowText(m_holdQty1, m_arguments.Order.HoldQty);
	m_instCheck.SetCheck(BST_UNCHECKED);	// 默认不启动第二合约
	OnBnClickedInstCheck();
	CommonFunction::Int2WindowText(m_exPeriod, m_arguments.ExPeriod);
	CommonFunction::Int2WindowText(m_awakeAmt, m_arguments.AwakeCnt);
	CommonFunction::Int2WindowText(m_period, m_arguments.Period);
	CommonFunction::Int2WindowText(m_totalAmt, m_arguments.TotalCnt);
}
void SuperGrabDlg::InitListCtrls() {
	int column = 0;
	m_businessList.InsertColumn(column++, _T("合约"), LVCFMT_LEFT, 150);
	m_businessList.InsertColumn(column++, _T("开仓参数"), LVCFMT_RIGHT, 225);
	m_businessList.InsertColumn(column++, _T("时间参数"), LVCFMT_CENTER, 150);
	m_businessList.InsertColumn(column++, _T("平仓参数"), LVCFMT_LEFT, 225);
	m_notifyColumn = column;
	m_businessList.InsertColumn(column++, _T("持仓"), LVCFMT_CENTER, 70);
	m_businessList.InsertColumn(column++, _T("状态"), LVCFMT_LEFT, 70);
	m_businessList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP);
}
void SuperGrabDlg::InitCombo1Data() {
	m_combo1.ResetContent();
	Instruments const& instruments = InstrumentFactory::GetInstruments();
	for each (Instrument const& inst in instruments.GetAll()) {
		CString instCode(inst.GetInstCode().Str());
		int index = m_combo1.AddString(instCode);
		m_combo1.SetItemData(index, inst.GetID());
	}
	m_combo1.SetCurSel(0);
}
void SuperGrabDlg::InitCombo2Data(int instID) {
	m_combo2.ResetContent();
	USES_CONVERSION;
	Instruments const& instruments = InstrumentFactory::GetInstruments();
	Instrument majorInst = instruments.Get(instID);
	CString instCode = A2W(majorInst.GetVariety().GetCode().c_str());
	for each (Instrument const& inst in instruments.GetAll()) {
		if (majorInst.GetExchange() == inst.GetExchange()/*同交易所*/ && inst.GetID() != majorInst.GetID()/*合约不同*/) {
			CString contractCode(inst.GetInstCode().Str());
			int index = m_combo2.AddString(contractCode);
			m_combo2.SetItemData(index, inst.GetID());
		}
	}
	m_combo2.SetCurSel(0);
}
void SuperGrabDlg::InitInstCombos() {
	InitCombo1Data();
	InitCombo2Data((InstID)m_combo1.GetItemData(0));
}
void SuperGrabDlg::InitStartTimeCtrls(SYSTEMTIME const& startTime) {
	UpdateTimeComboSel(m_hourCombo, startTime.wHour);
	UpdateTimeComboSel(m_minuteCombo, startTime.wMinute);
	UpdateTimeComboSel(m_secondCombo, startTime.wSecond);
	CommonFunction::Int2WindowText(m_msEdit, startTime.wMilliseconds);
}

void SuperGrabDlg::Update2Ctrls(Arguments& arguments) {
	// 更新合约1界面参数
	UpdateInstComboSel(m_combo1, arguments.Order.Inst);
	money unitPrice = InstrumentFactory::GetInstruments().Get(arguments.Order.Inst).GetUnitPrice();
	int precision = CommonFunction::GetPricePrecision(unitPrice);
	CommonFunction::Double2WindowText(m_opPrice1, arguments.Order.OpenPrice.value(), precision);
	CommonFunction::Double2WindowText(m_stPrice1, arguments.Order.StopPrice.value(), precision);
	CommonFunction::Double2WindowText(m_ofPrice1, arguments.Order.OffsetPrice.value(), precision);
	CommonFunction::Int2WindowText(m_openQty1, arguments.Order.OpenQty);
	CommonFunction::Int2WindowText(m_holdQty1, arguments.Order.HoldQty);
	m_holdCheck1.SetCheck(arguments.Order.HoldQty ? BST_CHECKED :BST_UNCHECKED);
	OnBnClickedHoldCheck1();
	// 更新合约2界面参数
	if (IsInvalidInstID(arguments.ExOrder.Inst)) {
		m_instCheck.SetCheck(BST_UNCHECKED);	// 不会触发OnBnClickedInstCheck,需要自己调用
		OnBnClickedInstCheck();
	}
	else {
		m_instCheck.SetCheck(BST_CHECKED);		// 不会触发OnBnClickedInstCheck,需要自己调用
		OnBnClickedInstCheck();
		InitCombo2Data(arguments.Order.Inst);
		UpdateInstComboSel(m_combo2, arguments.ExOrder.Inst);
		unitPrice = InstrumentFactory::GetInstruments().Get(arguments.ExOrder.Inst).GetUnitPrice();
		precision = CommonFunction::GetPricePrecision(unitPrice);
		CommonFunction::Double2WindowText(m_opPrice2, arguments.ExOrder.OpenPrice.value(), precision);
		CommonFunction::Double2WindowText(m_stPrice2, arguments.ExOrder.StopPrice.value(), precision);
		CommonFunction::Double2WindowText(m_ofPrice2, arguments.ExOrder.OffsetPrice.value(), precision);
		CommonFunction::Int2WindowText(m_openQty2, arguments.ExOrder.OpenQty);
		CommonFunction::Int2WindowText(m_holdQty2, arguments.ExOrder.HoldQty);
		m_holdCheck2.SetCheck(arguments.ExOrder.HoldQty ? BST_CHECKED : BST_UNCHECKED);
		OnBnClickedHoldCheck2();
	}

	// 更新时间界面参数
	m_startTime = arguments.StartTime;
	InitStartTimeCtrls(m_startTime);

	// 更新频率界面参数
	CommonFunction::Int2WindowText(m_exPeriod, arguments.ExPeriod);
	CommonFunction::Int2WindowText(m_awakeAmt, arguments.AwakeCnt);
	CommonFunction::Int2WindowText(m_period, arguments.Period);
	CommonFunction::Int2WindowText(m_totalAmt, arguments.TotalCnt);
}
void SuperGrabDlg::Update2Arguments() {
	m_startTime.wHour = CommonFunction::WindowText2Int(m_hourCombo);
	m_startTime.wMinute = CommonFunction::WindowText2Int(m_minuteCombo);
	m_startTime.wSecond = CommonFunction::WindowText2Int(m_secondCombo);
	m_startTime.wMilliseconds = CommonFunction::WindowText2Int(m_msEdit);

	InstID Inst = (InstID)m_combo1.GetItemData(m_combo1.GetCurSel());
	GBOrder order = { Inst, CommonFunction::WindowText2Double(m_opPrice1), CommonFunction::WindowText2Double(m_ofPrice1), CommonFunction::WindowText2Double(m_stPrice1)
		, CommonFunction::WindowText2UINT16(m_openQty1), CommonFunction::WindowText2UINT16(m_holdQty1), GetProductClass(Inst) };
	GBOrder exOrder;
	if (BST_CHECKED == m_instCheck.GetCheck() && CommonFunction::WindowText2UINT16(m_openQty2) > 0) {
		exOrder.Inst = (InstID)m_combo2.GetItemData(m_combo2.GetCurSel());
		exOrder.OpenPrice = CommonFunction::WindowText2Double(m_opPrice2);
		exOrder.OffsetPrice = CommonFunction::WindowText2Double(m_ofPrice2);
		exOrder.StopPrice = CommonFunction::WindowText2Double(m_stPrice2);
		exOrder.OpenQty = CommonFunction::WindowText2UINT16(m_openQty2);
		exOrder.HoldQty = CommonFunction::WindowText2UINT16(m_holdQty2);
		exOrder.ProductClass = GetProductClass(exOrder.Inst);
	}
	m_arguments = Arguments{ order, exOrder, m_startTime, CommonFunction::WindowText2UINT32(m_period), CommonFunction::WindowText2UINT32(m_exPeriod),
		CommonFunction::WindowText2Int(m_awakeAmt), CommonFunction::WindowText2Int(m_totalAmt) };
}
void SuperGrabDlg::UpdateTimeComboSel(CComboBox& combo, int value) {
	CString comboTxt;
	combo.SetCurSel(0);
	for (int i=0; i<combo.GetCount(); ++i) {
		combo.GetLBText(i, comboTxt);
		if (value == _ttoi(comboTxt)) {
			combo.SetCurSel(i);
			return;
		}
	}
}
void SuperGrabDlg::UpdateInstComboSel(CComboBox& combo, int InstID) {
	combo.SetCurSel(0);
	for (int i = 0; i < combo.GetCount(); ++i) {
		if ((int)combo.GetItemData(i) == InstID) {
			combo.SetCurSel(i);
			break;
		}
	}
}

std::shared_ptr<Business> SuperGrabDlg::AddBusiness(Arguments const& argument) {
	Framework& framework = Framework::Instance();
	return StrategyFactory::NewSuperGrab(this, argument, framework.GetStrategyPublisher());
}
void SuperGrabDlg::InsertStrategyList(int businessID, Arguments const& arguments) {
	USES_CONVERSION;
	Instruments instruments = InstrumentFactory::GetInstruments();

	int itemIndex = 0;
	int column = 1;
	int count = m_businessList.GetItemCount();	
	SYSTEMTIME const& time = arguments.StartTime;
	GBOrder order = arguments.Order;
	char const* openDir = (order.OpenPrice > order.StopPrice) ? "买:" : "卖:";
	char const* offsetDir = (order.OffsetPrice > order.OpenPrice) ? "卖:" : "买: ";
	GBOrder exOrder = arguments.ExOrder;
	if (exOrder.OpenQty <= 0) {
		itemIndex = m_businessList.InsertItem(count, A2W(fmt::format("{}", instruments.Get(order.Inst).GetInstCode().Str()).c_str()));
		m_businessList.SetSelectionMark(itemIndex);

		m_businessList.SetItemText(itemIndex, column++, A2W(fmt::format("{}[{:.3f}, {}]", openDir, order.OpenPrice, order.OpenQty).c_str()));
		m_businessList.SetItemText(itemIndex, column++, A2W(fmt::format("{:02d}:{:02d}:{:02d}.{:03d}", time.wHour, time.wMinute, time.wSecond, time.wMilliseconds).c_str()));
		if (order.HoldQty > 0) {
			m_businessList.SetItemText(itemIndex, column++, A2W(fmt::format("{}[{:.3f}] 留[{}]", offsetDir, order.OffsetPrice, order.HoldQty).c_str()));
		}
		else {
			m_businessList.SetItemText(itemIndex, column++, A2W(fmt::format("{}[{:.3f}]", offsetDir, order.OffsetPrice).c_str()));
		}
	}
	else {
		itemIndex = m_businessList.InsertItem(count, A2W(fmt::format("{} | {}", instruments.Get(order.Inst).GetInstCode().Str(), instruments.Get(exOrder.Inst).GetInstCode().Str()).c_str()));
		m_businessList.SetSelectionMark(itemIndex);

		char const* exOpenDir = (exOrder.OpenPrice > exOrder.StopPrice) ? "买:" : "卖:";
		char const* exOffsetDir = (exOrder.OffsetPrice > exOrder.OpenPrice) ? "卖:" : "买:";
		m_businessList.SetItemText(itemIndex, column++, A2W(fmt::format("{}[{:.3f}, {}] - {}[{:.3f}, {}]",
			openDir, order.OpenPrice, order.OpenQty, exOpenDir, exOrder.OpenPrice, exOrder.OpenQty).c_str()));
		m_businessList.SetItemText(itemIndex, column++, A2W(fmt::format("{:02d}:{:02d}:{:02d}.{:03d}", time.wHour, time.wMinute, time.wSecond, time.wMilliseconds).c_str()));
		if (order.HoldQty > 0 && exOrder.HoldQty > 0) {
			m_businessList.SetItemText(itemIndex, column++, A2W(fmt::format("{}[{:.3f}] 留[{}] - {}[{:.3f}] 留[{}]",
				offsetDir, order.OffsetPrice, order.HoldQty, exOffsetDir, exOrder.OffsetPrice, exOrder.HoldQty).c_str()));
		}	// 两腿都有保留
		else if (order.HoldQty > 0 && 0 == exOrder.HoldQty) {
			m_businessList.SetItemText(itemIndex, column++, A2W(fmt::format("{}[{:.3f}] 留[{}] - {}[{:.3f}]",
				offsetDir, order.OffsetPrice, order.HoldQty, exOffsetDir, exOrder.OffsetPrice).c_str()));
		}	// 第一腿有保留
		else if (0 == order.HoldQty && exOrder.HoldQty > 0) {
			m_businessList.SetItemText(itemIndex, column++, A2W(fmt::format("{}[{:.3f}] - {}[{:.3f}] 留[{}]",
				offsetDir, order.OffsetPrice, exOffsetDir, exOrder.OffsetPrice, exOrder.HoldQty).c_str()));
		}	// 第二腿有保留
		else {
			m_businessList.SetItemText(itemIndex, column++, A2W(fmt::format("{}[{:.3f}] - {}[{:.3f}]", 
				offsetDir, order.OffsetPrice, exOffsetDir, exOrder.OffsetPrice).c_str()));
		}	// 两腿都无保留
	}
	m_notifyColumn = column;
	m_businessList.SetItemText(itemIndex, column++, A2W(fmt::format("{} | {}", 0, 0).c_str()));
	m_businessList.SetItemText(itemIndex, column++, _T("New"));

	m_businessList.SetItemData(itemIndex, (DWORD_PTR)businessID);
	m_itemIndexMap[businessID] = itemIndex;
	m_gbArgumentsMap.insert(std::make_pair(itemIndex, m_arguments));
}
void SuperGrabDlg::UpdateStrategyList(int itemIndex, Arguments const& arguments) {
	USES_CONVERSION;
	int column = 1;
	GBOrder order = arguments.Order;
	GBOrder exOrder = arguments.ExOrder;
	SYSTEMTIME const& time = arguments.StartTime;
	char const* openDir = (order.OpenPrice > order.StopPrice) ? "买:" : "卖:";
	char const* offsetDir = (order.OffsetPrice > order.OpenPrice) ? "卖:" : "买:";
	if (exOrder.OpenQty <= 0) {
		m_businessList.SetItemText(itemIndex, column++, A2W(fmt::format("{}[{:.3f}, {}]", openDir, order.OpenPrice, order.OpenQty).c_str()));
		m_businessList.SetItemText(itemIndex, column++, A2W(fmt::format("{:02d}:{:02d}:{:02d}.{:03d}", time.wHour, time.wMinute, time.wSecond, time.wMilliseconds).c_str()));
		if (order.HoldQty > 0) {
			m_businessList.SetItemText(itemIndex, column++, A2W(fmt::format("{}[{:.3f}] Hold[{}]", offsetDir, order.OffsetPrice, order.HoldQty).c_str()));
		}
		else {
			m_businessList.SetItemText(itemIndex, column++, A2W(fmt::format("{}[{:.3f}]", offsetDir, order.OffsetPrice).c_str()));
		}
	}
	else {
		char const* exOpenDir = (exOrder.OpenPrice > exOrder.StopPrice) ? "买:" : "卖:";
		char const* exOffsetDir = (exOrder.OffsetPrice > exOrder.OpenPrice) ? "卖:" : "买:";
		m_businessList.SetItemText(itemIndex, column++, A2W(fmt::format("{}[{:.3f}, {}] | {}[{:.3f}, {}]",
			openDir, order.OpenPrice, order.OpenQty, exOpenDir, exOrder.OpenPrice, exOrder.OpenQty).c_str()));
		m_businessList.SetItemText(itemIndex, column++, A2W(fmt::format("{:02d}:{:02d}:{:02d}.{:03d}", time.wHour, time.wMinute, time.wSecond, time.wMilliseconds).c_str()));
		if (order.HoldQty > 0 && exOrder.HoldQty > 0) {
			m_businessList.SetItemText(itemIndex, column++, A2W(fmt::format("{}[{:.3f}] Hold[{}] | {}[{:.3f} Hold[{}]",
				offsetDir, order.OffsetPrice, order.HoldQty, exOffsetDir, exOrder.OffsetPrice, exOrder.HoldQty).c_str()));
		}	// 两腿都有保留
		else if (order.HoldQty > 0 && 0 == exOrder.HoldQty) {
			m_businessList.SetItemText(itemIndex, column++, A2W(fmt::format("{}[{:.3f} Hold[{}] | {}[{:.3f}]",
				offsetDir, order.OffsetPrice, order.HoldQty, exOffsetDir, exOrder.OffsetPrice).c_str()));
		}	// 第一腿有保留
		else if (0 == order.HoldQty && exOrder.HoldQty > 0) {
			m_businessList.SetItemText(itemIndex, column++, A2W(fmt::format("{}[{:.3f}] | {}[{:.3f} Hold[{}]",
				offsetDir, order.OffsetPrice, exOffsetDir, exOrder.OffsetPrice, exOrder.HoldQty).c_str()));
		}	// 第二腿有保留
		else {
			m_businessList.SetItemText(itemIndex, column++, A2W(fmt::format("{}[{:.3f}] | {}[{:.3f}]",
				offsetDir, order.OffsetPrice, exOffsetDir, exOrder.OffsetPrice).c_str()));
		}	// 两腿都无保留
	}
	m_gbArgumentsMap.at(itemIndex) = arguments;		// 更新参数集
}

bool SuperGrabDlg::ParameterValidate() {
	if (BST_UNCHECKED == m_instCheck.GetState() && CommonFunction::WindowText2Int(m_period) < 50) {
		::AfxMessageBox(_T("单发间隔不得小于50!"), MB_ICONWARNING);
		m_totalAmt.SetFocus();
		return false;
	}
	if (BST_CHECKED == m_instCheck.GetState() && CommonFunction::WindowText2Int(m_period) < 150) {
		::AfxMessageBox(_T("双发间隔不得小于150!"), MB_ICONWARNING);
		m_totalAmt.SetFocus();
		return false;
	}
	if (CommonFunction::WindowText2Int(m_totalAmt) < 5 || CommonFunction::WindowText2Int(m_totalAmt) > 500) {
		::AfxMessageBox(_T("发单量需大于5,小于500!"), MB_ICONWARNING);
		m_totalAmt.SetFocus();
		return false;
	}

	if (CommonFunction::WindowText2Int(m_openQty1) < 1) {
		::AfxMessageBox(_T("合约1开仓数量需大于0!"), MB_ICONWARNING);
		m_openQty1.SetFocus();
		return false;
	}
	double openPrice1 = CommonFunction::WindowText2Double(m_opPrice1);
	double stopPrice1 = CommonFunction::WindowText2Double(m_stPrice1);
	double offsetPrice1 = CommonFunction::WindowText2Double(m_ofPrice1);
	if (openPrice1 < 1) {
		::AfxMessageBox(_T("合约1开仓价格需大于0!"), MB_ICONWARNING);
		m_opPrice1.SetFocus();
		return false;
	}
	if (offsetPrice1 < 1 || openPrice1 == stopPrice1) {
		::AfxMessageBox(_T("合约1平仓价格需大于0,且不等于止损价!"), MB_ICONWARNING);
		m_ofPrice1.SetFocus();
		return false;
	}
	if (DoubleComparer::greater(openPrice1, stopPrice1) && DoubleComparer::less(offsetPrice1, openPrice1)) {
		m_ofPrice1.SetFocus();
		::AfxMessageBox(_T("合约1买平仓价格不能小于开仓价!"), MB_ICONWARNING);
		return false;
	}	// Buy, 可以同价位进出
	if (DoubleComparer::less(openPrice1, stopPrice1) && DoubleComparer::greater(offsetPrice1, openPrice1)) {
		m_ofPrice1.SetFocus();
		::AfxMessageBox(_T("合约1卖平仓价格不能大于开仓价"), MB_ICONWARNING);
		return false;
	}	// Sell, 可以同价位进出
	if (BST_CHECKED == m_holdCheck1.GetState()) {
		auto holdQty = CommonFunction::WindowText2Int(m_holdQty1);
		if (m_startTime.wHour < 14 && (holdQty < 1 || holdQty > CommonFunction::WindowText2Int(m_openQty1))) {
			::AfxMessageBox(_T("合约1保留数量不能为0且不得超过开仓数量"), MB_ICONWARNING);
			m_holdQty1.SetFocus();
			return false;
		}
	}
	if (m_startTime.wHour > 14 && m_startTime.wHour < 16 && BST_UNCHECKED == m_holdCheck1.GetState()) {
		::AfxMessageBox(_T("中立仓或交割申报必须设置保留数量"), MB_ICONWARNING);
		m_holdCheck1.SetCheck(BST_CHECKED);
		OnBnClickedHoldCheck1();
		m_holdQty1.SetFocus();
		return false;
	}

	if (BST_CHECKED == m_instCheck.GetState()) {
		if (CommonFunction::WindowText2Int(m_openQty2) < 1) {
			::AfxMessageBox(_T("合约2开仓数量需大于0!"), MB_ICONWARNING);
			m_openQty2.SetFocus();
			return false;
		}
		double openPrice2 = CommonFunction::WindowText2Double(m_opPrice2);
		double stopPrice2 = CommonFunction::WindowText2Double(m_stPrice2);
		double offsetPrice2 = CommonFunction::WindowText2Double(m_ofPrice2);
		if (openPrice2 < 1) {
			::AfxMessageBox(_T("合约2开仓价格需大于0!"), MB_ICONWARNING);
			m_opPrice2.SetFocus();
			return false;
		}
		if (offsetPrice2 < 1 || openPrice2 == stopPrice2) {
			::AfxMessageBox(_T("合约2平仓价格需大于0,且不等于止损价!"), MB_ICONWARNING);
			m_ofPrice2.SetFocus();
			return false;
		}
		if (DoubleComparer::greater(openPrice2, stopPrice2) && DoubleComparer::less(offsetPrice2, openPrice2)) {
			::AfxMessageBox(_T("合约2买平仓价格不能小于开仓价!"), MB_ICONWARNING);
			m_ofPrice2.SetFocus();
			return false;
		}
		if (DoubleComparer::less(openPrice2, stopPrice2) && DoubleComparer::greater(offsetPrice2, openPrice2)) {
			::AfxMessageBox(_T("合约2卖平仓价格不能大于开仓价!"), MB_ICONWARNING);
			m_ofPrice2.SetFocus();
			return false;
		}
		if (BST_CHECKED == m_holdCheck2.GetState()) {
			auto holdQty = CommonFunction::WindowText2Int(m_holdQty2);
			if (m_startTime.wHour < 14 && (holdQty < 1 || holdQty > CommonFunction::WindowText2Int(m_openQty2))) {
				::AfxMessageBox(_T("合约2保留数量不能为0且不得超过开仓数量!"), MB_ICONWARNING);
				m_holdQty2.SetFocus();
				return false;
			}
		}
		if (m_startTime.wHour > 14 && m_startTime.wHour < 16 && BST_UNCHECKED == m_holdCheck2.GetState()) {
			::AfxMessageBox(_T("中立仓或交割申报必须设置保留数量"), MB_ICONWARNING);
			m_holdCheck2.SetCheck(BST_CHECKED);
			OnBnClickedHoldCheck2();
			m_holdQty2.SetFocus();
			return false;
		}
	}
	return true;
}
TSGEProductClassType SuperGrabDlg::GetProductClass(InstID instID) {
	if (!IsSGE(instID)) {
		return SGE_OC_Unknown;
	}

	if (m_startTime.wHour >= 15 && m_startTime.wMinute >= 3) {
		return SGE_OC_Neutral;
	}	// 15:03以后的抢单一定是中立仓
	else if (m_startTime.wHour >= 14) {
		return SGE_OC_Delivery;
	}	// 14:00以后的抢单一定是交割申报

	return InstrumentFactory::GetInstruments().Get(instID).GetVariety().IsSpot() ? SGE_OC_Spot : SGE_OC_Defer;
}

int SuperGrabDlg::GetCommandType(WORD key) {
	switch (key) {
	case VK_F2:
		return StrategyCommand::PAUSE;
	case VK_F5:
		return StrategyCommand::START;
	default:
		return StrategyCommand::NONE;
	}
}
void SuperGrabDlg::ShortcutsHandler(int index, WORD key) {
	StrategyCommand command;
	command.Type = GetCommandType(key);
	command.BusinessID = int(m_businessList.GetItemData(index));
	command.Business = nullptr;

	Framework& framework = Framework::Instance();
	switch (command.Type) {
	case StrategyCommand::START: {
		framework.GetUIPublisher().Push(command);
		break;
	}
	case StrategyCommand::PAUSE: {
		framework.GetUIPublisher().Push(command);
		break;
	}
	default:
		break;
	}
}

void SuperGrabDlg::Update(Data const& data) {
	USES_CONVERSION;
	BusinessSnapshot const& updateData = *(BusinessSnapshot*)data.Data;
	int itemIndex = m_itemIndexMap[updateData.ID];
	if (itemIndex == -1)	return;

	CString status(Desc(updateData.Status));
	m_businessList.SetItemText(itemIndex, m_notifyColumn, A2W(fmt::format("{} | {}", updateData.Position1, updateData.Position2).c_str()));
	m_businessList.SetItemText(itemIndex, m_notifyColumn + 1, status);
}
