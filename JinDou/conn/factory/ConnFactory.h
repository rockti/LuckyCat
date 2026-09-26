#pragma once

#include "model/Instrument.h"

class ExchangeConn;
struct TradeSystemConfig;
class ConnPublisher;
namespace conn {
	class ConnFactory {
	public:
		static ExchangeConn* Create(TradeSystemConfig const& config, ConnPublisher& publisher);
	};

}