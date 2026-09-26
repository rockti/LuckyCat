// D:\GitSource\px\pixiu\HFNetural.cpp: 实现文件
//

#include "pch.h"
#include "pixiu.h"
#include "HFNeturalDlg.h"
#include "afxdialogex.h"

#include "model/factory/InstrumentFactory.h"
#include "runner/Framework.h"
#include "strategy/StrategyFactory.h"
#include "strategy/Business.h"
#include "strategy/hf_marketing/PriceBusiness.h"
#include "notify/UIPublisher.h"
#include "utils/Ctrl.h"
// HFNetural 对话框

using namespace hf::netural;
IMPLEMENT_DYNAMIC(HFNeturalDlg, CDialogEx)

HFNeturalDlg::HFNeturalDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_HF_NETURAL_DLG, pParent), m_notifyColumn(0)
	, m_arguments{ RunningMode::Balanced, HFInstruments(0,1), QuoteTicks{ 2, 1, 1, 2, 0.0 }, 1, 20, 0, 3/*RV*/, 15/*BV*/, 0/*CT*/, 50/*WT*/} {
}

HFNeturalDlg::~HFNeturalDlg() {
}

void HFNeturalDlg::DoDataExchange(CDataExchange* pDX) {
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_HF_NETURAL_STRATEGY_LIST, m_strategyList);
	DDX_Control(pDX, IDC_MODE_COMBO, m_modeCombo);
	DDX_Control(pDX, IDC_MAJOR_COMBO, m_majorCombo);
	DDX_Control(pDX, IDC_HEDGE_COMBO, m_hedgeCombo);
	DDX_Control(pDX, IDC_OFR_BASE_TICK_EDIT, m_ofrBaseTickEdit);
	DDX_Control(pDX, IDC_OFR_GAIN_TICK_EDIT, m_ofrGainTickEdit);
	DDX_Control(pDX, IDC_BID_BASE_TICK_EDIT, m_bidBaseTickEdit);
	DDX_Control(pDX, IDC_BID_GAIN_TICK_EDIT, m_bidGainTickEdit);
	DDX_Control(pDX, IDC_RISK_QTY_EDIT, m_riskQtyEdit);
	DDX_Control(pDX, IDC_BREAK_QTY_EDIT, m_breakQtyEdit);
	DDX_Control(pDX, IDC_POSITION_EDIT, m_positionEdit);
	DDX_Control(pDX, IDC_ORDER_AMT_EDIT, m_orderQtyEdit);
	DDX_Control(pDX, IDC_HEDGE_TICK_EDIT, m_hedgeTickEdit);
	DDX_Control(pDX, IDC_WAITING_TICK_EDIT, m_waitingTickEdit);
	DDX_Control(pDX, IDC_BASIC_AMT_EDIT, m_basicAmtEdit);
	DDX_Control(pDX, IDC_QUOTE_LIST, m_quoteList);
}


BEGIN_MESSAGE_MAP(HFNeturalDlg, CDialogEx)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_ADD_BTN, &HFNeturalDlg::OnBnClickedAddBtn)
	ON_BN_CLICKED(IDC_MODIFY_BTN, &HFNeturalDlg::OnBnClickedModifyBtn)
	ON_CBN_SELCHANGE(IDC_MODE_COMBO, &HFNeturalDlg::OnCbnSelchangeModeCombo)
	ON_CBN_SELCHANGE(IDC_MAJOR_COMBO, &HFNeturalDlg::OnCbnSelchangeMajorCombo)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_HF_NETURAL_STRATEGY_LIST, &HFNeturalDlg::OnLvnItemchangedHfNeturalStrategyList)
	ON_NOTIFY(LVN_KEYDOWN, IDC_HF_NETURAL_STRATEGY_LIST, &HFNeturalDlg::OnLvnKeydownHfNeturalStrategyList)
END_MESSAGE_MAP()


// HFNetural 消息处理程序
void HFNeturalDlg::OnOK() {
}
void HFNeturalDlg::OnCancel() {
	BOOL bSwitch = m_modeCombo.IsWindowEnabled();
	m_modeCombo.EnableWindow(!bSwitch);
	m_basicAmtEdit.EnableWindow(!bSwitch);
}

