#pragma once

//»¥³âÁ¿
class CMutex
{
public:
	CMutex() { m_lock = CreateMutex(NULL,FALSE,NULL); }
	~CMutex() { CloseHandle(m_lock); }

	void Lock() { WaitForSingleObject(m_lock, INFINITE); }
	bool TryLock() { return (WAIT_OBJECT_0 == WaitForSingleObject(m_lock,0)); }
	void UnLock() { ReleaseMutex(m_lock); }	
private:
	HANDLE  m_lock;
};

class CAutoMutex
{
public:
	CAutoMutex(CMutex* pMutex)
	{
		m_pMutex = pMutex;
		m_pMutex->Lock();
	}
	~CAutoMutex()
	{
		m_pMutex->UnLock();
	}
private:
	CMutex* m_pMutex;
};