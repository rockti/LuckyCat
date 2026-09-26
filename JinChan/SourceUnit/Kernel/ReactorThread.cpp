#include "stdafx.h"
#include "ReactorThread.h"

CReactorThread::CReactorThread()
:m_bIOListHasNull(false),m_bSpeciaFlag(false) {
	m_IOHandlerList.clear();
}
CReactorThread::~CReactorThread() {
}

void CReactorThread::RegisterIOHandler(CEventHandler* pEventHandler) {
	m_IOHandlerList.push_back(pEventHandler);
}
void CReactorThread::RemoveIOHandler(CEventHandler* pEventHandler) {
	for (CEventHandlerList::iterator iter=m_IOHandlerList.begin(); iter!=m_IOHandlerList.end(); iter++) {
		if ((*iter) == pEventHandler) {
			(*iter) = NULL;
			m_bIOListHasNull = true;
		}
	}
}

bool CReactorThread::HandleOtherTask() {
	if (!m_queueEvent.IsEmpty()) {
		return true;	//有Event，返回
	}
	if (m_timerHandler.CheckExpire(m_nCurrClockTick)) {
		return true;	//有Timer，返回
	}
	return false;
}
