#pragma once
#include "strategy/hk_marketing/HKMarketingStrategy.h"
#include "notify/Notify.h"
#include "config/Config.h"



using namespace hk;

// Marketing 对话框

class HKMarketingDlg : public CDialogEx, UIUpdater
{
	DECLARE_DYNAMIC(HKMarketingDlg)

public:
	HKMarketingDlg(Config const& config, CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~HKMarketingDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_HK_MARKETING_DLG };
#endif

protected:
	DECLARE_MESSAGE_MAP()
	virtual void OnOK();
	virtual void OnCancel();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();

	POINT m_oldPoint;

public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedAddBtn();
	afx_msg void OnBnClickedModifyBtn();
	afx_msg void OnCbnSelchangeModelCombo();
	afx_msg void OnCbnSelchangeQuoteInstCombo();
	afx_msg void OnCustomDrawMyList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLvnKeydownIdcHfMarketingStrategyList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLvnItemchangedIdcHfMarketingStrategyList(NMHDR* pNMHDR, LRESULT* pResult);

	void PauseAll();
private:
	void InitStrategyList();
	void InitInstrumentCombo();

	void GetFollowInsts(InstID instID);		//get GC && CUS
	void Update2Ctrls(hk::Arguments const& argument, int itemIndex=-1);
	void Update2Arguments(bool isFormal, int itemIndex=-1);
	void UpdateInstComboSel(CComboBox& combo, int itemIndex);

	std::shared_ptr<Business> AddBusiness(hk::Arguments const& argument);

	void InsertStrategyList(int businessID, hk::Arguments const& arguments);
	void UpdateStrategyList(int itemIndex, hk::Arguments const& arguments);
	void Update(Data const& data);

	bool ParameterValidate();
	void ShortcutsHandler(int index, WORD key);
	int GetCommandType(WORD key);
	int GetPositionFromList(int index);
	int GetBiasTickFromList(int index);

private:
	CComboBox m_quoteInstCombo;		//报价合约
	CComboBox m_modelCombo;			//报价模式{偏卖、中性、偏买}
	CComboBox m_SGainCombo;			//卖盈点数
	CComboBox m_BGainCombo;			//买盈点数

	CEdit m_instCostEdit;			//合约成本
	CEdit m_qualityTickEdit;		//成色加点（直接加减报价点）
	CEdit m_quoteAmtEdit;			//报价数量
	CEdit m_spreadEdit;				//报价间距
	CEdit m_riskPositionEdit;		//风险头寸
	CEdit m_maxPositionEdit;		//最大持仓
	CEdit m_positionEdit;			//持仓量
	CEdit m_sensitivityEdit;		//M参数

	CListCtrl m_strategyList;		//策略列表

private:
	Config const& m_config;
	InstID m_instFollowID;
	InstID m_hedgeID;
	hk::Arguments m_arguments;
	std::map<int, int> m_itemIndexMap;				//<businessID, itemIndex>
	std::map<int, hk::Arguments> m_hkArgumentsMap;	//<itemIndex, hfArguments>
	std::map<int, bool> m_formalQuotingMap;			//<itemIndex, isFormalQuoting>
	std::vector<std::shared_ptr<Business>> m_businesses;
	std::vector<std::string> m_months;
	std::vector<std::string> m_follows;
	std::map<std::string, std::string> m_followMap;	//<major, follow>	
};
