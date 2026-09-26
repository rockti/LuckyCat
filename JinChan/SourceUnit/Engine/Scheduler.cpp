#include "stdafx.h"
#include "Scheduler.h"

CScheduler::CScheduler(IFlow* pMemChangeFlow)
:m_pMemChangeFlow(pMemChangeFlow),m_pPubMessage(new CZTPPackage)
{
	m_pPubMessage->ConstructAllocate(ZTP_PACKAGE_MAX_SIZE, 1000);
// 	m_pPubMessage->PreparePublish(TID_NtfMemtableChange);
}
CScheduler::~CScheduler()
{
	if(NULL != m_pPubMessage)
	{
		delete m_pPubMessage;
		m_pPubMessage = NULL;
	}
}

void CScheduler::Dispatch(char const* pszObjectName, CFieldDescribe* pDescirbe, void const* pData, type::MemTableChangeType changeType)
{
	data::CMemTableChange field;
	field.MemTableName = pszObjectName;
	field.MemtableChangeType = changeType;

	m_pPubMessage->PreparePublish(TID_NtfMemtableChange);
	m_pPubMessage->AddField(field);
	m_pPubMessage->AddField(pDescirbe, (void*)pData);
	m_pPubMessage->MakePackage();
	m_pMemChangeFlow->Append(m_pPubMessage->GetAddress(), m_pPubMessage->GetLength());

	//检测落库字段
// 	TFieldHeader theFieldHeader;
// 	COrderInsertFailedField failOrderField;
// 	CMemTableChangeField getMemField;
// 	m_pPubMessage->ValidPackage();
// 	CFieldTypeIterator it = m_pPubMessage->GetFieldTypeIterator();
// 	it.RetrieveHeader(theFieldHeader);
// 	it.Retrieve(&getMemField.m_Describe, &getMemField);
// 	it.Next();
// 	it.Retrieve(&failOrderField.m_Describe, &failOrderField);
}

//把出入金通知发送到内存变化流中去，这样前置模块收到这个流，就可以发送给客户。这个函数由触发器 CUsDepositTrigger 调用。
void CScheduler::InvestorAccountDepositNotify(CFieldDescribe *pDescirbe, const void *pData)
{
// 	int nSeqNo = 0;
// 	m_pPubMessage->PreparePublish(TID_NtfInvestorAccountDeposit);
// 	m_pPubMessage->AddField(pDescirbe,(void *)pData);//增加可变消息域
// 	m_pPubMessage->MakePackage();
// 	nSeqNo = m_pResultFlow->Append(m_pPubMessage->Address(),m_pPubMessage->Length());
// 	if(nSeqNo < 0 )
// 	{  
// 		REPORT_EVENT(LOG_CRITICAL,"CUsScheduler::InvestorAccountDepositNotify", "Can not Append To ResultFlow");
// 	}
}
