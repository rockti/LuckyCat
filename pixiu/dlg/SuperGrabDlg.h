#pragma once
#include "strategy/grab/GrabStrategy.h"
#include "notify/Notify.h"


// SuperGrab 对话框
class SuperGrabDlg : public CDialogEx, UIUpdater {
	DECLARE_DYNAMIC(SuperGrabDlg)

public:
	SuperGrabDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~SuperGrabDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TG_GRAB_DLG };
#endif

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void OnOK();
	virtual void OnCancel();

	POINT m_oldPoint;
	CRect m_windowRect;

public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedAddGrab();
	afx_msg void OnBnClickedMdfGrab();
	afx_msg void OnBnClickedInstCheck();
	afx_msg void OnBnClickedHoldCheck1();
	afx_msg void OnBnClickedHoldCheck2();
	afx_msg void OnCbnSelchangeCombo1();
	afx_msg void OnLvnKeydownGrabStrategyList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLvnItemchangedGrabStrategyList(NMHDR* pNMHDR, LRESULT* pResult);

	void PauseAll() {};
private:
	void InitCtrls();
	void InitListCtrls();
	void InitCombo1Data();
	void InitCombo2Data(int instID);
	void InitInstCombos();
	void InitStartTimeCtrls(SYSTEMTIME const& startTime);

	void Update2Ctrls(tg::grab::Arguments& arguments);
	void Update2Arguments();
	void UpdateTimeComboSel(CComboBox& combo, int value);
	void UpdateInstComboSel(CComboBox& combo, int InstID);

	std::shared_ptr<Business> AddBusiness(tg::grab::Arguments const& argument);
	void InsertStrategyList(int businessID, tg::grab::Arguments const& arguments);
	void UpdateStrategyList(int itemIndex, tg::grab::Arguments const& arguments);

	bool ParameterValidate();
	TSGEProductClassType GetProductClass(InstID instID);

	int GetCommandType(WORD key);
	void ShortcutsHandler(int index, WORD key);

	void Update(Data const& data);

private:
	CListCtrl m_businessList;
	CComboBox m_combo1;
	CComboBox m_combo2;
	CComboBox m_hourCombo;
	CComboBox m_minuteCombo;
	CComboBox m_secondCombo;
	CButton m_instCheck;
	CButton m_holdCheck1;
	CButton m_holdCheck2;
	CEdit m_opPrice1;
	CEdit m_opPrice2;
	CEdit m_stPrice1;
	CEdit m_stPrice2;
	CEdit m_openQty1;
	CEdit m_openQty2;
	CEdit m_ofPrice1;
	CEdit m_ofPrice2;
	CEdit m_holdQty1;
	CEdit m_holdQty2;
	CEdit m_msEdit;
	CEdit m_exPeriod;
	CEdit m_awakeAmt;
	CEdit m_period;
	CEdit m_totalAmt;

	CToolTipCtrl m_toolTips;

private:
	int m_notifyColumn = 0;
	SYSTEMTIME m_startTime;
	tg::grab::Arguments m_arguments;
	std::map<int, int> m_itemIndexMap;						// <businessID, itemIndex>
	std::map<int, tg::grab::Arguments> m_gbArgumentsMap;	// <itemIndex, gbArguments>
	std::vector<std::shared_ptr<Business>> m_businesses;	// 保存只能指针，防止被自动释放
};
