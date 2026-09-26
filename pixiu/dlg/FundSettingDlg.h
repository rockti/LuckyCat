#pragma once
#include "type/Money.h"
#include "utils/Ctrl.h"
#include "model/Constants.h"
#include "config/Config.h"

// FundSettingDlg 对话框

class FundSettingDlg : public CDialogEx
{
	DECLARE_DYNAMIC(FundSettingDlg)

public:
	FundSettingDlg(ConnPlatform const& platform, CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~FundSettingDlg();

	money GetPlatformAvailable(ConnPlatform platformIndex);
// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FUND_SETTING_DLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
private:
	ConnPlatform m_platform;
	CEdit m_futuresAvailableEdit;
	CEdit m_SGEAvailableEdit;
	CEdit m_HKAvailableEdit;

	money m_futuresAvailable;
	money m_spotAvailablle;
	money m_hkAvailablle;
private:
	void InitFundsEdit(ConnPlatform platformIndex);
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
};
