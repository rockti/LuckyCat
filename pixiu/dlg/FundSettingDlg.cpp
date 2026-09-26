// D:\GitSource\px\pixiu\dlg\FundSettingDlg.cpp: 实现文件
//

#include "pch.h"
#include "pixiu.h"
#include "FundSettingDlg.h"
#include "afxdialogex.h"


// FundSettingDlg 对话框

IMPLEMENT_DYNAMIC(FundSettingDlg, CDialogEx)

FundSettingDlg::FundSettingDlg(ConnPlatform const& platform, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_FUND_SETTING_DLG, pParent), m_platform(platform) {
}

FundSettingDlg::~FundSettingDlg() {
}

void FundSettingDlg::DoDataExchange(CDataExchange* pDX) {
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_AVAILABLE_EDIT, m_futuresAvailableEdit);
	DDX_Control(pDX, IDC_SGE_AVAILABLE_EDIT, m_SGEAvailableEdit);
	DDX_Control(pDX, IDC_HKEX_AVAILABLE_EDIT, m_HKAvailableEdit);
}

BOOL FundSettingDlg::OnInitDialog() {
	CDialogEx::OnInitDialog();
	InitFundsEdit(m_platform);
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}



BEGIN_MESSAGE_MAP(FundSettingDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &FundSettingDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &FundSettingDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// FundSettingDlg 消息处理程序
void FundSettingDlg::OnBnClickedOk() {
	if (m_futuresAvailableEdit.IsWindowEnabled()) {
		m_futuresAvailable = CommonFunction::WindowText2Double(m_futuresAvailableEdit);
		if (m_futuresAvailable <= 0) {
			::AfxMessageBox(_T("期货可用资金必须大于0"));
			m_futuresAvailableEdit.SetWindowText(_T("0.0"));
			m_futuresAvailableEdit.SetFocus();
			return;
		}
	}
	if (m_SGEAvailableEdit.IsWindowEnabled()) {
		m_spotAvailablle = CommonFunction::WindowText2Double(m_SGEAvailableEdit);
		if (m_spotAvailablle <= 0) {
			::AfxMessageBox(_T("现货可用资金必须大于0"));
			m_SGEAvailableEdit.SetWindowText(_T("0.0"));
			m_SGEAvailableEdit.SetFocus();
			return;
		}
	}

	CDialogEx::OnOK();
}

void FundSettingDlg::OnBnClickedCancel() {
	CDialogEx::OnCancel();
}

money FundSettingDlg::GetPlatformAvailable(ConnPlatform platformIndex) {
	return (ConnPlatform::SGE == platformIndex) ? m_spotAvailablle : m_futuresAvailable;
}
void FundSettingDlg::InitFundsEdit(ConnPlatform platformIndex) {
	if (ConnPlatform::SGE == platformIndex) {
		m_SGEAvailableEdit.EnableWindow(TRUE);
		m_SGEAvailableEdit.SetWindowText(_T("0.0"));
		m_SGEAvailableEdit.SetFocus();
	}
	else {
		m_futuresAvailableEdit.EnableWindow(TRUE);
		m_futuresAvailableEdit.SetWindowText(_T("0.0"));
		m_futuresAvailableEdit.SetFocus();
	}
}
