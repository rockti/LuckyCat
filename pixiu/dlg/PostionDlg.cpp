// PostionDlg.cpp: 实现文件
//

#include "pch.h"
#include "pixiu.h"
#include "PostionDlg.h"
#include "afxdialogex.h"
#include "utils/Ctrl.h"
#include "strategy/QuoteManager.h"

// PostionDlg 对话框

IMPLEMENT_DYNAMIC(PostionDlg, CDialogEx)

PostionDlg::PostionDlg(int positionAmt, money unitPrice, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_POSITION_DLG, pParent), m_positionAmt(positionAmt), m_unitPrice(unitPrice) {

}

PostionDlg::~PostionDlg() {
}

void PostionDlg::DoDataExchange(CDataExchange* pDX) {
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_POS_AMT_EDIT, m_positionAmtEdit);
	DDX_Control(pDX, IDC_POS_PRICE_EDIT, m_positionPriceEdit);
}

BOOL PostionDlg::OnInitDialog() {
	CDialogEx::OnInitDialog();
	CommonFunction::Int2WindowText(m_positionAmtEdit, m_positionAmt);
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

HBRUSH PostionDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) {
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);
	if (CTLCOLOR_STATIC == nCtlColor) {
		if (IDC_DIR_STATIC == pWnd->GetDlgCtrlID()) {
			m_positionAmt < 0 ? pDC->SetTextColor(RGB(0, 255, 0)) : pDC->SetTextColor(RGB(255, 0, 0));
		}
	}

	return hbr;
}


BEGIN_MESSAGE_MAP(PostionDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &PostionDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &PostionDlg::OnBnClickedCancel)
	ON_EN_CHANGE(IDC_POS_AMT_EDIT, &PostionDlg::OnEnChangePosAmtEdit)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// PostionDlg 消息处理程序

void PostionDlg::OnBnClickedOk() {
	if (0 == m_positionAmt) {
		::AfxMessageBox(_T("历史持仓不能为0"), MB_ICONWARNING);
		m_positionAmtEdit.SetFocus();
		return;
	}

	auto balanceQty = CommonFunction::WindowText2Int(m_positionPriceEdit);
	m_balanceQty = abs(balanceQty);
// 	if (m_priceTick<1000 || m_priceTick>50000) {
// 		::AfxMessageBox(_T("历史持仓价格错误"), MB_ICONWARNING);
// 		m_positionPriceEdit.SetFocus();
// 		return;
// 	}
	CDialogEx::OnOK();
}


void PostionDlg::OnBnClickedCancel() {
	CDialogEx::OnCancel();
}

void PostionDlg::OnEnChangePosAmtEdit() {
	m_positionAmt = CommonFunction::WindowText2Int(m_positionAmtEdit);
	CString dirText = m_positionAmt < 0 ? _T("卖") : _T("买");
	GetDlgItem(IDC_DIR_STATIC)->SetWindowText(dirText);
}