BOOL HFNeturalDlg::OnInitDialog() {
	CDialogEx::OnInitDialog();

	InitCtrls();
	InitListCtrls();
	Update2Ctrls(m_arguments);

	{
		GetWindowRect(&m_windowRect);		//获取窗口大小
		m_oldPoint.x = m_windowRect.right - m_windowRect.left;
		m_oldPoint.y = m_windowRect.bottom - m_windowRect.top;
	}	//for size

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

void HFNeturalDlg::OnSize(UINT nType, int cx, int cy) {
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
			if (IDC_OPERATION_COMBO == ctrlId || IDC_MODE_COMBO == ctrlId || IDC_MAJOR_COMBO == ctrlId || IDC_HEDGE_COMBO == ctrlId) {
				NewBRPoint.y += 80;
			}
			Rect.SetRect(NewTLPoint, NewBRPoint);
			GetDlgItem(ctrlId)->MoveWindow(Rect, TRUE);
			hwndChild = ::GetWindow(hwndChild, GW_HWNDNEXT);
		}
		m_oldPoint = newPoint;
	}
}

void HFNeturalDlg::OnBnClickedAddBtn() {
	Update2Arguments();
	if (ParameterValidate()) {
		auto business = AddBusiness(m_arguments);
		m_businesses.push_back(business);		
		Framework::Instance().GetUIPublisher().AddBusiness(&*business);

		InsertStrategyList(business->GetId(), m_arguments);
		// OnLvnItemchangedHfNeturalStrategyList会被系统调用导致InitCrls会被调用，如果要保持上个参数要Update2Ctrls(m_arguments)
		Update2Ctrls(m_arguments);
	}
}
void HFNeturalDlg::OnBnClickedModifyBtn() {
	int index = m_strategyList.GetSelectionMark();
	if (-1 == index) return;

	Update2Arguments();
	if (ParameterValidate()) {
		StrategyCommand command;
		command.Type = StrategyCommand::UPDATE_ARGUMENT; 
		
		int businessID = int(m_strategyList.GetItemData(index));
		command.Business = nullptr;
		command.BusinessID = businessID;
		Arguments& argument = *(Arguments*)command.Data;
		argument = m_arguments;
		Framework::Instance().GetUIPublisher().Push(command);
		UpdateStrategyList(index, m_arguments);

		// OnLvnItemchangedHfNeturalStrategyList会被系统调用导致InitCrls会被调用，如果要保持上个参数要Update2Ctrls(m_arguments)
		Update2Ctrls(m_arguments);
	}
}
void HFNeturalDlg::OnCbnSelchangeModeCombo() {
	switch (m_modeCombo.GetCurSel()) {
	case 0: {
		m_arguments.Mode = RunningMode::OpenOnly;
		m_basicAmtEdit.EnableWindow(FALSE);
		m_positionEdit.EnableWindow(TRUE);
		break;
	}
	case 1: {
		m_arguments.Mode = RunningMode::Balanced;
		m_basicAmtEdit.EnableWindow(TRUE);
		m_positionEdit.EnableWindow(TRUE);
		break;
	}
	default: {
		m_arguments.Mode = RunningMode::OffsetOnly;
		m_positionEdit.EnableWindow(FALSE);
		m_basicAmtEdit.EnableWindow(FALSE);
		break;
	}
	}
}
void HFNeturalDlg::OnCbnSelchangeMajorCombo() {
	InitHedgeCombo(int(m_majorCombo.GetItemData(m_majorCombo.GetCurSel())));
}

void HFNeturalDlg::OnLvnKeydownHfNeturalStrategyList(NMHDR* pNMHDR, LRESULT* pResult) {
	LPNMLVKEYDOWN pLVKeyDow = reinterpret_cast<LPNMLVKEYDOWN>(pNMHDR);
	int index = m_strategyList.GetSelectionMark();
	if (-1 == index)
		return;
	ShortcutsHandler(index, pLVKeyDow->wVKey);
	*pResult = 0;
}
void HFNeturalDlg::OnLvnItemchangedHfNeturalStrategyList(NMHDR* pNMHDR, LRESULT* pResult) {
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	if ((pNMLV->uChanged & LVIF_STATE) && (pNMLV->uNewState & LVNI_SELECTED)) {	
		int businessID = int(m_strategyList.GetItemData(pNMLV->iItem));
		if (businessID > 0) {
			int itemIndex = m_itemIndexMap[businessID];
			m_arguments = m_hfArgumentsMap.at(itemIndex);
			Update2Ctrls(m_arguments);
			CWnd* enableWnds[] = { &m_majorCombo, &m_hedgeCombo, &m_modeCombo };
			CommonFunction::EnableWindows(enableWnds, CommonFunction::ArraySize(enableWnds), FALSE);
		}
	}
	else if (true) {
		InitCtrls();
	}
	*pResult = 0;
}

