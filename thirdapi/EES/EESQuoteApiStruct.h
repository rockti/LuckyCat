#pragma once
#pragma pack(push, 1)	//压栈时1字节对齐

struct multicast_info {
	char	m_localAddr[32];				///< 组播本机地址
	char	m_remoteAddr[32];				///< 组播行情远端地址
	int		m_remotePort;					///< 组播行情远端端口
	int		m_localPort;					///< 组播本机端口
};

/********************************************高速行情结构********************************************/
struct efh3_2_fut_lev1 {
	unsigned int	m_sequence;				///<会话编号
	char			m_exchange_id;			///<市场  0 表示中金  1表示上期
	char			m_channel_id;			///<通道编号
	char			m_symbol[8];			///<合约
	char			m_update_time_h;		///<最后更新的时间hh
	char			m_update_time_m;		///<最后更新的时间mm
	char			m_update_time_s;		///<最后更新的时间ss
	unsigned short  m_millisecond;		    ///<最后更新的毫秒数         

	double			m_last_px;				///<最新价
	unsigned int	m_last_share;			///<最新成交量
	double			m_total_value;			///<成交金额
	double			m_total_pos;			///<持仓量

	double			m_bid_px;				///<买一价
	unsigned int	m_bid_share;			///<买一量
	double			m_ask_px;				///<卖一价
	unsigned int	m_ask_share;			///<卖一量
	char            m_reserve;  			///<保留字段             
};
/********************************************高速行情结构********************************************/

/********************************************深度行情结构********************************************/
struct efh3_2_fut_lev2 {
	unsigned int	m_sequence;				///<会话编号
	char			m_exchange_id;			///<市场  0 表示中金  1表示上期
	char			m_channel_id;			///<通道编号
	char			m_symbol[8];			///<合约
	char			m_update_time_h;		///<最后更新的时间hh
	char			m_update_time_m;		///<最后更新的时间mm
	char			m_update_time_s;		///<最后更新的时间ss
	unsigned short  m_millisecond;		    ///<最后更新的毫秒数        

	double			m_last_px;				///<最新价
	unsigned int	m_last_share;			///<最新成交量
	double			m_total_value;			///<成交金额
	double			m_total_pos;			///<持仓量

	double			m_bid1_px;				///<买一价
	unsigned int	m_bid1_share;			///<买一量
	double			m_bid2_px;				///<买二价
	unsigned int	m_bid2_share;			///<买二量
	double			m_bid3_px;				///<买三价
	unsigned int	m_bid3_share;			///<买三量
	double			m_bid4_px;				///<买四价
	unsigned int	m_bid4_share;			///<买四量
	double			m_bid5_px;				///<买五价
	unsigned int	m_bid5_share;			///<买五量

	double			m_ask1_px;				///<卖一价
	unsigned int	m_ask1_share;			///<卖一量
	double			m_ask2_px;				///<卖二价
	unsigned int	m_ask2_share;			///<卖二量
	double			m_ask3_px;				///<卖三价
	unsigned int	m_ask3_share;			///<卖三量
	double			m_ask4_px;				///<卖四价
	unsigned int	m_ask4_share;			///<卖四量
	double			m_ask5_px;				///<卖五价
	unsigned int	m_ask5_share;			///<卖五量

	char            m_reserve;  			///<保留字段             
};
/********************************************深度行情结构********************************************/
#pragma pack(pop)