#pragma once
#include "ReactorThread.h"

//网络信号处理线程
class CSelectThread : public CReactorThread  
{
public:
	CSelectThread();
	virtual ~CSelectThread();

private:
	virtual void DispatchIOs();
	void DispatchIO(fd_set& readfds, fd_set& writefds, int& maxID);
	void PrepareIds(fd_set& readfds, fd_set& writefds, int& maxID);
};