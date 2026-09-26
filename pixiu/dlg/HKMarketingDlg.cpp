// Marketing.cpp: 实现文件
//

#include "pch.h"
#include "pixiu.h"
#include "HKMarketingDlg.h"
#include "afxdialogex.h"
#include "PostionDlg.h"

#include "model/factory/InstrumentFactory.h"
#include "runner/Framework.h"
#include "strategy/StrategyFactory.h"
#include "notify/UIPublisher.h"
#include "utils/Ctrl.h"
#include "utils/String.h"
#include "container/Utils.h"
// Marketing 对话框
using namespace hk;

int const NO_BALANCE_VALUE = 100;
IMPLEMENT_DYNAMIC(HKMarketingDlg, CDialogEx)

HKMarketingDlg::HKMarketingDlg(Config const& config, CWnd* pParent /*=nullptr*/)
	:CDialogEx(IDD_HK_MARKETING_DLG, pParent)
	,m_config(config), m_instFollowID(INVALID_INSTRUMENT_ID), m_hedgeID(INVALID_INSTRUMENT_ID)
	,m_arguments{ InstrumentModel::CME_GDU, MarketingInstIDs{0,1,2}, QuotingPriceModel::SellSafe, 0, 0, 0, 1, 2, 6, 1, 1, 0.01 } {
}


HKMarketingDlg::~HKMarketingDlg() {
}

void HKMarketingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_QUOTE_INST_COMBO, m_quoteInstCombo);
	DDX_Control(pDX, IDC_INST_COST_EDIT, m_instCostEdit);
	DDX_Control(pDX, IDC_QUALITY_COST_EDIT, m_qualityTickEdit);
	DDX_Control(pDX, IDC_SPREAD_EDIT, m_spreadEdit);
	DDX_Control(pDX, IDC_AMT_EDIT, m_quoteAmtEdit);
	DDX_Control(pDX, IDC_RISK_AMT_EDIT, m_riskPositionEdit);
	DDX_Control(pDX, IDC_MAX_AMT_EDIT, m_maxPositionEdit);
	DDX_Control(pDX, IDC_HF_MARKETING_STRATEGY_LIST, m_strategyList);
	DDX_Control(pDX, IDC_POS_EDIT, m_positionEdit);
	DDX_Control(pDX, IDC_SENSITIVITY_EDIT, m_sensitivityEdit);
	DDX_Control(pDX, IDC_MODEL_COMBO, m_modelCombo);
	DDX_Control(pDX, IDC_SGAIN_COMBO, m_SGainCombo);
	DDX_Control(pDX, IDC_BGAIN_COMBO, m_BGainCombo);
}
void HKMarketingDlg::OnOK() {
}
void HKMarketingDlg::OnCancel() {
}

// Marketing 消息处理程序
BEGIN_MESSAGE_MAP(HKMarketingDlg, CDialogEx)
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_ADD_BTN, &HKMarketingDlg::OnBnClickedAddBtn)
	ON_BN_CLICKED(IDC_MODIFY_BTN, &HKMarketingDlg::OnBnClickedModifyBtn)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_HF_MARKETING_STRATEGY_LIST, &HKMarketingDlg::OnCustomDrawMyList)
	ON_NOTIFY(LVN_KEYDOWN, IDC_HF_MARKETING_STRATEGY_LIST, &HKMarketingDlg::OnLvnKeydownIdcHfMarketingStrategyList)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_HF_MARKETING_STRATEGY_LIST, &HKMarketingDlg::OnLvnItemchangedIdcHfMarketingStrategyList)
	ON_CBN_SELCHANGE(IDC_MODEL_COMBO, &HKMarketingDlg::OnCbnSelchangeModelCombo)
	ON_CBN_SELCHANGE(IDC_QUOTE_INST_COMBO, &HKMarketingDlg::OnCbnSelchangeQuoteInstCombo)
END_MESSAGE_MAP()

