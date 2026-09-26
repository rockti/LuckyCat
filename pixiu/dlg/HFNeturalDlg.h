#pragma once
#include "strategy/hf_marketing/NeturalSpreadStrategy.h"
#include "notify/Notify.h"
// HFNetural 对话框

class HFNeturalDlg : public CDialogEx, UIUpdater
{
	DECLARE_DYNAMIC(HFNeturalDlg)

public:
	HFNeturalDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~HFNeturalDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_HF_NETURAL_DLG };
#endif

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();

	CRect m_windowRect;
	POINT m_oldPoint;

public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedAddBtn();
	afx_msg void OnBnClickedModifyBtn();
	afx_msg void OnCbnSelchangeModeCombo();
	afx_msg void OnCbnSelchangeMajorCombo();
	afx_msg void OnLvnKeydownHfNeturalStrategyList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLvnItemchangedHfNeturalStrategyList(NMHDR* pNMHDR, LRESULT* pResult);

	void PauseAll();
private:
	void InitCtrls();
	void InitListCtrls();
	void InitInstrumentCombos();
	void InitMajorCombo();
	void InitHedgeCombo(int majorID);
	void Update2Ctrls(hf::netural::Arguments& arguments);
	void Update2Arguments();
	void UpdateModeCombSel(hf::netural::Arguments const& argument);
	void UpdateInstrumentComboSel(CComboBox& combo, int InstID);

	std::shared_ptr<Business> AddBusiness(hf::netural::Arguments const& argument);
	void InsertStrategyList(int businessID, hf::netural::Arguments const& arguments);
	void UpdateStrategyList(int itemIndex, hf::netural::Arguments const& arguments);
	void UpdateRvsPendingList(int instID, hf::netural::PriceBusinessManager const& businessMgr);

	void Update(Data const& data);

	bool ParameterValidate();
	void ShortcutsHandler(int index, WORD key);
	int GetCommandType(WORD key);

private:
	CListCtrl m_strategyList;
	CListCtrl m_quoteList;
	CComboBox m_modeCombo;
	CComboBox m_majorCombo;
	CComboBox m_hedgeCombo;
	CEdit m_basicAmtEdit;		// 基本量，只有平衡模式下有效
	CEdit m_ofrBaseTickEdit;
	CEdit m_ofrGainTickEdit;
	CEdit m_bidBaseTickEdit;
	CEdit m_bidGainTickEdit;
	CEdit m_riskQtyEdit;		// 风险敞口
	CEdit m_breakQtyEdit;		// 放量阈值
	CEdit m_positionEdit;
	CEdit m_orderQtyEdit;
	CEdit m_hedgeTickEdit;
	CEdit m_waitingTickEdit;	// 时间窗口

	
private:	
	int m_notifyColumn = 0;
	hf::netural::Arguments m_arguments;
	std::map<int, int> m_itemIndexMap;			// <businessID, itemIndex>
	std::map<int, hf::netural::Arguments> m_hfArgumentsMap;	//<itemIndex, hfArguments>
	std::vector<std::shared_ptr<Business>> m_businesses;	// 为了自动释放指针
};
