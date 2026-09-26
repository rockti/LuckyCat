#include "stdafx.h"
#include "SelectThread.h"
#include "SocketInit.h"


static CSocketInit _SOCKET_INIT;
const int SR_DEFAULT_SELECT_TIMEOUT = 60; //微秒
CSelectThread::CSelectThread() {
	//将该线程绑定在第二个cpu上运行，既保证双核也能起作用，又不会因为核心时钟频率不一致而导致时间不准确。
	if (!SetThreadAffinityMask(GetCurrentThread(),0x00000002)) {
//		Log::Warn(FT("CSelectThread: SetThreadAffinityMask Failed![%u]", GetLastError()));	
	}
}
CSelectThread::~CSelectThread() {
}

void CSelectThread::DispatchIOs() {
	DWORD dwSelectTimeout = SR_DEFAULT_SELECT_TIMEOUT;
	if (HandleOtherTask()) {
		dwSelectTimeout = 0;
	}

	int maxID = 0;
	fd_set readfds;
	fd_set writefds;
	PrepareIds(readfds, writefds, maxID);

	int ret = 0;
	if(0 == maxID) {
		::Sleep(1);
	}
	else {
		struct timeval timeout;
		timeout.tv_sec = dwSelectTimeout / 1000000;
		timeout.tv_usec = dwSelectTimeout % 1000000;
		ret = select(++maxID, &readfds, &writefds, NULL, &timeout);
	}

	SyncTime();
	if(ret > 0) {
		DispatchIO(readfds, writefds, maxID);
	}
}
void CSelectThread::DispatchIO(fd_set& readfds, fd_set& writefds, int& maxID) {
	for(CEventHandlerList::iterator itor=m_IOHandlerList.begin(); itor!=m_IOHandlerList.end(); ++itor) {
		int nReadID, nWriteID;

		if (NULL == (*itor))	continue;	//事件处理对象(Session)可能已被删除
		((CEventHandler*)(*itor))->GetIds(&nReadID, &nWriteID);
		if(nReadID<0 || (nReadID>0 && FD_ISSET(nReadID,&readfds))) {
			((CEventHandler*)(*itor))->HandleInput();
		}

		if (NULL == (*itor))	continue;	//事件处理对象(Session)可能已被删除
		if(nWriteID<0 || (nWriteID>0 && FD_ISSET(nWriteID, &writefds))) {
			((CEventHandler*)(*itor))->HandleOutput();
		}
	}
}
void CSelectThread::PrepareIds(fd_set& readfds, fd_set& writefds, int& maxID) {
	//去掉无效处理器指针
	if(m_bIOListHasNull) {
		m_IOHandlerList.remove(NULL);
		m_bIOListHasNull = false;
	}

	FD_ZERO(&readfds);
	FD_ZERO(&writefds);
	for(CEventHandlerList::iterator itor=m_IOHandlerList.begin(); itor!=m_IOHandlerList.end(); itor++) {
		if(NULL == (*itor))
			continue;

		int nReadID = 0;
		int nWriteID = 0;
		CEventHandler* pp = (CEventHandler*)(*itor);
		((CEventHandler*)(*itor))->GetIds(&nReadID, &nWriteID);
		if(nReadID > 0) {
			FD_SET(nReadID, &readfds);
			if(nReadID > maxID) {
				maxID = nReadID;
			}
		}

		if(nWriteID > 0) {
			FD_SET(nWriteID, &writefds);
			if(nWriteID > maxID) {
				maxID = nWriteID;
			}
		}
	}
}