BOOL HKMarketingDlg::OnInitDialog() {
	CDialogEx::OnInitDialog();

	InitInstrumentCombo();
	InitStrategyList();
	Update2Ctrls(m_arguments);
	
// 	SetTimer(10, 500, NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}
void HKMarketingDlg::OnSize(UINT nType, int cx, int cy) {
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
			if (IDC_QUOTE_INST_COMBO == ctrlId || IDC_QUOTE_DIR_COMBO == ctrlId || IDC_REF_INST_COMBO == ctrlId || IDC_REF_RATE_COMBO == ctrlId) {
				NewBRPoint.y += 80;
			}
			Rect.SetRect(NewTLPoint, NewBRPoint);
			GetDlgItem(ctrlId)->MoveWindow(Rect, TRUE);
			hwndChild = ::GetWindow(hwndChild, GW_HWNDNEXT);
		}
		m_oldPoint = newPoint;
	}
}
void HKMarketingDlg::OnTimer(UINT_PTR nIDEvent) {
	CTime const& time = CTime::GetCurrentTime();
	int minute = (7 == time.GetDayOfWeek()) ? 30 : 45;	//周六2：30暂停报价，其他时间2：45暂停报价
	if (2==time.GetHour() && time.GetMinute()>= minute) {
		PauseAll();
	}
	CDialogEx::OnTimer(nIDEvent);
}
void HKMarketingDlg::OnBnClickedAddBtn() {
// 	for (auto const& [k, v] : m_hkArgumentsMap) {
// 		if (v.InstIDs.Major == (InstID)m_quoteInstCombo.GetItemData(m_quoteInstCombo.GetCurSel()) &&
// 			0!=m_strategyList.GetItemText(k, 8).Compare(_T("Stop"))) {
// 			::AfxMessageBox(_T("无法添加重复合约"), MB_ICONWARNING);
// 			m_quoteInstCombo.SetFocus();
// 			return;
// 		}
// 	}

	if (ParameterValidate()) {
		Update2Arguments(false);	//增加business时都是非正式报价

		//如果有position的话，输入价位
		if (0 != m_arguments.Position) {
			PostionDlg dlg(m_arguments.Position, m_arguments.UnitPrice);
			if (IDOK == dlg.DoModal()) {
				m_arguments.Position = dlg.GetPositionAmt();
				m_arguments.InitPosition = dlg.GetPosition();
			} else {
				return;
			}
		}

		Framework& framework = Framework::Instance();

		auto business = AddBusiness(m_arguments);
		m_businesses.push_back(business);
		framework.GetUIPublisher().AddBusiness(&*business);

		InsertStrategyList(business->GetId(), m_arguments);
	}
}
void HKMarketingDlg::OnBnClickedModifyBtn() {
	int itemIndex = m_strategyList.GetSelectionMark();
	if (-1 == itemIndex) return;

	if (ParameterValidate()) {
		Update2Arguments(m_formalQuotingMap.at(itemIndex), itemIndex);

		StrategyCommand command;
		command.Type = StrategyCommand::UPDATE_ARGUMENT;
		int businessID = int(m_strategyList.GetItemData(itemIndex));
		command.Business = nullptr;
		command.BusinessID = businessID;
		Arguments& argument = *(Arguments*)command.Data;
		argument = m_arguments;
		Framework::Instance().GetUIPublisher().Push(command);

		UpdateStrategyList(itemIndex, m_arguments);
	}
}