void HFNeturalDlg::PauseAll() {
	for (int i = 0; i <= m_strategyList.GetItemCount(); ++i) {
		StrategyCommand command;
		command.Type = StrategyCommand::PAUSE;
		command.BusinessID = i;
		command.Business = nullptr;
		Framework& framework = Framework::Instance();
		framework.GetUIPublisher().Push(command);
	}
}

void HFNeturalDlg::InitCtrls() {
	InitInstrumentCombos();
	int precision = 0;
	CommonFunction::Int2WindowText(m_ofrBaseTickEdit, m_arguments.Ticks.OfrBaseTick);
	CommonFunction::Int2WindowText(m_ofrGainTickEdit, m_arguments.Ticks.OfrGainTick);
	CommonFunction::Int2WindowText(m_bidGainTickEdit, m_arguments.Ticks.BidGainTick);
	CommonFunction::Int2WindowText(m_bidBaseTickEdit, m_arguments.Ticks.BidBaseTick);

	CommonFunction::Int2WindowText(m_riskQtyEdit, m_arguments.RiskQty);
	CommonFunction::Int2WindowText(m_breakQtyEdit, m_arguments.BreakQty);

	CommonFunction::Int2WindowText(m_basicAmtEdit, m_arguments.MinPosition);
	CommonFunction::Int2WindowText(m_positionEdit, m_arguments.MaxPosition);
	CommonFunction::Int2WindowText(m_orderQtyEdit, 0);	//通过单次委托量来控制初始状态不被启动
	CommonFunction::Int2WindowText(m_hedgeTickEdit, m_arguments.CostTick);
	CommonFunction::Int2WindowText(m_waitingTickEdit, m_arguments.WaitingTick);

	CWnd* enableWnds[] = { &m_majorCombo, &m_hedgeCombo };
	CommonFunction::EnableWindows(enableWnds, CommonFunction::ArraySize(enableWnds), TRUE);
}
void HFNeturalDlg::InitListCtrls() {
	{
		int column = 0;
		m_strategyList.InsertColumn(column++, _T("合约"), LVCFMT_LEFT, 65);
		m_strategyList.InsertColumn(column++, _T("模式"), LVCFMT_CENTER, 60);
		m_notifyColumn = column;
		m_strategyList.InsertColumn(column++, _T("头寸"), LVCFMT_CENTER, 120);
		m_strategyList.InsertColumn(column++, _T("统计"), LVCFMT_CENTER, 333);
		m_strategyList.InsertColumn(column++, _T("状态"), LVCFMT_RIGHT, 70);
		m_strategyList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP);
	}

	{
		int column = 0;
		m_quoteList.InsertColumn(column++, _T("卖"), LVCFMT_LEFT, 40);
		m_quoteList.InsertColumn(column++, _T("价格"), LVCFMT_CENTER, 75);
		m_quoteList.InsertColumn(column++, _T("买"), LVCFMT_RIGHT, 40);
		m_quoteList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP);
	}
}
void HFNeturalDlg::InitInstrumentCombos() {
	InitMajorCombo();
	InitHedgeCombo((InstID)m_majorCombo.GetItemData(0));
}
void HFNeturalDlg::InitMajorCombo() {
	m_majorCombo.ResetContent();
	Instruments const& instruments = InstrumentFactory::GetInstruments();
	for each (Instrument const& inst in instruments.GetAll()) {
		CString instCode(inst.GetInstCode().Str());
		int index = m_majorCombo.AddString(instCode);
		m_majorCombo.SetItemData(index, inst.GetID());
	}
	m_majorCombo.SetCurSel(0);
} 
void HFNeturalDlg::InitHedgeCombo(int majorInstID) {
	m_hedgeCombo.ResetContent();
	USES_CONVERSION;
	Instruments const& instruments = InstrumentFactory::GetInstruments();
	Instrument majorInst = instruments.Get(majorInstID);
	CString instCode = A2W(majorInst.GetVariety().GetCode().c_str());
	for each (Instrument const& inst in instruments.GetAll()) {
		if (0==instCode.Compare(A2W(inst.GetVariety().GetCode().c_str())) && inst.GetID()!= majorInst.GetID()) {
			CString contractCode(inst.GetInstCode().Str());
			int index = m_hedgeCombo.AddString(contractCode);
			m_hedgeCombo.SetItemData(index, inst.GetID());
		}
	}
	m_hedgeCombo.SetCurSel(0);
}

