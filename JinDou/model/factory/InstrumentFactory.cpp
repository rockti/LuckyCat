#include "model/factory/InstrumentFactory.h"
#include <chrono>

static Instruments s_instruments;


Instruments const& InstrumentFactory::GetInstruments() {
	return s_instruments;
}

void InstrumentFactory::Add(Instrument const& instrument) {
	type::InstID instID = InstIDGen(instrument.Exchange, (int)s_instruments.Count());
	Instrument inst = instrument;
	inst.InstID = instID;
	s_instruments.Add(instrument);
}