void HKMarketingDlg::OnCbnSelchangeModelCombo() {
	switch (m_modelCombo.GetCurSel())
	{
	case 0:		//偏空,报价偏低
		m_BGainCombo.SetCurSel(0);
		m_SGainCombo.SetCurSel(1);
		break;
	case 2:		//偏多,报价偏高
		m_BGainCombo.SetCurSel(1);
		m_SGainCombo.SetCurSel(0);
		break;
	default:
		m_BGainCombo.SetCurSel(1);
		m_SGainCombo.SetCurSel(1);
	}
}
void HKMarketingDlg::OnCbnSelchangeQuoteInstCombo() {
	InstID quoteInstID = (InstID)m_quoteInstCombo.GetItemData(m_quoteInstCombo.GetCurSel());
	GetFollowInsts(quoteInstID);

	Instruments const& instruments = InstrumentFactory::GetInstruments();
	Instrument quoteInst = instruments.Get(quoteInstID);
	if (0 == quoteInst.GetVariety().GetCode().compare("GDU")) {
		CommonFunction::Int2WindowText(m_qualityTickEdit, 4);
	} else {
		CommonFunction::Int2WindowText(m_qualityTickEdit, 0);
	}
}
void HKMarketingDlg::OnCustomDrawMyList(NMHDR* pNMHDR, LRESULT* pResult) {
	NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>(pNMHDR);
	// Take the default processing unless we set this to something else below.  
	*pResult = CDRF_DODEFAULT;

	// First thing - check the draw stage. If it's the control's prepaint  
	// stage, then tell Windows we want messages for every item.  
	if (CDDS_PREPAINT == pLVCD->nmcd.dwDrawStage)
	{
		*pResult = CDRF_NOTIFYITEMDRAW;
	}
	else if (CDDS_ITEMPREPAINT == pLVCD->nmcd.dwDrawStage)
	{
		// This is the notification message for an item.  We'll request  
		// notifications before each subitem's prepaint stage.  
		*pResult = CDRF_NOTIFYSUBITEMDRAW;
	}
	else if ((CDDS_ITEMPREPAINT | CDDS_SUBITEM) == pLVCD->nmcd.dwDrawStage)
	{
		COLORREF clrNewBkColor = RGB(255, 255, 255);
		int nItem = static_cast<int>(pLVCD->nmcd.dwItemSpec);

		clrNewBkColor = RGB(200, 200, 169);	//初始颜色
		CString statusStr = m_strategyList.GetItemText(nItem, 8);
		if (0 == statusStr.Compare(_T("Running")))
		{
			clrNewBkColor = RGB(249, 205, 173);	//运行
		}
		else if (0 == statusStr.Compare(_T("Paused")))
		{
			clrNewBkColor = RGB(216, 220, 0);	//暂停(黄色)
		}
		else  if (0 == statusStr.Compare(_T("Stop")))
		{
			clrNewBkColor = RGB(205, 92, 92);	//停止(红色)
		}

		pLVCD->clrText = RGB(0, 90, 171);
		pLVCD->clrTextBk = clrNewBkColor;

		// Tell Windows to paint the control itself.  
		*pResult = CDRF_DODEFAULT;
	}
}
void HKMarketingDlg::OnLvnKeydownIdcHfMarketingStrategyList(NMHDR* pNMHDR, LRESULT* pResult) {
	LPNMLVKEYDOWN pLVKeyDow = reinterpret_cast<LPNMLVKEYDOWN>(pNMHDR);
	int index = m_strategyList.GetSelectionMark();
	if (-1 == index)	return;
	ShortcutsHandler(index, pLVKeyDow->wVKey);
	*pResult = 0;
}
void HKMarketingDlg::OnLvnItemchangedIdcHfMarketingStrategyList(NMHDR* pNMHDR, LRESULT* pResult) {
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	CWnd* enableWnds[] = { &m_quoteInstCombo, &m_positionEdit };
	if ((pNMLV->uChanged & LVIF_STATE) && (pNMLV->uNewState & LVNI_SELECTED)) {
		int businessID = int(m_strategyList.GetItemData(pNMLV->iItem));
		if (businessID > 0) {
			int itemIndex = m_itemIndexMap[businessID];
			Update2Ctrls(m_hkArgumentsMap.at(itemIndex), itemIndex);
			CommonFunction::EnableWindows(enableWnds, CommonFunction::ArraySize(enableWnds), FALSE);
		}
	} else if (true) {
		CWnd* enableWnds[] = { &m_quoteInstCombo, &m_positionEdit };
		CommonFunction::EnableWindows(enableWnds, CommonFunction::ArraySize(enableWnds), TRUE);
	}
	*pResult = 0;
}

