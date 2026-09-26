#include "pch.h"

#include "DialogTabCtrl.h"

IMPLEMENT_DYNAMIC(CDialogTabCtrl, CTabCtrl)
CDialogTabCtrl::CDialogTabCtrl()
:m_curSel(-1)
{
}

CDialogTabCtrl::~CDialogTabCtrl()
{
	for (size_t paneCount = 0; paneCount < m_tabPanes.size(); ++paneCount)
	{
		delete m_tabPanes[paneCount];
	}
}


BEGIN_MESSAGE_MAP(CDialogTabCtrl, CTabCtrl)
	ON_NOTIFY_REFLECT(NM_CLICK, OnNMClick)
	ON_WM_MOVE()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
END_MESSAGE_MAP()


// CibTabCtrl message handlers

void CDialogTabCtrl::OnNMClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	UNREFERENCED_PARAMETER(pNMHDR);

	m_curSel = GetCurSel();
	HideAllDialog();
	ShowCurDialog();

	*pResult = 0;
}


void CDialogTabCtrl::AddTabPane(CString strCaption, unsigned int nTemplateID,CDialog* pDlg)
{
	pDlg->Create(nTemplateID,this);
	m_tabPanes.push_back(pDlg); 
	TC_ITEM tci = {0};
	tci.mask = TCIF_TEXT;
	tci.pszText = strCaption.AllocSysString();
	tci.cchTextMax = strCaption.GetLength();
	InsertItem(GetItemCount(),&tci);
}

void CDialogTabCtrl::OnMove(int x, int y)
{
	CTabCtrl::OnMove(x, y);
	ShowCurDialog();
}


void CDialogTabCtrl::SetDefaultPane(int index)
{
	if(index < m_tabPanes.size())
	{
		m_curSel = index;
		SetCurSel(index);
		ShowCurDialog();
	}
}

void CDialogTabCtrl::HideAllDialog()
{
	for (size_t paneCount = 0; paneCount < m_tabPanes.size(); ++paneCount)
	{
		m_tabPanes[paneCount]->ShowWindow(SW_HIDE);
	}
}

void CDialogTabCtrl::ShowCurDialog()
{
 	CDialog* pDlg = m_tabPanes[m_curSel];
	CRect r;
	GetClientRect(r);
//	GetWindowRect(r);
//	GetParent()->ScreenToClient(r);
	pDlg->SetWindowPos(&CWnd::wndBottom, r.left, r.top+32, r.Width()-3, r.Height()-35, SWP_FRAMECHANGED|SWP_NOZORDER);
	pDlg->ShowWindow(SW_SHOW);
	pDlg->UpdateWindow();
}

void EnableDialogAllControl(CDialog* pDlg, bool enable)
{
	CWnd *pWnd = pDlg->GetWindow(GW_CHILD);
	while( pWnd )
	{
		pWnd->EnableWindow(enable);  //这里可以改为做其它的操作
		pWnd = pWnd->GetWindow( GW_HWNDNEXT );
	}
}

void CDialogTabCtrl::EnableAllDialog(bool enable)
{
	for (size_t paneCount = 0; paneCount < m_tabPanes.size(); ++paneCount)
	{
		EnableDialogAllControl(m_tabPanes[paneCount],enable);
	}	
}
void CDialogTabCtrl::OnSetFocus(CWnd* pOldWnd)
{
	CTabCtrl::OnSetFocus(pOldWnd);
	RECT rect = {0};
	GetItemRect(m_curSel,&rect);

	this->RedrawWindow(&rect);
}

void CDialogTabCtrl::OnKillFocus(CWnd* pNewWnd)
{
	CTabCtrl::OnKillFocus(pNewWnd);
	RECT rect = {0};
	GetItemRect(m_curSel,&rect);

	this->RedrawWindow(&rect);
}
