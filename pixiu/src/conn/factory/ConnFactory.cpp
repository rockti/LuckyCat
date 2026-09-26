#include "pch.h"

#include "conn\factory\ConnFactory.h"
#include "conn\ExchangeConn.h"
#include "conn\ctp\CTPConnection.h"
#include "conn\esunny\ESUNNYConnection.h"
#include "conn\hft\HFTConnection.h"
#include "conn\sge\SGEConnection.h"

namespace conn {
	ExchangeConn* createHKEX_ESUNNY(ConnConfig const& config, Instruments const& instruments, ConnPublisher& publisher) {
		static ESUNNYConnection* esunny = new ESUNNYConnection(config, instruments, publisher);
		esunny->Init();
		return esunny;
	}

	ExchangeConn* createCTP(ConnConfig const& config, Instruments const& instruments, ConnPublisher& publisher) {
		static CTPConnection* ctp = new CTPConnection(const_cast<ConnConfig&>(config), instruments, publisher);
		ctp->Init();
		return ctp;
	}

	ExchangeConn* createHFT(ConnConfig const& config, Instruments const& instruments, ConnPublisher& publisher) {
		static HFTConnection* hft = new HFTConnection(const_cast<ConnConfig&>(config), instruments, publisher);
		hft->Init();
		return hft;
	}

	ExchangeConn* createSGE(ConnConfig const& config, Instruments const& instruments, ConnPublisher& publisher) {
		static SGEConnection* sge = new SGEConnection(const_cast<ConnConfig&>(config), instruments, publisher);
		sge->Init();
		return sge;
	}

	ExchangeConn* ConnFactory::Create(ConnConfig const& config, Instruments const& instruments, ConnPublisher& publisher) {
		switch (config.Platform) {
		case ConnPlatform::CTP:
			return createCTP(config, instruments, publisher);
		case ConnPlatform::HKEX_ESUNNY:
			return createHKEX_ESUNNY(config, instruments, publisher);
		case ConnPlatform::HFT:	
			return createHFT(config, instruments, publisher);
		case ConnPlatform::SGE:
			return createSGE(config, instruments, publisher);
		default:
			return nullptr;
			break;
		}
	}
}
