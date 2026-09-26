#pragma once
#include "DBScheduler.h"
#include "ZTPPackage.h"
#include "DBWorkThread.h"

class CDBEngine
{
public:
	CDBEngine(CDBScheduler* pMDBSchulder);
	void HandleMessage(CZTPPackage* pPackage);
	void HandleMemberDBMsg(CZTPPackage* pPackage);
	void ProcField(WORD fid, CFieldTypeIterator& iter, TMDBActionType actType);

private:
	int m_count;
	CDBScheduler* m_pMDBSchulder;
};