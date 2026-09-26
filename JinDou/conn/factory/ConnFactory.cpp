#include "conn\factory\ConnFactory.h"
#include "conn\ExchangeConn.h"
#include "conn\ctp\CTPConnection.h"
#include "conn\ztp\ZTPConnection.h"
#include "conn\esunny\ESUNNYConnection.h"

namespace conn {
	ExchangeConn* createCTP(TradeSystemConfig const& config, ConnPublisher& publisher) {
		static CTPConnection* ctp = new CTPConnection(const_cast<TradeSystemConfig&>(config), publisher);
//		ctp->Init();
		return ctp;
	}
	ExchangeConn* createZTP(TradeSystemConfig const& config, ConnPublisher& publisher) {
		static ZTPConnection* ctp = new ZTPConnection(const_cast<TradeSystemConfig&>(config), publisher);
//		ctp->Init();
		return ctp;
	}
	ExchangeConn* createHKEX_ESUNNY(TradeSystemConfig const& config, ConnPublisher& publisher) {
		static ESUNNYConnection* esunny = new ESUNNYConnection(config, publisher);
//		esunny->Init();
		return esunny;
	}

	ExchangeConn* ConnFactory::Create(TradeSystemConfig const& config, ConnPublisher& publisher) {
		switch (config.Platform) {
		case TradePlatform::CTP:
			return createCTP(config, publisher);
		case TradePlatform::HKEX_ESUNNY:
			return createHKEX_ESUNNY(config, publisher);
		case TradePlatform::ZTP:
			return createZTP(config, publisher);
		default:
			return nullptr;
			break;
		}
	}
}
