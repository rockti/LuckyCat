#include "stdafx.h"
#include "MarketEngine.h"
#include "ZtpDataStruct.h"
#include "PackageDefine.h"

CMarketEngine::CMarketEngine(CMarketScheduler* pMarketSchulder):m_pMarketSchulder(pMarketSchulder),m_count(0)
{
}

void CMarketEngine::HandleMessage(CZTPPackage* pPackage)
{
	m_pMarketSchulder->AssignTask(pPackage->GetSequenceNo());		//先分配记录
	switch (pPackage->GetTID())
	{
	case TID_NtfMemtableChange:
		HandleMemberDBMsg(pPackage);
		break;
	default:
		break;
	}
	m_pMarketSchulder->SetCommitPoint(pPackage->GetSequenceNo());	//每条记录设置一个CommitPoint
}
void CMarketEngine::HandleMemberDBMsg(CZTPPackage* pPackage)
{
// 	CFieldTypeIterator iter = pPackage->GetFieldTypeIterator();
// 	while(!iter.IsEnd())
// 	{
// 		CMemTableChangeField field;
// 		iter.Retrieve(&CMemTableChangeField::m_Describe, &field);
// 		TMDBActionType actType;
// 		if(ZTP_MTA_INSERT == field.MemtableChangeType)
// 		{
// 			actType = TA_INSERT;
// 		}
// 		else if(ZTP_MTA_UPDATE == field.MemtableChangeType)
// 		{
// 			actType	= TA_UPDATE;
// 		}
// 		else if(ZTP_MTA_DELETE == field.MemtableChangeType)
// 		{
// 			actType	= TA_DELETE;
// 		}
// 		iter.Next();
// 
// 		//取出字段
// 		TFieldHeader fieldHead;
// 		iter.RetrieveHeader(fieldHead);
// 		ProcField(fieldHead.FieldID, iter, actType);
// 		iter.Next();
// 	}
}

void CMarketEngine::ProcField(WORD fid, CFieldTypeIterator& iter, TMDBActionType actType)
{
}