void HKMarketingDlg::InitStrategyList() {
	int column = 0;
	m_strategyList.InsertColumn(column++, _T("报价合约"), LVCFMT_LEFT, 100);
	m_strategyList.InsertColumn(column++, _T("参考合约"), LVCFMT_CENTER, 90);
	m_strategyList.InsertColumn(column++, _T("偏向"), LVCFMT_CENTER, 50);
	m_strategyList.InsertColumn(column++, _T("成本"), LVCFMT_CENTER, 50);
	m_strategyList.InsertColumn(column++, _T("数量"), LVCFMT_CENTER, 50);
	m_strategyList.InsertColumn(column++, _T("间距"), LVCFMT_CENTER, 50);
	m_strategyList.InsertColumn(column++, _T("报价情况"), LVCFMT_CENTER, 180);
	m_strategyList.InsertColumn(column++, _T("持仓情况"), LVCFMT_CENTER, 125);
	m_strategyList.InsertColumn(column++, _T("状态"), LVCFMT_CENTER, 90);
	m_strategyList.InsertColumn(column++, _T("持仓明细"), LVCFMT_LEFT, 300);
	m_strategyList.InsertColumn(column++, _T("对冲明细"), LVCFMT_LEFT, 180);
	m_strategyList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP);
}
void HKMarketingDlg::InitInstrumentCombo() {
	m_quoteInstCombo.ResetContent();
	Instruments const& instruments = InstrumentFactory::GetInstruments();
	m_months = split(m_config.MMVarities.Months, ",");
	m_follows = split(m_config.MMVarities.Follows, ",");
	for (int i=0; i<m_months.size(); ++i) {
		m_followMap.emplace(m_months[i], fmt::format("GC{}",m_follows[i]));
	}
// 	auto instCode = fmt::format("{}-{}", inst.GetInstCode().Str(), m_followMap[inst.GetContractCode()]);
	for each (Instrument const& inst in instruments.GetAll()) {
		if (inst.GetVariety().GetCode() == m_config.MMVarities.Variety && contains(m_months,std::string(inst.GetContractCode()))) {
			CString instCode(inst.GetInstCode().Str());			
			int index = m_quoteInstCombo.AddString(instCode);
			m_quoteInstCombo.SetItemData(index, inst.GetID());
		}
	}
	m_quoteInstCombo.SetCurSel(0);
	GetFollowInsts((InstID)m_quoteInstCombo.GetItemData(0));
}

