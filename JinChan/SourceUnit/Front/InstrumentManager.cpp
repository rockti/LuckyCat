#include "stdafx.h"
#include "InstrumentManager.h"
InstrumentManager<CInstrumentIDType>& FTDInstance() {
	static InstrumentManager<CInstrumentIDType> s_FTDInstrumentManager;
	return s_FTDInstrumentManager;
}
