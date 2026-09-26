#pragma once
#include "SGEHandler.h"
#include "ZTPPackage.h"
#include "FTDPackage.h"
#include "MarketFlowMgr.h"
#include "MarketRepository.h"
#include "ZTPType.h"
#include "ServiceDef.h"
using namespace ztp;

enum class TriggerEvent {
	Timer,
	Order,
	Cancel,
};

class OrderStatus;
typedef std::shared_ptr<OrderStatus> OrderStatusPtr;
typedef std::vector<OrderStatusPtr> OrderStatuses;

class OrderStatus {
public:
	OrderStatus(std::string const& code, bool isFinal, bool isTradeable, OrderStatuses const& orderNexts, OrderStatuses const& cancelNexts) :
		Code(code), IsFinal(isFinal), IsTradeable(isTradeable), Nexts(orderNexts), CancelNexts(cancelNexts) {

	}
	std::string Code = "Sent";
	bool IsFinal = false;
	bool IsTradeable = false;
	OrderStatuses Nexts;
	OrderStatuses CancelNexts;
};

class SGEMockHandler;
class Changer {
public:
	Changer(SGEMockHandler* handler) : m_handler(handler) {}
	virtual void Action(data::COrder& order, data::CTrade& trade) = 0;

protected:
	SGEMockHandler* m_handler;
};

typedef std::shared_ptr<Changer> ChangerPtr;

typedef std::pair<OrderStatusPtr, ChangerPtr> StatusActionPair;
typedef std::vector<StatusActionPair> StatusActions;





struct NothingChanger : public Changer {
	NothingChanger(SGEMockHandler* handler) :Changer(handler) {}
	void Action(data::COrder& order, data::CTrade& trade) override {};
};

struct OnlyStatusChanger : public Changer {
	OnlyStatusChanger(SGEMockHandler* handler,type::OrderStatus status, DWORD notifyId) :Changer(handler),m_status(status),m_notifyId(notifyId) {}
	void Action(data::COrder& order, data::CTrade& trade) override;;
private:
	type::OrderStatus m_status;

	DWORD m_notifyId;

};

struct OrderedChanger : public Changer {
	OrderedChanger(SGEMockHandler* handler, char const* systemNo) :Changer(handler), m_systemNo(systemNo) {}
	void Action(data::COrder& order, data::CTrade& trade) override;
private:
	std::string m_systemNo;
};

struct AllTradeChanger : public Changer {
	AllTradeChanger(SGEMockHandler* handler) :Changer(handler) {}
	void Action(data::COrder& order, data::CTrade& trade) override;;
};

struct PartialTradeChanger : public Changer {
	PartialTradeChanger(SGEMockHandler* handler, int tradeVolume) :Changer(handler),m_tradeVolume(tradeVolume) {}
	void Action(data::COrder& order, data::CTrade& trade) override;;
private:
	int m_tradeVolume;
};


class Playbook {
public:
	Playbook(data::COrder const& order):
	m_order(order) {
		m_trade.BrokerID = order.BrokerID;
		m_trade.ExchangeID = order.ExchangeID;
		m_trade.ClientID = order.ClientID;
		m_trade.Direction = order.Direction;
		m_trade.InstrumentID = order.InstrumentID;
		m_trade.InvestorID = order.InvestorID;
		m_trade.OffsetFlag = order.OffsetFlag;
		m_trade.OrderLocalID = order.OrderLocalID;
		m_trade.OrderSysID = order.OrderSysID;
		m_trade.OrderOperatorID = order.OrderOperatorID;
		m_trade.ProductClass = order.ProductClass;
	}

	Playbook(Playbook const& other) = default;

	void OnTimer() {
		if (m_index < m_chain.size()) {
			m_chain[m_index++].second->Action(m_order, m_trade);
		}
	}

	void OnCancel() {
		m_cancel.second->Action(m_order,m_trade);
	}

	void Add(OrderStatusPtr status, ChangerPtr changer) {
		m_chain.push_back(make_pair(status, changer));
	}

	void AddCancel(OrderStatusPtr status, ChangerPtr changer) {
		m_cancel = make_pair(status, changer);
	}
private:
	data::COrder m_order;
	data::CTrade m_trade;
	StatusActions m_chain;
	StatusActionPair m_cancel;
	size_t m_index = 0;
};