void HKMarketingDlg::GetFollowInsts(InstID instID) {
	Instruments const& instruments = InstrumentFactory::GetInstruments();
	Instrument quoteInst = instruments.Get(instID);
	std::string followCode = m_followMap[quoteInst.GetContractCode()];
	std::string hedgeCode = m_config.MMVarities.Hedge;

	for each (Instrument const& inst in instruments.GetAll()) {
		if ("GC" == inst.GetVariety().GetCode() && 0 == strcmp(hedgeCode.c_str(), inst.GetContractCode())) {
			m_hedgeID = inst.GetID();
		}
		if (0 == followCode.compare(inst.GetInstCode().Str())) {
			m_instFollowID = inst.GetID();
		}
// 		if ("GC" == inst.GetVariety().GetCode() && 0 == strcmp(quoteInst.GetContractCode(), inst.GetContractCode())) {
// 			m_instFollowID = inst.GetID();
// 		}
	}
}
void HKMarketingDlg::Update2Ctrls(Arguments const& argument, int itemIndex) {
	int position = (itemIndex >= 0) ? GetPositionFromList(itemIndex) : CommonFunction::WindowText2Int(m_positionEdit);

	CommonFunction::Int2WindowText(m_instCostEdit, argument.InstCostTick);
	CommonFunction::Int2WindowText(m_qualityTickEdit, argument.QualityTick);
	CommonFunction::Int2WindowText(m_spreadEdit, argument.SpreadTick);
	CommonFunction::Int2WindowText(m_positionEdit, position);
	CommonFunction::Int2WindowText(m_sensitivityEdit, argument.SensitivityTick);

	//尝试报价不修改报价量、风险持仓及最大持仓对话框
	if (itemIndex == -1/*初始化*/ || true == m_formalQuotingMap.at(itemIndex)/*正式报价*/) {
		CommonFunction::Int2WindowText(m_quoteAmtEdit, argument.QuoteQuantity);
		CommonFunction::Int2WindowText(m_riskPositionEdit, argument.RiskQuantity);
		CommonFunction::Int2WindowText(m_maxPositionEdit, argument.MaxQuantity);
	}

	UpdateInstComboSel(m_quoteInstCombo, argument.InstIDs.Major);
	m_modelCombo.SetCurSel((int)argument.PriceModel);
	//如果不平盘，combol设第四个参数（非常非常搓的实现，有空再改）zty
	(NO_BALANCE_VALUE == argument.SellGainTick) ? m_SGainCombo.SetCurSel(4) : m_SGainCombo.SetCurSel(argument.SellGainTick);
	(NO_BALANCE_VALUE == argument.BuyGainTick) ? m_BGainCombo.SetCurSel(4) : m_BGainCombo.SetCurSel(argument.BuyGainTick);

	if (argument.IsHedge) {
		m_SGainCombo.SetCurSel(5);
		m_BGainCombo.SetCurSel(5);
	} else {
		(NO_BALANCE_VALUE == argument.SellGainTick) ? m_SGainCombo.SetCurSel(4) : m_SGainCombo.SetCurSel(argument.SellGainTick);
		(NO_BALANCE_VALUE == argument.BuyGainTick) ? m_BGainCombo.SetCurSel(4) : m_BGainCombo.SetCurSel(argument.BuyGainTick);
	}
}
void HKMarketingDlg::Update2Arguments(bool isFormal, int itemIndex) {
	MarketingInstIDs InstIDs{ (InstID)m_quoteInstCombo.GetItemData(m_quoteInstCombo.GetCurSel()), m_instFollowID, m_hedgeID };
	Instrument quoteInst = InstrumentFactory::GetInstruments().Get(InstIDs.Major);
// 	Instrument followInst = InstrumentFactory::GetInstruments().Get(InstIDs.InstFollow);
	money unitPrice = quoteInst.GetUnitPrice();
	
	InstrumentModel model = InstrumentModel::CME_GDU;
	if ("GDU" == quoteInst.GetVariety().GetCode()) {
		model = InstrumentModel::CME_GDU;
	}
	
	//非正式报价，委托量、风险持仓及最大持仓都是1
	QtyT quotingAmt = isFormal ? CommonFunction::WindowText2UINT16(m_quoteAmtEdit) : 1;
	QtyT riskPosition = isFormal ? CommonFunction::WindowText2UINT16(m_riskPositionEdit) : 1;
	QtyT maxPosition = isFormal ? CommonFunction::WindowText2UINT16(m_maxPositionEdit) : 1;

	//第四挡选项则是不平盘 （非常非常搓的实现，有空再改）zty
	QtyT bGain = (4 == m_BGainCombo.GetCurSel() || 5 == m_BGainCombo.GetCurSel()) ?
		NO_BALANCE_VALUE : (QtyT)m_BGainCombo.GetCurSel();
	QtyT sGain = (4 == m_SGainCombo.GetCurSel() || 5 == m_SGainCombo.GetCurSel()) ? 
		NO_BALANCE_VALUE : (QtyT)m_SGainCombo.GetCurSel();
	//从list里拿business更新过的持仓
	int position = (itemIndex >= 0) ? GetPositionFromList(itemIndex) : CommonFunction::WindowText2Int(m_positionEdit);
	m_arguments = Arguments{ model, InstIDs, (QuotingPriceModel)m_modelCombo.GetCurSel(), position,
		CommonFunction::WindowText2Int(m_instCostEdit),
		CommonFunction::WindowText2Int(m_qualityTickEdit),
		CommonFunction::WindowText2UINT32(m_sensitivityEdit),
		CommonFunction::WindowText2UINT16(m_spreadEdit),
		quotingAmt,riskPosition,maxPosition,unitPrice,
		bGain, sGain, m_arguments.InitPosition
	};
	m_arguments.IsHedge = (5 == m_SGainCombo.GetCurSel() || 5 == m_BGainCombo.GetCurSel());
}
void HKMarketingDlg::UpdateInstComboSel(CComboBox& combo, int itemIndex) {
	combo.SetCurSel(0);
	for (int i = 0; i < combo.GetCount(); ++i) {
		if ((int)combo.GetItemData(i) == itemIndex) {
			combo.SetCurSel(i);
			GetFollowInsts((InstID)m_quoteInstCombo.GetItemData(i));
			break;
		}
	}
}

std::shared_ptr<Business> HKMarketingDlg::AddBusiness(Arguments const& argument) {
	Framework& framework = Framework::Instance();
	return StrategyFactory::NewHKMarketing(this, argument, framework.GetStrategyPublisher());
}

