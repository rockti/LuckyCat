#pragma once

#include "GrabStatus.h"
#include "model/dealing/Dealing.h"

namespace tg::grab {
	class GrabStrategy;
	class GrabBusiness {
	public:
		GrabBusiness(GrabStrategy& pParent, InstID inst, QtyT openQty, QtyT holdQty/*保留*/, money offsetPrice, money stopPrice);
		void OnActiveOrder(Dealing const& dealing);
		void OnActiveTrade(int qty);
		void OnActiveCancel(int qty);

		void OnQuote();
		void OnPause();
		void OnPassiveTrade(int qty);
		void OnPassiveCancel(int qty);

		void CancelSGEActive(Dealing const& dealing);
		void SendPassive(money price, int qty);	// SGE只有Defer才会自动平仓
		void CancelPassive();
		bool IsPassiveOrdered();
		bool MissPassiveChance();
		money GetStopPrice();

		int CalcPassiveQty() { return m_position - m_holdQty - m_passiveSent; }
		BaseStatus* GetStatus(Phase index) { return m_machine.GetStatus(index); }

	public:
		GrabStrategy& m_parent;	
		GrabMachine m_machine;
		BaseStatus* m_currentStatusPtr = nullptr;

		InstID const m_instID;		// 开仓合约
		QtyT const m_openQty;		// 开仓数量
		QtyT const m_holdQty;		// 保留持仓
		money const m_offsetPrice;	// 平仓价
		money const m_stopPrice;	// 止损价

		Dealing* m_passiveDealingPtr = nullptr;
		QtyT m_passiveSent = 0;		// 已发平仓量
		QtyT m_position = 0;		// 持仓总量
		QtyT m_pendingAmt = 0;		// 挂单总量
		std::string m_lastSystemID;	// 系统号

		bool m_endTag = false;		// 结束标记
		bool m_stopTag = false;		// 止损标记
		bool m_retryTag = false;	// 重发Passive
		int m_confirmCnt = 2;		// 确认两次没有成交
	};
}