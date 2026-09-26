
// pixiuDlg.h: 头文件
//

#pragma once
#include "dlg/DialogTabCtrl.h"
#include "config/Config.h"
#include "strategy/Business.h"
#include "notify/Notify.h"
#include "dlg/HKMarketingDlg.h"
#include "dlg/HFNeturalDlg.h"
#include "dlg/HFSpreadDlg.h"
#include "dlg/SuperGrabDlg.h"

class CColorBtn : public CButton {
	DECLARE_DYNAMIC(CColorBtn)

public:
	CColorBtn(COLORREF bgColor):m_bPressed(FALSE),m_bgColor(bgColor),m_textColor(RGB(0,0,0)) {
	}
	virtual ~CColorBtn() {}

	void SetBkColor(COLORREF color) { m_bgColor = color; }
private:
	BOOL m_bPressed;
	COLORREF m_bgColor;
	COLORREF m_textColor;

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) {
		CRect rect;
		GetClientRect(rect);
		CDC dc;
		dc.Attach(lpDrawItemStruct->hDC);

		UINT state = lpDrawItemStruct->itemState;
		CRect focusRect(rect);
		focusRect.DeflateRect(1, 1, 1, 1);
		if ((state & ODS_SELECTED) || (state & ODS_FOCUS)) {
			CPen pen(PS_DASHDOTDOT, 1, RGB(0, 0, 0));
			CBrush brush;
			brush.CreateStockObject(NULL_BRUSH);
			dc.SelectObject(&brush);
			dc.SelectObject(&pen);
			dc.FillSolidRect(rect, m_bgColor);
			dc.DrawFocusRect(focusRect);
			dc.Draw3dRect(rect, RGB(0, 0, 255), RGB(0, 0, 255));
		} else {
			CPen pen(PS_DOT, 1, m_bgColor);
			CBrush brush;
			brush.CreateStockObject(NULL_BRUSH);
			dc.SelectObject(&brush);
			dc.SelectObject(&pen);
			dc.FillSolidRect(rect, m_bgColor);
			dc.Rectangle(focusRect);
		}
		dc.DrawEdge(rect, BDR_RAISEDINNER | BDR_SUNKENOUTER, BF_BOTTOMLEFT | BF_TOPRIGHT);

		if (m_bPressed) {
			dc.DrawFocusRect(focusRect);
			dc.DrawEdge(rect, BDR_RAISEDINNER | BDR_SUNKENOUTER, BF_BOTTOMLEFT | BF_TOPRIGHT);
			dc.Draw3dRect(rect, RGB(0, 0, 255), RGB(0, 0, 255));
		}

		CString strText;
		GetWindowText(strText);
		dc.SetBkMode(TRANSPARENT);
		dc.SetTextColor(m_textColor);
		if (state & ODS_DISABLED) {
			dc.SetTextColor(RGB(0, 0, 0));
		}
		dc.DrawText(strText, rect, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
		dc.Detach();
	}
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point) {
		m_bPressed = TRUE;
		CButton::OnLButtonDown(nFlags, point);
	}
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point) {
		m_bPressed = FALSE;
		CButton::OnLButtonDown(nFlags, point);
	}
};


class Framework;
class Instrument;
class Dealing;
// CpixiuDlg 对话框
class CpixiuDlg : public CDialogEx, UIUpdater
{
// 构造
public:
	CpixiuDlg(Config& config, CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PIXIU_DIALOG };
#endif

// 实现
protected:
	HICON m_hIcon;
	CDialogTabCtrl m_dialogTabCtrl;

	// 生成的消息映射函数
	virtual void OnOK();
	virtual void OnCancel();
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnDeltaposPriceSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposBuySpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSellSpin(NMHDR* pNMHDR, LRESULT* pResult);

	afx_msg void OnClose();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	afx_msg void OnCbnSelchangePlatformCombo();
	afx_msg void OnCbnSelchangeInstrumentCombo();
	afx_msg void OnBnClickedSetFundsBtn();
	afx_msg void OnBnClickedFuturesLoginBtn();
	afx_msg void OnBnClickedDeferSpotLoginBtn();
	afx_msg void OnBnClickedOverseaLoginBtn();

	afx_msg void OnBnClickedShfeRadio();
	afx_msg void OnBnClickedDceRadio();
	afx_msg void OnBnClickedCzceRadio();
	afx_msg void OnBnClickedCffexRadio();
	afx_msg void OnBnClickedSgeRadio();
	afx_msg void OnBnClickedHkexRadio();

	afx_msg void OnBnClickedBopenBtn();
	afx_msg void OnBnClickedSopenBtn();
	afx_msg void OnBnClickedBoffsetBtn();
	afx_msg void OnBnClickedSoffsetBtn();

	afx_msg void OnBnClickedQryPositionBtn();

	afx_msg void OnNMDblclkDealingList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMDblclkPositionList(NMHDR* pNMHDR, LRESULT* pResult);

private:
	void InitStrategyPaneSet();
	void InitCtrls();
	void InitDealingList();
	void InitPositionList();
	void InitComboData(Exchange exchange);
	void AddMTCBusiness();

	void OnNotify(Data const& data);
	void OnInfoNotification(CString const& info);
	void OnExchangeNotification(ExchangeNotification const& notification);
	void NotifyByPlatform(UINT quoteID, UINT tradeID, UINT statusID, ExchangeNotification const& notification);
	void OnAccountUpdate(PlatformAccount const& notification);
	void OnPositionNotification(PositionNotification const& notification);
	void OnDealing(Dealing const& dealing);

	bool FindDealingFromList(CString const& systemNo, int& pos);
	bool FindPositionFromList(InstID instID, Direction direction, int& pos);
	void OnTradeDetail(OrderNotification const& notification);		// 成交明细更新界面持仓
	void UpdatePositionList(PositionNotification notification);		// 持仓查询更新界面持仓	

	void ChangeCtlColor(CDC* pDc, int ctrlID);
	void ShowOpenBtn(BOOL bEnable);
	void ShowOffsetBtn(BOOL enableSpin, BOOL isHK=FALSE);
	void SwitchExchRadio(UINT RadioID);

	bool ParameterValidate();
	InstID GetInstID(CString instCode);
	void UpdateQuote();

	void Update(Data const& data);
private:
	CRect m_windowRect;
	POINT m_oldPoint;
	POINT m_initPoint;

	CListCtrl m_logList;
	CListCtrl m_dealingList;
	CListCtrl m_positionList;
	CComboBox m_platformCombo;
	CComboBox m_instrumentCombo;
	CEdit m_amtEdit;
	CEdit m_priceEdit;
	CColorBtn m_bOpenBtn;
	CColorBtn m_sOpenBtn;
	CColorBtn m_bOffsetBtn;
	CColorBtn m_sOffsetBtn;
	CSpinButtonCtrl m_priceSpin;
	CSpinButtonCtrl m_buyOffsetSpin;
	CSpinButtonCtrl m_sellOffsetSpin;
	HFNeturalDlg* m_neturalDlg = nullptr;
	HFSpreadDlg* m_spreadDlg = nullptr;
	SuperGrabDlg* m_grabDlg = nullptr;

private:
	Config& m_config;
	Framework& m_framework;
	std::shared_ptr<Business> m_businessPtr;
	std::map<ConnPlatform, UINT> m_platformIDMap;
	std::map<ExchangeStatus, std::string> m_statusInfoMap;
	std::set<ConnPlatform> m_loginedPlatform;		// 已登录的平台
	ExchangeStatus m_currentStatus = ExchangeStatus::UNKNOW;
};
