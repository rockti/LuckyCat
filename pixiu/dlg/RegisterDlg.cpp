// RegisterDlg.cpp: 实现文件
//

#include "pch.h"
#include "pixiu.h"
#include "RegisterDlg.h"
#include "afxdialogex.h"


// RegisterDlg 对话框

IMPLEMENT_DYNAMIC(RegisterDlg, CDialogEx)

RegisterDlg::RegisterDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_REGIST_DLG, pParent)
{

}

RegisterDlg::~RegisterDlg()
{
}

void RegisterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CODE_EDIT, m_codeEdit);
}


BEGIN_MESSAGE_MAP(RegisterDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &RegisterDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &RegisterDlg::OnBnClickedCancel)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// RegisterDlg 消息处理程序
void RegisterDlg::OnClose() {
	CDialogEx::OnCancel();
}

void RegisterDlg::OnBnClickedOk() {
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnOK();
}

void RegisterDlg::OnBnClickedCancel() {
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnCancel();
}


