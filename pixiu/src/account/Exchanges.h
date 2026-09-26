#pragma once

class RealExchange { 
public:
private:
	ExchangeConn* m_conn;
	Positions m_position;
};

class Exchanges {
public:


private:
	std::array<ExchangeConn*, 5> m_conns;
};