#pragma once
#include "model/Constants.h"
#include "type/ZTPType.h"
#include <fstream>
using namespace ztp;

/*
	如何生成正确的LocalID以避免重复，有以下几个思考
	1.我们是否是唯一应用，如果不是，如何保障OrderLocalID的不重叠，这是最核心问题
	2.如果应用唯一，如何保障应用重启后产生的OrderLocalID是不重复的
	3.是否不同的平台共用全局唯一的OrderLocalID（目前代码是这样设计的，但没有解决问题2)
	4.如果要生成全局唯一，有三种方法: 1.保存到文件 2.使用时间生成前缀 3.查询订单后取最大
	5.保存文件方案是根据TradingDate来保存文件，重启时重新读取，然后增加2000个以保障安全，
	  挑战是如何保障方案的安全。写文件要保障安全只能使用append only 的方式，如果一天订单数不多，
	  是可以考虑的，可能会略微影响性能，使用异步文件IO可以解决（1秒一次flash)
	6.使用时间生成的方式可能使用当前时间与当月（如果是1号就是上月）1号的时间差（秒级）作为前缀，
	  在重启后直接生成，这种方式需要操作系统的时间需要与时间服务器同步，不然差别太大会影响逻辑，
	  同时如果在时间有偏差时与时间服务器同步同时有重启有可能重复
	7.在查询所有平台的订单后取最大的OrderLocalID，然后增加2000个以保障安全，这个方案目前看
	  效果最好，2000的buffer应该可以满足本应用的场景
	8.所以方案三作用最终方案，但需要确认问题1.
	9.查询取最大在订单失败然后重启后有问题，需要增加文件来保障，文件1秒写一次
*/

struct LocalIDStore {
	type::TradingDay TradingDay;
	type::UserOrderLocalID LastLocalID;
};

class LocalIDFileSaver {
public:
	LocalIDFileSaver();

	~LocalIDFileSaver();

	LocalOrderID Load(type::TradingDay const& tradingDay);

	void SaveToFile(LocalOrderID id);
private:
	std::fstream fs_;
	LocalIDStore store_;
	type::OrderLocalID lastSaved_;
	bool write_opened_ = false;
};

class LocalIdACC {
public:
	inline static LocalOrderID NextNormal() {
		LocalOrderID& base = GetBase();
		base += 1;
		return base * 10 + GetSuffix();
	}

	static void Init(LocalOrderID base) {
		LocalOrderID& b = GetBase();
		b = base / 10;
	}

	static void SetSuffix(uint8_t suffix) {
		auto& s = GetSuffix();
		s = suffix;
	}

	inline static LocalOrderID GetCurrentId() { return GetBase() * 10; }

	inline static bool IsMine(LocalOrderID localId) {
		if (localId < INIT_BASE * 10) {
			return false;
		}
		auto id = fmt::format("{}", localId);
		return id.rfind("88", 0) == 0;
	}

	inline static LocalIDFileSaver& Saver() {
		static LocalIDFileSaver s_saver;
		return s_saver;
	}

	inline static void SaveToFile() {
		Saver().SaveToFile(GetCurrentId());
	}
private:
	static constexpr LocalOrderID INIT_BASE = 8800000;
	static LocalOrderID& GetBase() {
		static LocalOrderID s_base = INIT_BASE;
		return s_base;
	}
	static uint8_t& GetSuffix() {
		static uint8_t s_suffix = 0;
		return s_suffix;
	}
};