class PlaybookFactory {
public:
	PlaybookFactory(SGEMockHandler* handler) :m_handler(handler)
	{
		m_cancellFailed.reset(new OrderStatus{ "CancelFailed", true, false, {}, {} });
		m_cancelledWithMoreTrade.reset(new OrderStatus{ "CancelWithMoreTrade", true, false, {}, {} });
		m_cancelled.reset(new OrderStatus{ "Cancelled", true, false, {}, {} });
		m_allTrade.reset(new OrderStatus{ "AllTrade", true, false, {}, {} });
		m_partialTrade.reset(new OrderStatus{ "PartialTrade", false, true, { m_allTrade}, {m_cancelled, m_cancelledWithMoreTrade, m_cancellFailed} });
		m_neverTrade.reset(new OrderStatus{ "NeverTrade", true, false, {}, {m_cancelled} });
		m_noTrade.reset(new OrderStatus{ "NoTrade", false, true, { m_allTrade, m_partialTrade},  {m_cancelled, m_cancelledWithMoreTrade, m_cancellFailed} });
		m_orderFailed.reset(new OrderStatus{ "OrderFailed", true, false, {}, {} });
		m_accepted.reset(new OrderStatus{ "Accepted", false, true, {m_noTrade, m_neverTrade,m_allTrade, m_partialTrade},  {m_cancelled, m_cancelledWithMoreTrade, m_cancellFailed} });
		m_sent.reset(new OrderStatus{ "Sent", false, false, { m_accepted, m_orderFailed},  {m_cancelled, m_cancelledWithMoreTrade, m_cancellFailed} });
		m_current = (&*m_sent);
	}

	Playbook Normal(data::COrder& orginalOrder) {
		Playbook pb(orginalOrder);
		static int s_systemNo = 0;
		auto systemNo = "MOCKSYSNO" + std::to_string(s_systemNo++);
		pb.Add(m_noTrade, ChangerPtr(new OrderedChanger(m_handler, systemNo.c_str())));
		if (orginalOrder.Volume > 1) {
			pb.Add(m_partialTrade, ChangerPtr(new PartialTradeChanger(m_handler, 1)));
		} else {
			pb.Add(m_allTrade, ChangerPtr(new AllTradeChanger(m_handler)));
		}
		pb.AddCancel(m_cancelled, ChangerPtr(new OnlyStatusChanger(m_handler, type::OrderStatus::Canceled, TID_NotifyOrder)));
		return pb;
	}

	Playbook FakTradeNormal(data::COrder& orginalOrder) {
		Playbook pb(orginalOrder);
		pb.Add(m_noTrade, ChangerPtr(new OnlyStatusChanger(m_handler, type::OrderStatus::NoTradeQueueing, TID_NotifyOrder)));
		if (orginalOrder.Volume > 1) {
			pb.Add(m_partialTrade, ChangerPtr(new PartialTradeChanger(m_handler, 1)));
		} else {
			pb.Add(m_allTrade, ChangerPtr(new AllTradeChanger(m_handler)));
		}
		pb.AddCancel(m_cancelled, ChangerPtr(new OnlyStatusChanger(m_handler, type::OrderStatus::Canceled, TID_NotifyOrder)));
	}

private:
	OrderStatusPtr m_sent;
	OrderStatusPtr m_accepted;
	OrderStatusPtr m_orderFailed;
	OrderStatusPtr m_noTrade;
	OrderStatusPtr m_neverTrade;
	OrderStatusPtr m_allTrade;
	OrderStatusPtr m_partialTrade;
	OrderStatusPtr m_cancelled;
	OrderStatusPtr m_cancelledWithMoreTrade;
	OrderStatusPtr m_cancellFailed;
	OrderStatus* m_current;

	SGEMockHandler* m_handler;
};

class SGEMockConnection;
class SGEMockHandler : public CBaseExchApi
{
public:
	SGEMockHandler(SgeLinkType::TYPE type, CMarketFlowManager* pFlowManager, int topicID, std::string const& sgeAddress, int sgePort, SafeContext& safeContext);
	~SGEMockHandler();

//	char const* GetAddress(int addressId) { return m_addressList[addressId].c_str(); }
	void RegistSGEContext(SGEContext* pContext) { m_pSgeContext = pContext; }

	void Login();
	bool ReqOrderInsert(data::COrder& inputOrderField);
	bool ReqOrderAction(data::COrderAction& actionField);

	void StartWork();

	virtual bool GetNextUseNoBuf(CPackage* pPackage) { 
		return m_seriesReader.GetNextUseNoBuf(pPackage); 
	}

	virtual int GetOrderLocalID() { 
		return m_maxOrderLocalID++;
	}
	type::TradingDay const& GetTradingDay() const { return m_tradingDay; }
private:

public:
	SgeLinkType::TYPE GetLinkType() const { return m_linkType; }
	void RegistContext(SGEContext* pContext) { m_pSgeContext = pContext; }
	void PushOrder(data::COrder& order, int errorCode, std::string const& msg) {
		m_pZtpPackage->PreparePublish(TID_NotifyOrder);
		m_pZtpPackage->SetSubjectID(TID_NotifyOrder);
		m_pZtpPackage->SetSequenceNo(m_privateSeqNo);
		m_pZtpPackage->AddField(order);
		m_pZtpPackage->MakePackage();
		m_pSeriesFlow->Append(m_pZtpPackage->GetAddress(), m_pZtpPackage->GetLength());
	}