void HFNeturalDlg::Update2Ctrls(Arguments& arguments) {
	money unitPrice = InstrumentFactory::GetInstruments().Get(arguments.Instruments.Major).GetUnitPrice();
	int precision = CommonFunction::GetPricePrecision(unitPrice);
	CommonFunction::Int2WindowText(m_ofrBaseTickEdit, arguments.Ticks.OfrBaseTick);
	CommonFunction::Int2WindowText(m_ofrGainTickEdit, arguments.Ticks.OfrGainTick);
	CommonFunction::Int2WindowText(m_bidGainTickEdit, arguments.Ticks.BidGainTick);
	CommonFunction::Int2WindowText(m_bidBaseTickEdit, arguments.Ticks.BidBaseTick);

	CommonFunction::Int2WindowText(m_riskQtyEdit, arguments.RiskQty);
	CommonFunction::Int2WindowText(m_breakQtyEdit, arguments.BreakQty);

	CommonFunction::Int2WindowText(m_basicAmtEdit, arguments.MinPosition);
	CommonFunction::Int2WindowText(m_positionEdit, arguments.MaxPosition);
	CommonFunction::Int2WindowText(m_orderQtyEdit, arguments.UnitQty);
	CommonFunction::Int2WindowText(m_hedgeTickEdit, arguments.CostTick);
	CommonFunction::Int2WindowText(m_waitingTickEdit, arguments.WaitingTick);

	UpdateModeCombSel(arguments);
	UpdateInstrumentComboSel(m_majorCombo, arguments.Instruments.Major);
	InitHedgeCombo(int(m_majorCombo.GetItemData(m_majorCombo.GetCurSel())));
	UpdateInstrumentComboSel(m_hedgeCombo, arguments.Instruments.Hedge);
}
void HFNeturalDlg::Update2Arguments() {
	HFInstruments insts((InstID)m_majorCombo.GetItemData(m_majorCombo.GetCurSel()), (InstID)m_hedgeCombo.GetItemData(m_hedgeCombo.GetCurSel()));
	auto const& instrument = InstrumentFactory::GetInstruments().Get(insts.Major);
	money unitPrice = instrument.GetUnitPrice();

	QuoteTicks lines{ CommonFunction::WindowText2UINT32(m_ofrBaseTickEdit), CommonFunction::WindowText2UINT32(m_ofrGainTickEdit),
		CommonFunction::WindowText2UINT32(m_bidGainTickEdit), CommonFunction::WindowText2UINT32(m_bidBaseTickEdit), unitPrice};
	m_arguments = Arguments{ m_arguments.Mode, insts, lines, CommonFunction::WindowText2UINT8(m_basicAmtEdit), CommonFunction::WindowText2UINT8(m_positionEdit),
		CommonFunction::WindowText2UINT8(m_orderQtyEdit), CommonFunction::WindowText2UINT8(m_riskQtyEdit), CommonFunction::WindowText2UINT8(m_breakQtyEdit),
		CommonFunction::WindowText2UINT8(m_hedgeTickEdit), CommonFunction::WindowText2UINT8(m_waitingTickEdit) };
}
void HFNeturalDlg::UpdateModeCombSel(Arguments const& argument) {
	switch (argument.Mode) {
	case RunningMode::OpenOnly:
		m_modeCombo.SetCurSel(0);
		m_basicAmtEdit.EnableWindow(FALSE);
		break;
	case RunningMode::Balanced:
		m_modeCombo.SetCurSel(1);
		m_basicAmtEdit.EnableWindow(TRUE);
		break;
	case RunningMode::OffsetOnly:
		m_modeCombo.SetCurSel(2);
		m_basicAmtEdit.EnableWindow(FALSE);
		break;
	}
}
void HFNeturalDlg::UpdateInstrumentComboSel(CComboBox& combo, int InstID) {
	combo.SetCurSel(0);
	for (int i = 0; i < combo.GetCount(); ++i) {
		if ((int)combo.GetItemData(i) == InstID) {
			combo.SetCurSel(i);
			break;
		}
	}
}

