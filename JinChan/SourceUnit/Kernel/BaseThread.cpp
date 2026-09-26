#include "stdafx.h"
#include "BaseThread.h"

DWORD WINAPI CThreadBase::_ThreadEntry(LPVOID pParam) {
	CThreadBase* pThread = (CThreadBase*)pParam;
	srand((unsigned)time(NULL));
	if(pThread->InitInstance()) {
		pThread->Run();
	}
	pThread->ExitInstance();
	return NULL;
}

CThreadBase::CThreadBase()
:m_threadHandle(NULL),m_threadId(0) {
}
CThreadBase::~CThreadBase() {
}

bool CThreadBase::Create() {
	if (NULL == m_threadHandle) {
		m_threadHandle = ::CreateThread(NULL,0,_ThreadEntry,this,0,&m_threadId);
	}
	return (NULL!=m_threadHandle);
}
bool CThreadBase::Join() {
	if (NULL == m_threadHandle) {
		return true;
	}
	return (0 != WaitForSingleObject(m_threadHandle, INFINITE));
}

bool CThreadBase::InitInstance() {
	return true;
}
void CThreadBase::ExitInstance() {
}
bool CThreadBase::IsCurrentThread() {
	return ::GetCurrentThreadId() == m_threadId;
}
