#pragma once


class  CThreadBase  
{
public:
	CThreadBase();
	virtual ~CThreadBase();

	virtual bool Create();
	bool Join();
	bool IsCurrentThread();
	DWORD GetThreadID() const { return m_threadId; }

private:	
	static DWORD WINAPI _ThreadEntry(LPVOID pParam);
	virtual bool InitInstance();
	virtual void ExitInstance();
	virtual void Run() = 0;
	
private:
	 HANDLE m_threadHandle;
	 DWORD m_threadId;
};