void HKMarketingDlg::InsertStrategyList(int businessID, Arguments const& arguments) {
	MarketingInstIDs const& InstIDs = arguments.InstIDs;
	Instruments instruments = InstrumentFactory::GetInstruments();
	std::string quoteCode(instruments.Get(InstIDs.Major).GetInstCode().Str());
	std::string refInstCode(instruments.Get(InstIDs.Follow).GetInstCode().Str());

	USES_CONVERSION;
	int column = 1;
	int count = m_strategyList.GetItemCount();
	int index = m_strategyList.InsertItem(count, A2W(fmt::format("{}", quoteCode).c_str()));
	m_strategyList.SetSelectionMark(index);
	m_strategyList.SetItemText(index, column++, A2W(fmt::format("{}", refInstCode).c_str()));
	m_strategyList.SetItemText(index, column++, A2W(fmt::format("{}", arguments.InstCostTick).c_str()));
	m_strategyList.SetItemText(index, column++, A2W(fmt::format("{}", arguments.QualityTick).c_str()));
	m_strategyList.SetItemText(index, column++, A2W(fmt::format("{}", arguments.QuoteQuantity).c_str()));
	m_strategyList.SetItemText(index, column++, A2W(fmt::format("{}", arguments.SpreadTick).c_str()));

	if (arguments.IsHedge) {
		m_strategyList.SetItemText(index, column++, A2W(fmt::format("M:{} SG:{} BG:{} B:{}", arguments.SensitivityTick, "@@", "@@", 0).c_str()));
	} else {
		m_strategyList.SetItemText(index, column++, A2W(fmt::format("M:{} SG:{} BG:{} B:{}", arguments.SensitivityTick, arguments.SellGainTick, arguments.BuyGainTick, 0).c_str()));
	}
	m_strategyList.SetItemText(index, column++, A2W(fmt::format("|{}| : |{}| : {}", arguments.MaxQuantity, arguments.RiskQuantity, arguments.Position).c_str()));
	m_strategyList.SetItemText(index, column++, _T("New"));

	m_strategyList.SetItemData(index, (DWORD_PTR)businessID);
	m_itemIndexMap[businessID] = index;
	m_hkArgumentsMap.insert(std::make_pair(index, m_arguments));
	m_formalQuotingMap.insert(std::make_pair(index, false));
}
void HKMarketingDlg::UpdateStrategyList(int itemIndex, Arguments const& arguments) {
	USES_CONVERSION;
	int column = 2;
	//特殊处理：尝试报价，报价量、风险持仓及最大持仓都是1
	QtyT quoteQuantity = m_formalQuotingMap.at(itemIndex) ? arguments.QuoteQuantity : 1;
	QtyT riskPosition = m_formalQuotingMap.at(itemIndex) ? arguments.RiskQuantity : 1;
	QtyT maxPosition = m_formalQuotingMap.at(itemIndex) ? arguments.MaxQuantity : 1;
	m_strategyList.SetItemText(itemIndex, column++, A2W(fmt::format("{}", arguments.InstCostTick).c_str()));
	m_strategyList.SetItemText(itemIndex, column++, A2W(fmt::format("{}", arguments.QualityTick).c_str()));
	m_strategyList.SetItemText(itemIndex, column++, A2W(fmt::format("{}", quoteQuantity).c_str()));
	m_strategyList.SetItemText(itemIndex, column++, A2W(fmt::format("{}", arguments.SpreadTick).c_str()));
	int biasTick = GetBiasTickFromList(itemIndex);
	if (arguments.IsHedge) {
		m_strategyList.SetItemText(itemIndex, column++, A2W(fmt::format("M:{} SG:{} BG:{} B:{}", arguments.SensitivityTick, "@@", "@@", biasTick).c_str()));
	}
	else {
		m_strategyList.SetItemText(itemIndex, column++, A2W(fmt::format("M:{} SG:{} BG:{} B:{}", arguments.SensitivityTick, arguments.SellGainTick, arguments.BuyGainTick, biasTick).c_str()));
	}
	m_strategyList.SetItemText(itemIndex, column++, A2W(fmt::format("|{}| : |{}| : {}", maxPosition, riskPosition, arguments.Position).c_str()));
	m_hkArgumentsMap.at(itemIndex) = arguments;
}
// void HKMarketingDlg::UpdateQuotePriceItem(int itemIndex, Arguments const& arguments) {
// 	QuoteService& quote = WritableQuoteService();
// 	money sellPrice = quote.GetSellPrice(arguments.InstIDs.Major);
// 	money buyPrice = quote.GetBuyPrice(arguments.InstIDs.Major);
// 	Quote const& instQuote = quote.GetQuote(arguments.InstIDs.InstFollow);
// 	money refSellPrice = instQuote.GetSell().GetPrice();
// 	money refBuyPrice = instQuote.GetBuy().GetPrice();
// 	money refRatePrice = quote.GetBuyPrice(arguments.InstIDs.RateFollow);
// 	SimpleQuote hidenBuy = instQuote.GetHidenBuy();
// 	SimpleQuote hidenSell = instQuote.GetHidenSell();
// 
// 	if (hidenBuy.GetQuantity() > 0 && refBuyPrice < hidenBuy.GetPrice()) {
// 		refBuyPrice = hidenBuy.GetPrice();
// 	}
// 	if (hidenSell.GetQuantity() > 0 && refSellPrice > hidenSell.GetPrice()) {
// 		refSellPrice = hidenSell.GetPrice();
// 	}
// 
// 	USES_CONVERSION;
// 	money unitPrice = InstrumentFactory::GetInstruments().Get(arguments.InstIDs.Major).GetUnitPrice();
// 	char const* deltaFormat = CommonFunction::GetDeltaFormat(unitPrice);
// 	if (0 != sellPrice.value() && 0 != buyPrice.value()) {
// 		m_strategyList.SetItemText(itemIndex, 10, A2W(fmt::format(deltaFormat, buyPrice.value(), sellPrice.value()).c_str()));
// 	}
// 
// 	money refUnitPirice = InstrumentFactory::GetInstruments().Get(arguments.InstIDs.InstFollow).GetUnitPrice();
// 	char const* refFormat = CommonFunction::GetDeltaFormat(refUnitPirice);
// 	if (0 != refSellPrice.value() && 0 != refBuyPrice.value()) {
// 		m_strategyList.SetItemText(itemIndex, 11, A2W(fmt::format(refFormat, refBuyPrice.value(), refSellPrice.value()).c_str()));
// 	}
// 	if (0 != refRatePrice.value()) {
// 		m_strategyList.SetItemText(itemIndex, 12, A2W(fmt::format("{:.4f}", refRatePrice.value()).c_str()));
// 	}
// }
void HKMarketingDlg::Update(Data const& data) {
	hk::Snapshot const& updateData = *(hk::Snapshot*)data.Data;
	int itemIndex = m_itemIndexMap[updateData.ID];
	if (itemIndex == -1)	return;

	USES_CONVERSION;
	int column = 6;
	Arguments const& arguments = m_hkArgumentsMap.at(itemIndex);
	if (arguments.IsHedge) {
		m_strategyList.SetItemText(itemIndex, column++, A2W(fmt::format("M:{} SG:{} BG:{} B:{}", arguments.SensitivityTick, "@@", "@@", (int)updateData.BiasTick).c_str()));
	}
	else {
		m_strategyList.SetItemText(itemIndex, column++, A2W(fmt::format("M:{} SG:{} BG:{} B:{}", arguments.SensitivityTick, arguments.SellGainTick, arguments.BuyGainTick, (int)updateData.BiasTick).c_str()));
	}
	m_strategyList.SetItemText(itemIndex, column++, A2W(fmt::format("|{}| : |{}| : {}", arguments.MaxQuantity, arguments.RiskQuantity, updateData.Position).c_str()));

	CString status(Desc(updateData.Status));
	m_strategyList.SetItemText(itemIndex, column++, status);
	m_strategyList.SetItemText(itemIndex, column++, A2W(updateData.PosInfo.Str()));
	m_strategyList.SetItemText(itemIndex, column++, A2W(updateData.HedgeInfo.Str()));
}

