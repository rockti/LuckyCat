// C:\PrivateWorks\pxgit\pixiu\dlg\LoginDlg.cpp: 实现文件
//

#include "pch.h"
#include "pixiu.h"
#include "dlg/LoginDlg.h"
#include "afxdialogex.h"


// LoginDlg 对话框

IMPLEMENT_DYNAMIC(LoginDlg, CDialogEx)

LoginDlg::LoginDlg(Credential const& credential, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_LOGIN_DLG, pParent),m_credential(credential)
{

}

LoginDlg::~LoginDlg()
{
}

void LoginDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BOOL LoginDlg::OnInitDialog() {
	CDialogEx::OnInitDialog();
	GetDlgItem(IDC_BROKERID_STATIC)->SetWindowTextW(CA2T(m_credential.BrokerID.c_str()));
	GetDlgItem(IDC_USERID_STATIC)->SetWindowTextW(CA2T(m_credential.UserID.c_str()));
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


BEGIN_MESSAGE_MAP(LoginDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &LoginDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &LoginDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// LoginDlg 消息处理程序
void LoginDlg::OnBnClickedOk() {
	GetDlgItem(IDC_PWD_EDIT)->GetWindowText(m_password);
	if (m_password.IsEmpty()) {
		::AfxMessageBox(_T("登录密码不能为空!"), MB_ICONERROR);
		GetDlgItem(IDC_PWD_EDIT)->SetFocus();
		return;
	}
	CDialogEx::OnOK();
}


void LoginDlg::OnBnClickedCancel() {
	CDialogEx::OnCancel();
}

