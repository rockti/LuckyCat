#pragma once

#include "model/Instrument.h"

class ExchangeConn;
struct ConnConfig;
class ConnPublisher;
namespace conn {
	class ConnFactory {
	public:
		static ExchangeConn* Create(ConnConfig const& config, Instruments const& instruments, ConnPublisher& publisher);
	};

}