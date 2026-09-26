#pragma once

#include "model/Constants.h"
#include <array>

#ifdef WIN32
#include <intrin.h>
static uint32_t __inline ctz(uint32_t x) {
	unsigned long r = 0;
	_BitScanReverse(&r, x);
	return r;
}

static uint32_t __inline clz(uint32_t x) {
	unsigned long r = 0;
	_BitScanForward(&r, x);
	return r;
}
#endif

struct Data;
struct ConnCommand;
class ExchangeConn;

class ConnRunnerHandler {
public:
	ConnRunnerHandler(std::array<ExchangeConn*, 10> & conns) :m_conns(conns) {}
	void Handle(Data const& data);

private:
	void onCommand(ConnCommand const& command);
	std::array<ExchangeConn*, 10>& m_conns;
};