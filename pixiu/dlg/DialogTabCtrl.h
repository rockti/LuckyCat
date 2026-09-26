#pragma once
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxtempl.h>
#endif // _AFX_NO_AFXCMN_SUPPORT

// CibTabCtrl

class CDialogTabCtrl : public CTabCtrl
{
	DECLARE_DYNAMIC(CDialogTabCtrl)

public:
	CDialogTabCtrl();
	virtual ~CDialogTabCtrl();

public:
	void AddTabPane(CString strCaption, unsigned int nTemplateID,CDialog* pDlg);
	void SetDefaultPane(int index);

	void EnableAllDialog(bool enable);

protected:
	afx_msg void OnNMClick(NMHDR *pNMHDR, LRESULT *pResult);

	afx_msg void OnMove(int x, int y);
	DECLARE_MESSAGE_MAP()

private:
	std::vector<CString> m_tabCaptions;
	std::vector<CDialog*> m_tabPanes;
	int m_curSel;
	void HideAllDialog();
	void ShowCurDialog();

public:
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
};