std::shared_ptr<Business> HFNeturalDlg::AddBusiness(Arguments const& argument) {
	Framework& framework = Framework::Instance();
	return StrategyFactory::NewHFMarketing(this, argument, framework.GetStrategyPublisher());
}
void HFNeturalDlg::InsertStrategyList(int businessID, Arguments const& arguments) {
	HFInstruments const& HFInst = arguments.Instruments;
	Instruments instruments = InstrumentFactory::GetInstruments();
	std::string majorCode(instruments.Get(HFInst.Major).GetInstCode().Str());

	USES_CONVERSION;
	int count = m_strategyList.GetItemCount();
	int index = m_strategyList.InsertItem(count, A2W(fmt::format("{}", majorCode).c_str()));
	m_strategyList.SetSelectionMark(index);

	int column = 1;
	CString comboTxt;
	m_modeCombo.GetLBText(m_modeCombo.GetCurSel(), comboTxt);
	m_strategyList.SetItemText(index, column++, comboTxt);
	m_strategyList.SetItemText(index, column++, A2W("{0}[S:{0}-({0})-{0}:B]{0}"));
	m_strategyList.SetItemText(index, column++, A2W(fmt::format("{}", 0).c_str()));
	m_strategyList.SetItemText(index, column++, _T("New"));
	m_strategyList.SetItemData(index, (DWORD_PTR)businessID);
	m_itemIndexMap[businessID] = index;
	m_hfArgumentsMap.insert(std::make_pair(index, m_arguments));
}
void HFNeturalDlg::UpdateStrategyList(int itemIndex, Arguments const& arguments) {
	USES_CONVERSION;
	int column = 1;
	CString comboTxt;
	m_modeCombo.GetLBText(m_modeCombo.GetCurSel(), comboTxt);
	m_strategyList.SetItemText(itemIndex, column++, comboTxt);
// 	m_strategyList.SetItemText(itemIndex, column++, A2W(fmt::format("[{},{}]", arguments.MinPosition, arguments.MaxPosition).c_str()));
	m_hfArgumentsMap.at(itemIndex) = arguments;
}
void HFNeturalDlg::UpdateRvsPendingList(int instID, hf::netural::PriceBusinessManager const& businessMgr) {
	m_quoteList.DeleteAllItems();

	QuoteService& quote = WritableQuoteService();
	money unitPrice = quote.GetUnitPrice(instID);
	char const* priceFormat = CommonFunction::GetPriceFormat(unitPrice);

	USES_CONVERSION;
	int itemIndex = 0;
	int itemCount = 0;
	std::set<PendingTV> sellBook;
	std::set<PriceBusiness*>& buys = businessMgr.GetBuyBusinessSet();
	if (!buys.empty()) {
		for (PriceBusiness* business : buys) {
			if (business->IsReversePending()) {
				sellBook.insert({ business->GetPendingRvsTick(), business->GetPendingRvsQty() });
			}
		}
		if (!sellBook.empty()) {
			for (auto iter = sellBook.rbegin(); iter != sellBook.rend(); ++iter) {
				auto sellPrice = tickToPrice(iter->Tick, unitPrice);
				itemIndex = m_quoteList.InsertItem(itemCount++, A2W(fmt::format("{}", iter->Volume).c_str()));
				m_quoteList.SetItemText(itemIndex, 1, A2W(fmt::format(priceFormat, sellPrice.value()).c_str()));
				m_quoteList.SetItemText(itemIndex, 2, _T("0"));
			}
		}
	}

	money lastPrice = quote.GetLastPrice(instID);
	int lastPriceColumn = m_quoteList.InsertItem(itemCount++, _T("-"));
	m_quoteList.SetItemText(lastPriceColumn, 1, A2W(fmt::format(priceFormat, lastPrice.value()).c_str()));
	m_quoteList.SetItemText(lastPriceColumn, 2, _T("-"));

	std::set<PendingTV> buyBook;
	std::set<PriceBusiness*>& sells = businessMgr.GetSellBusinessSet();
	if (!sells.empty()) {
		for (PriceBusiness* business : sells) {
			if (business->IsReversePending()) {
				buyBook.insert({ business->GetPendingRvsTick(), business->GetPendingRvsQty() });
			}
		}
		if (!buyBook.empty()) {
			for (auto iter = buyBook.rbegin(); iter != buyBook.rend(); ++iter) {
				auto buyPrice = tickToPrice(iter->Tick, unitPrice);
				itemIndex = m_quoteList.InsertItem(itemCount++, _T("0"));
				m_quoteList.SetItemText(itemIndex, 1, A2W(fmt::format(priceFormat, buyPrice.value()).c_str()));
				m_quoteList.SetItemText(itemIndex, 2, A2W(fmt::format("{}", iter->Volume).c_str()));
			}
		}
	}

	m_quoteList.SetItemState(itemIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
}

void HFNeturalDlg::Update(Data const& data) {
	USES_CONVERSION;
	MarketingSnapshot const& updateData = *(MarketingSnapshot*)data.Data;
	int itemIndex = m_itemIndexMap[updateData.ID];
	if (itemIndex == -1)	return;
	
	NeturalPosition const& position = updateData.GlobalPosition;
	int totalSell = position.MajorVolume->TotalSell();
	int totalBuy = position.MajorVolume->TotalBuy();
	int BRvsCnt = updateData.BRvsCnt;
	int SRvsCnt = updateData.SRvsCnt;
	int pFactor = updateData.PFactor;
	m_strategyList.SetItemText(itemIndex, m_notifyColumn, A2W(fmt::format("{}[S:{}-({})-{}:B]{}", BRvsCnt, totalSell, pFactor, totalBuy, SRvsCnt).c_str()));		//持仓
	(updateData.Status != BusinessStatus::Paused) ? m_modeCombo.EnableWindow(FALSE) : m_modeCombo.EnableWindow(TRUE);	
	CString status(Desc(updateData.Status));
	money DBLSTDev = updateData.macroDBLStdev;
	m_strategyList.SetItemText(itemIndex, m_notifyColumn + 1, A2W(fmt::format("[S:{}|H:{}|G:{}|C:{}] [{:.2f}|{:.2f}|{:.2f}]",
		updateData.LossAmt, updateData.HedgeAmt, updateData.GainAmt, updateData.CancelAmt, 
		updateData.macroAvgPrice + DBLSTDev, updateData.macroAvgPrice, updateData.macroAvgPrice - DBLSTDev).c_str()));	// 统计信息
	m_strategyList.SetItemText(itemIndex, m_notifyColumn + 2, status);		//状态

	PriceBusinessManager const& businessBook = *updateData.businessManagerPtr;
	UpdateRvsPendingList(position.MajorVolume->InstId, businessBook);

	if (MAX_FUTURE_CANCEL_COUNT <= updateData.CancelAmt) {
		CString desc(fmt::format("撤单超过[{}],强制暂停策略并配平", MAX_FUTURE_CANCEL_COUNT).c_str());
		::AfxMessageBox(desc, MB_ICONWARNING);

		StrategyCommand command;
		command.Type = StrategyCommand::PAUSE;
		command.BusinessID = int(m_strategyList.GetItemData(itemIndex));
		command.Business = nullptr;
		Framework& framework = Framework::Instance();
		framework.GetUIPublisher().Push(command);
	}
}
bool HFNeturalDlg::ParameterValidate() {
	if (m_arguments.Mode == RunningMode::Balanced && (m_arguments.MinPosition < 0 || m_arguments.MinPosition >= m_arguments.MaxPosition)) {
		::AfxMessageBox(_T("基础持仓必须大于0且小于最大持仓!"), MB_ICONWARNING);
		m_basicAmtEdit.SetFocus();
		return false;
	}
	if (m_arguments.UnitQty < 1) {
		::AfxMessageBox(_T("委托数必须大于0!"), MB_ICONWARNING);
		m_orderQtyEdit.SetFocus();
		return false;
	}
	if (m_arguments.UnitQty > 1) {
		::AfxMessageBox(_T("当前版本只支持1手委托!"), MB_ICONWARNING);
		m_orderQtyEdit.SetFocus();
		return false;
	}
	if (m_arguments.UnitQty > m_arguments.MaxPosition) {
		::AfxMessageBox(_T("单次委托量必须小于最大持仓!"), MB_ICONWARNING);
		m_positionEdit.SetFocus();
		return false;
	}

	if (m_arguments.RiskQty > m_arguments.UnitQty*4) {
		if (IDNO == MessageBox(_T("风险敞口确定超过4倍委托量?"), AfxGetApp()->m_pszAppName, MB_YESNO | MB_ICONQUESTION)) {
			m_riskQtyEdit.SetFocus();
			return false;
		}
		if (m_arguments.RiskQty > m_arguments.MaxPosition / 2) {
			::AfxMessageBox(_T("风险敞口不能超过最大持仓的一半"), MB_ICONWARNING);
			m_riskQtyEdit.SetFocus();
			return false;
		}
	}

	if (0 == m_arguments.Ticks.OfrBaseTick) {
		if (IDNO == MessageBox(_T("OFR方向强制报价?"), AfxGetApp()->m_pszAppName, MB_YESNO | MB_ICONQUESTION)) {
			m_ofrBaseTickEdit.SetFocus();
			return false;
		}
	}
	if (0 == m_arguments.Ticks.BidBaseTick) {
		if (IDNO == MessageBox(_T("BID方向强制报价?"), AfxGetApp()->m_pszAppName, MB_YESNO | MB_ICONQUESTION)) {
			m_bidBaseTickEdit.SetFocus();
			return false;
		}
	}
	if (0 == m_arguments.Ticks.OfrBaseTick && 0 == m_arguments.Ticks.BidBaseTick) {
		::AfxMessageBox(_T("两个方向不可同时强制报价"), MB_ICONWARNING);
		m_ofrBaseTickEdit.SetFocus();
		return false;
	}

	if (m_arguments.Ticks.BidBaseTick < 0 || m_arguments.Ticks.BidGainTick < 0 ||
		m_arguments.Ticks.OfrBaseTick < 0 || m_arguments.Ticks.OfrGainTick < 0) {
		::AfxMessageBox(_T("报价参数不合法"), MB_ICONWARNING);
		m_ofrBaseTickEdit.SetFocus();
		return false;
	}
	if (m_arguments.BreakQty < 10) {
		::AfxMessageBox(_T("BV值不可小于10"), MB_ICONWARNING);
		m_riskQtyEdit.SetFocus();
		return false;
	}
	if (m_arguments.CostTick < 0 || m_arguments.CostTick > 5) {
		::AfxMessageBox(_T("CT值不合法![0,5]"), MB_ICONWARNING);
		m_hedgeTickEdit.SetFocus();
		return false;
	}
	if (m_arguments.WaitingTick < 10 || m_arguments.WaitingTick > 100) {
		::AfxMessageBox(_T("WT值不合法![10,100]"), MB_ICONWARNING);
		m_waitingTickEdit.SetFocus();
		return false;
	}
	return true;
}
void HFNeturalDlg::ShortcutsHandler(int index, WORD key) {
	StrategyCommand command;
	command.Type = GetCommandType(key);
	command.BusinessID = int(m_strategyList.GetItemData(index));
	command.Business = nullptr;

	Framework& framework = Framework::Instance();
	switch (command.Type) {
	case StrategyCommand::START: {
		auto statusTxt = m_strategyList.GetItemText(index, 4);
		if (0 != statusTxt.Compare(_T("Running"))) {
			if (IDYES == MessageBox(_T("开启报价?"), AfxGetApp()->m_pszAppName, MB_YESNO | MB_ICONQUESTION)) {
				framework.GetUIPublisher().Push(command);
			}
		}
		else {
			if (IDYES == MessageBox(_T("重新统计数据?"), AfxGetApp()->m_pszAppName, MB_YESNO | MB_ICONQUESTION)) {
				framework.GetUIPublisher().Push(command);
			}
		}
		break;
	}
	case StrategyCommand::PAUSE: {
		if (IDYES == MessageBox(_T("暂停并配平?"), AfxGetApp()->m_pszAppName, MB_YESNO | MB_ICONQUESTION)) {
			framework.GetUIPublisher().Push(command);
		}
		break;
	}
	default:
		break;
	}
}
int HFNeturalDlg::GetCommandType(WORD key) {
	switch (key) {
	case VK_F2:
		return StrategyCommand::PAUSE;
	case VK_F5:
		return StrategyCommand::START;
	default:
		return StrategyCommand::NONE;
	}
}
