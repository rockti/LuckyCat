#pragma once
#include "strategy/hk_marketing/Arguments.h"

// PostionDlg 对话框

class PostionDlg : public CDialogEx
{
	DECLARE_DYNAMIC(PostionDlg)

public:
	PostionDlg(int positionAmt, money unitPrice, CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~PostionDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_POSITION_DLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:

	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnEnChangePosAmtEdit();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

	int GetPositionAmt() const { return m_positionAmt;  }
	hk::InitPosition const& GetPosition() {		
		Direction dir = m_positionAmt < 0 ? Direction::SELL : Direction::BUY;
		m_initPosition = hk::InitPosition{ dir, 0, QtyT(abs(m_positionAmt)), m_balanceQty };
		return m_initPosition;
	}
private:
	int m_positionAmt;
	money m_unitPrice;
	QtyT m_balanceQty;
	hk::InitPosition m_initPosition;
private:
	CEdit m_positionAmtEdit;
	CEdit m_positionPriceEdit;

};