bool HKMarketingDlg::ParameterValidate() {
	int instCostTick = CommonFunction::WindowText2Int(m_instCostEdit);
	if (instCostTick>100 || instCostTick<-100) {
		::AfxMessageBox(_T("报价偏向范围在[-100,100]之间!"), MB_ICONWARNING);
		m_instCostEdit.SetFocus();
		return false;
	}

	TickT sensitivityTick = CommonFunction::WindowText2UINT32(m_sensitivityEdit);
	if (sensitivityTick > 2) {
		::AfxMessageBox(_T("M参数不得超过2"), MB_ICONWARNING);
		m_sensitivityEdit.SetFocus();
		return false;
	}

	int position = CommonFunction::WindowText2Int(m_positionEdit);
	if ((instCostTick>0 && position>0) || (instCostTick<0 && position<0)) {
		if (IDNO == MessageBox(_T("报价偏向确定与持仓方向相同?"), AfxGetApp()->m_pszAppName, MB_YESNO | MB_ICONQUESTION)) {
			m_instCostEdit.SetFocus();
			return false;
		}
	}

	TickT spreadTick = CommonFunction::WindowText2UINT32(m_spreadEdit);
	if (spreadTick < 1) {
		::AfxMessageBox(_T("报价间距不得小于1跳!"), MB_ICONWARNING);
		m_spreadEdit.SetFocus();
		return false;
	}

	QtyT quantity = CommonFunction::WindowText2UINT16(m_quoteAmtEdit);
	if (quantity<0 || quantity>50) {
		::AfxMessageBox(_T("报价量必须在1到50手之间"), MB_ICONWARNING);
		m_quoteAmtEdit.SetFocus();
		return false;
	}
	
	QtyT maxQuantity = CommonFunction::WindowText2UINT16(m_maxPositionEdit);
	QtyT riskQuantity = CommonFunction::WindowText2UINT16(m_riskPositionEdit);
	if (riskQuantity<0 || riskQuantity>100 || riskQuantity>maxQuantity) {
		::AfxMessageBox(_T("风险头寸必须在1到100手之间，且不大于最大持仓!"), MB_ICONWARNING);
		m_riskPositionEdit.SetFocus();
		return false;
	}
	if (abs(position) > maxQuantity) {
		::AfxMessageBox(_T("现有持仓不得大于最大持仓!"), MB_ICONWARNING);
		m_positionEdit.SetFocus();
		return false;
	}

	return true;
}
void HKMarketingDlg::ShortcutsHandler(int index, WORD key) {
	StrategyCommand command;
	command.Type = GetCommandType(key);
	command.BusinessID = int(m_strategyList.GetItemData(index));
	command.Business = nullptr;


	Framework& framework = Framework::Instance();
	switch (command.Type) {
	case StrategyCommand::START: {
		if (0 == m_strategyList.GetItemText(index, 9).Compare(_T("Stop"))) {
			::AfxMessageBox(_T("无法改变当前状态!"), MB_ICONWARNING);
			return;
		}

		bool isFormal = m_formalQuotingMap.at(index);
		if (false==m_formalQuotingMap.at(index) && IDYES == MessageBox(_T("开始正式报价?"), AfxGetApp()->m_pszAppName, MB_YESNO | MB_ICONQUESTION)) {
			m_formalQuotingMap.at(index) = true;		//先标记为正式报价
			OnBnClickedModifyBtn();
		}
		framework.GetUIPublisher().Push(command);
		break;
	}
	case StrategyCommand::PAUSE: {
		if (0 == m_strategyList.GetItemText(index, 9).Compare(_T("Stop"))) {
			::AfxMessageBox(_T("无法改变当前状态!"), MB_ICONWARNING);
			return;
		}

		if (IDYES == MessageBox(_T("暂停报价,继续平盘?"), AfxGetApp()->m_pszAppName, MB_YESNO | MB_ICONQUESTION)) {
			framework.GetUIPublisher().Push(command);
		}
		break;
	}
	case StrategyCommand::STOP: {
		if (0 == m_strategyList.GetItemText(index, 9).Compare(_T("Stop"))) {
			return;
		}

		if (IDYES == MessageBox(_T("确定停止策略?"), AfxGetApp()->m_pszAppName, MB_YESNO | MB_ICONQUESTION)) {
			framework.GetUIPublisher().Push(command);
		}
		break;
	}
	default:
		break;
	}
}
int HKMarketingDlg::GetCommandType(WORD key) {
	switch (key) {
	case VK_F2:
		return StrategyCommand::PAUSE;
	case VK_F4:
		return StrategyCommand::STOP;
	case VK_F5:
		return StrategyCommand::START;
	default:
		return StrategyCommand::NONE;
	}
}
void HKMarketingDlg::PauseAll() {
	for (int i=0; i<=m_strategyList.GetItemCount(); ++i){
		StrategyCommand command;
		command.Type = StrategyCommand::PAUSE;
		command.BusinessID = i;
		command.Business = nullptr;
		Framework& framework = Framework::Instance();
		framework.GetUIPublisher().Push(command);
	}
}
int HKMarketingDlg::GetPositionFromList(int index) {
	USES_CONVERSION;
	CString positionStr = m_strategyList.GetItemText(index, 7);
	int begPos = positionStr.ReverseFind(':');
	int endPos = positionStr.GetLength();
	positionStr = positionStr.Right(endPos - begPos - 1);
	return atoi(W2A(positionStr.GetBuffer()));
}
int HKMarketingDlg::GetBiasTickFromList(int index) {
	USES_CONVERSION;
	CString positionStr = m_strategyList.GetItemText(index, 6);
	int begPos = positionStr.ReverseFind(':');
	int endPos = positionStr.GetLength();
	positionStr = positionStr.Right(endPos - begPos - 1);
	return atoi(W2A(positionStr.GetBuffer()));
}
