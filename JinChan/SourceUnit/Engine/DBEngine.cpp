#include "stdafx.h"
#include "DBEngine.h"
#include "ZtpDataStruct.h"

CDBEngine::CDBEngine(CDBScheduler* pMDBSchulder):m_pMDBSchulder(pMDBSchulder),m_count(0)
{
}

void CDBEngine::HandleMessage(CZTPPackage* pPackage)
{
	m_pMDBSchulder->AssignTask(pPackage->GetSequenceNo());		//先分配记录
	switch (pPackage->GetTID())
	{
	case TID_NtfMemtableChange:
		HandleMemberDBMsg(pPackage);
		break;
	default:
		break;
	}
	m_pMDBSchulder->SetCommitPoint(pPackage->GetSequenceNo());	//每条记录设置一个CommitPoint
}
void CDBEngine::HandleMemberDBMsg(CZTPPackage* pPackage)
{
	CFieldTypeIterator iter = pPackage->GetFieldTypeIterator();
	while(!iter.IsEnd())
	{
		CMemTableChangeField field;
		iter.Retrieve(&CMemTableChangeField::m_Describe, &field);
		TMDBActionType actType;
		if(ZTP_MTA_INSERT == field.MemtableChangeType)
		{
			actType = TA_INSERT;
		}
		else if(ZTP_MTA_UPDATE == field.MemtableChangeType)
		{
			actType	= TA_UPDATE;
		}
		else if(ZTP_MTA_DELETE == field.MemtableChangeType)
		{
			actType	= TA_DELETE;
		}
		iter.Next();

		//取出字段
		TFieldHeader fieldHead;
		iter.RetrieveHeader(fieldHead);
		ProcField(fieldHead.FieldID, iter, actType);
		iter.Next();
	}
}

void CDBEngine::ProcField(WORD fid, CFieldTypeIterator& iter, TMDBActionType actType)
{
// 	Log::Debug(fmt::format("Get package[{}] fid[0x%x]", ++m_count, fid));
	switch (fid)
	{
	case FID_SystemStatus:		//系统状态
		{
			CSystemStatusField field;
			iter.Retrieve(&CSystemStatusField::m_Describe, &field);
			m_pMDBSchulder->Dispatch("t_Sync_SystemStatus", &CSystemStatusField::m_Describe, &field, TA_DELETE);
			m_pMDBSchulder->Dispatch("t_Sync_SystemStatus", &CSystemStatusField::m_Describe, &field, TA_INSERT);
		}
		break;
	case FID_SeatInfo:			//席位状态
		{
			CSeatInfoField field;
			iter.Retrieve(&CSeatInfoField::m_Describe, &field);
			m_pMDBSchulder->Dispatch("t_Sync_SeatInfo", &CSeatInfoField::m_Describe, &field, actType);
		}
		break;
	case FID_Order:				//委托回报
		{
			COrderField field;
			iter.Retrieve(&COrderField::m_Describe, &field);
			m_pMDBSchulder->Dispatch("t_Oper_Order", &COrderField::m_Describe, &field, actType);
		}
		break;
	case FID_Trade:				//成交回报
		{
			CTradeField field;
			iter.Retrieve(&CTradeField::m_Describe, &field);
			m_pMDBSchulder->Dispatch("t_Oper_Trade", &CTradeField::m_Describe, &field, actType);
		}
		break;
	case FID_InvestorAccount:	//投资者账户
		{
			CInvestorAccountField field;
			iter.Retrieve(&CInvestorAccountField::m_Describe, &field);
			m_pMDBSchulder->Dispatch("t_Sync_InvestorAccount", &CInvestorAccountField::m_Describe, &field, actType);
			Log::Debug(fmt::format("Account:Type[{}] Margin[{:.3f}] Fee[{:.3f}] FrozenMargin[{:.3f}] FrozenFee[{:.3f}] LMargin[{:.3f}] SMargin[{:.3f}] LFMargin[{:.3f}] SFMargin[{:.3f}] Risk[{:.3f}]", actType
				,field.FrozenMargin,field.FrozenFee,field.LongMargin,field.ShortMargin,field.LongFrozenMargin,field.ShortFrozenMargin,field.Risk));
		}
		break;
	case FID_InvestorPosition:	//投资者持仓
		{
			CInvestorPositionField field;
			iter.Retrieve(&CInvestorPositionField::m_Describe,&field);
			m_pMDBSchulder->Dispatch("t_Oper_InvestorPosition", &CInvestorPositionField::m_Describe, &field, actType);
			Log::Debug(fmt::format("Position:Type[{}] FrozenMargin[{:.3f}] FrozenFee[{:.3f}] FrozenPosition[{}]", actType
				,field.FrozenMargin,field.FrozenFee,field.FrozenPosition));
		}
		break;
	case FID_UserSession:		//用户会话
		{
			CUserSessionField field;
			iter.Retrieve(&CUserSessionField::m_Describe, &field);
			m_pMDBSchulder->Dispatch("t_Oper_UserSession", &CUserSessionField::m_Describe, &field, actType);
		}
		break;
	case FID_Instrument:		//合约信息
		{
			CInstrumentField field;
			iter.Retrieve(&CInstrumentField::m_Describe,&field);
			m_pMDBSchulder->Dispatch("t_Sync_Instrument", &CInstrumentField::m_Describe, &field, actType);
		}
		break;
	case FID_InvestorMargin:	//保证金
		{
			CInvestorMarginField field;
			iter.Retrieve(&CInvestorMarginField::m_Describe,&field);
			m_pMDBSchulder->Dispatch("t_Sync_InvestorMargin", &CInvestorMarginField::m_Describe, &field, actType);
		}
		break;
	case FID_InvestorFee:		//手续费
		{
			CInvestorFeeField field;
			iter.Retrieve(&CInvestorFeeField::m_Describe,&field);
			m_pMDBSchulder->Dispatch("t_Sync_InvestorFee", &CInvestorFeeField::m_Describe, &field, actType);
		}
		break;
	default:
		break;
	}
}