	void PushTrade(data::CTrade& trade) {
		m_pZtpPackage->PreparePublish(TID_NotifyTrade);
		m_pZtpPackage->SetSubjectID(PRIVATE_FLOW);
		m_pZtpPackage->SetSequenceNo(m_privateSeqNo);
		m_pZtpPackage->AddField(trade);
		m_pZtpPackage->MakePackage();
		m_pSeriesFlow->Append(m_pZtpPackage->GetAddress(), m_pZtpPackage->GetLength());

	}

	void PushLogin() {
		m_pZtpPackage->PreparePublish(TID_NotifySeatLoginSuccess);
		m_pZtpPackage->SetSubjectID(PUBLIC_FLOW);
		m_pZtpPackage->SetSequenceNo(m_privateSeqNo);

		data::CSeatLogin loginInfoField;
		loginInfoField.ExchangeID = "SGE";
		loginInfoField.TraderID = type::TraderID(m_seatInfo.ParticipantID.value());
		loginInfoField.SeatID = m_seatInfo.SeatID;
		loginInfoField.OrderLocalID = 0;
		loginInfoField.TradingDay = m_tradingDay;
		m_pZtpPackage->AddField(loginInfoField);
		m_pZtpPackage->MakePackage();
		m_pSeriesFlow->Append(m_pZtpPackage->GetAddress(), m_pZtpPackage->GetLength());
	}

	void PushInstrumentStatus(bool isOpen) {
		data::CInstrumentStatus field;
		field.ExchangeID = "SGE";
		field.InstrumentStatus = isOpen ? type::InstrumentStatus::Continous: type::InstrumentStatus::Pause;
		m_pZtpPackage->PreparePublish(TID_NotifyInstrumentStatus);
		m_pZtpPackage->SetSubjectID(PUBLIC_FLOW);
		m_pZtpPackage->SetSequenceNo(m_privateSeqNo);
		m_pZtpPackage->AddField(field);
		m_pZtpPackage->MakePackage();
		m_pSeriesFlow->Append(m_pZtpPackage->GetAddress(), m_pZtpPackage->GetLength());
	}

	void PushInstrumentReady() {
		PushInstrumentStatus(true);
	}

	void PushSpotPosition() {
		data::CSpotPositionNotify spotPos;
		spotPos.ExchangeID = "SGE";
		spotPos.InvestorID = "0215188";
		spotPos.BrokerID = "007421";
		spotPos.InstrumentID = "Au99.99";
		spotPos.Volume = 100;
		spotPos.OffsetFlag = type::OffsetFlag::Open;
		m_pZtpPackage->PreparePublish(TID_NotifySpotPosition);
		m_pZtpPackage->SetSubjectID(PRIVATE_FLOW);
		m_pZtpPackage->SetSequenceNo(m_privateSeqNo);
		m_pZtpPackage->AddField(spotPos);
		m_pZtpPackage->MakePackage();
		m_pSeriesFlow->Append(m_pZtpPackage->GetAddress(), m_pZtpPackage->GetLength());
	}

	void PushDeferFee() {
		data::CDeferFee deferFee;
		deferFee.InstrumentID = "Au(T+D)";
		deferFee.Direction = type::DeferFeeDirection::LongPay;
		deferFee.FeeRate = 0.008;
		m_pZtpPackage->PreparePublish(TID_RtnDeferFee);
		m_pZtpPackage->SetSubjectID(PRIVATE_FLOW);
		m_pZtpPackage->SetSequenceNo(m_privateSeqNo);
		m_pZtpPackage->AddField(deferFee);
		m_pZtpPackage->MakePackage();
		m_pSeriesFlow->Append(m_pZtpPackage->GetAddress(), m_pZtpPackage->GetLength());
	}

private:

public:
	SgeLinkType::TYPE m_linkType;
	std::string m_address;
	std::string m_localIp;

	int m_port;
	int m_topicID;								//行情主题号
	int m_encryptedMsgLen;						//安全信息长度
	unsigned char m_encryptedMsg[SGE_MSG_LEN];	//发送的加密串

	SafeHandler m_safeHandler;					//安全回调接口
	session_client_info_t m_sessionInfo;		//SGE连接参数
	Parser m_parser;							//SGE报文解析器

private:
	CMarketFlowManager* m_pMarketFlowManager;
//	CMarketRepository* m_pMarketRepository;
	SGEContext* m_pSgeContext;				//回调上下文
	CFTDPackage* m_pFtdPackage;				//行情数据包
	CZTPPackage* m_pZtpPackage;				//交易数据包


	int m_marketSeqNo;						//行情序号，前置维护，落库用
	int m_privateSeqNo;						//通讯报文序号(重复的通讯报文不录入数据库)
	int m_maxOrderLocalID;					//最大本地委托号

	std::map<std::string, data::CInstrument*> m_contractSet;	//合约列表
	type::InstrumentStatus m_exchangeStatus;			//交易状态

	PlaybookFactory m_playbookFactory;
	std::map<int, Playbook> m_orders;
};