#pragma once

//–≈∫≈¡ø
class  CSemaphore
{
public:
	CSemaphore(int nValue = 1) { m_sid = CreateSemaphore(NULL,nValue,1000,NULL); }
	~CSemaphore() { CloseHandle(m_sid); }
	bool Lock() { return (WAIT_OBJECT_0 == WaitForSingleObject(m_sid,INFINITE)); }
	bool Lock(int timeout) { return (WAIT_OBJECT_0 == WaitForSingleObject(m_sid,timeout));}	
	bool UnLock(int* pPrevCount = NULL) { return (TRUE == ReleaseSemaphore(m_sid,1,(long*)pPrevCount));}
private:
	HANDLE m_sid;
};
