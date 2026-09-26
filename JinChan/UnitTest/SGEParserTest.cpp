#include "stdafx.h"

#include <catch.hpp>
#include "SGESerializer.h"
#include "BaseDataType.h"
#include "Repository/MemoryRepositoryImpl.h"

using namespace ztp;

namespace {
	template<int l>
	void assign(CStringType<l>& target, string_view const& str) {
		char tmp[1024] = "";
		memcpy(tmp, str.data(), min(str.length(), sizeof(tmp) - 1));
		target = tmp;
	}

	void assign(CIntType& target, string_view const& str) {
		char tmp[1024] = "";
		memcpy(tmp, str.data(), min(str.length(), sizeof(tmp) - 1));
		target = atoi(tmp);
	}

	void assign(CLongType& target, string_view const& str) {
		char tmp[1024] = "";
		memcpy(tmp, str.data(), min(str.length(), sizeof(tmp) - 1));
		target = atoi(tmp);
	}

	void assign(CCharType& target, string_view const& str) {
		target = str[0];
	}

	template<int l, int p>
	void assign(CDoubleType<l, p>& target, string_view const& str) {
		char tmp[256] = "";
		memcpy(tmp, str.data(), min(str.length(), sizeof(tmp) - 1));
		target = atof(tmp);
	}
	void assign(char* target, string_view const& str) {
		char tmp[1024] = "";
		memcpy(tmp, str.data(), min(str.length(), sizeof(tmp) - 1));
		strcpy_s(target, str.length() + 1, tmp);
	}

	void assign(char& target, string_view const& str) {
		target = str[0];
	}

	void assign(long long& target, string_view const& str) {
		target = _atoi64(str.data());
	}

	void assign(int& target, string_view const& str) {
		target = atoi(str.data());
	}

	template<class tag>
	void assign(DoubleType<tag>& target, string_view const& str) {
		char tmp[256] = "";
		memcpy(tmp, str.data(), min(str.length(), sizeof(tmp) - 1));
		target = atof(tmp);
	}

	template<class tag>
	void assign(RatioType<tag>& target, string_view const& str) {
		char tmp[256] = "";
		memcpy(tmp, str.data(), min(str.length(), sizeof(tmp) - 1));
		target = atof(tmp);
	}
	void assign(double& target, string_view const& str) {
		char tmp[256] = "";
		memcpy(tmp, str.data(), min(str.length(), sizeof(tmp) - 1));
		target = atof(tmp);
	}

	template<uint8_t size, class tag>
	void assign(StrongCharArray<size, tag>& target, string_view const& str) {
		char tmp[256] = "";
		memcpy(tmp, str.data(), min(str.length(), sizeof(tmp) - 1));
		target = tmp;
	}

	template<class BaseT, class tag>
	void assign(NumberType<BaseT, tag>& target, string_view const& str) {
		char tmp[256] = "";
		memcpy(tmp, str.data(), min(str.length(), sizeof(tmp) - 1));
		target = atoi(tmp);
	}
}

template<class Target>
bool copy_if_key(char const* expectedKey, string_view const& key, Target& target, Parser& parser) {
	if (expectedKey == key) {
		assign(target, parser.GetStringValue());
		return true;
	}
	return false;
}
inline bool CopyErrorID(char const* ekey, string_view const& key, type::ErrorID& errorID, Parser& parser) {
	if (key == ekey) {
		errorID = parser.GetIntValue(3);
		return true;
	}
	return false;
}
#define COPY_CONTINUE(ekey,target) if(copy_if_key(ekey, key, target, parser)) continue;
#define COPY_ERRORID(ekey,target) if (CopyErrorID(ekey, key, target,parser)) continue;

/*
static MyInstrumentRepository instRepo;

bool SpotVarityCodeToInstrumentID(int varityCode, ztp::type::InstrumentID& instID) {
	switch (varityCode) {
	case 201:
		instID = "Au99.99";
		break;
	case 202:
		instID = "Au99.95";
		break;
	case 203:
		instID = "Pt99.95";
		break;
	case 204:
		instID = "Au50g";
		break;
	case 207:
		instID = "Au100g";
		break;
	case 210:
		instID = "Au99.5";
		break;
	case 301:
		instID = "iAu99.99";
		break;
	case 302:
		instID = "iAu99.5";
		break;
	case 303:
		instID = "iAu100g";
		break;
	default:
		return false;
		break;
	}
	return true;
}


type::Volume SpotWeightToVolume(type::InstrumentID instID, double weight) {
	auto* pInst = instRepo.Get(instID);
	if (pInst == nullptr) {
		Log::Error(fmt::format("SGEHandler spotWeightToVolume: can not find spot instruemnt[{}]", instID));
		return 0;
	}
	auto volume = weight * 1000 / pInst->VolumeMultiple;
	if (volume < 1) {
		Log::Error(fmt::format("SGEHandler spotWeightToVolume: can not convert InstrumentID[{}] Weight[{}] Multiple[{}]",
			instID, weight, pInst->VolumeMultiple));
		return 0;
	}
	return int(volume);
}
*/

TEST_CASE("SGEParserTest", "SGEParserTest") {

	std::map <type::ClientID, type::InvestorID > m_clientInvestorMap;


	SECTION("Storage Parse Test") {
		Parser parser;
//		char const* testMsg = "X01=GTP1.0,X04=3,X05=8,X03=S802,X06=L,M00=0074,M20=007421,T13=20200410,T80=2471,K04=RK20200410000006,M30=0100041882,K01=1,V00=201,W00=8201,T52=16.000000,T50=15.998400,S16=16.000000,T16=20200410,T17=09:29:46";
		char const* testMsg = "X01=GTP1.0,X04=3,X05=3,X03=S802,X06=L,M00=0074,M20=007421,T13=20200511,T80=1441,K04=TH20200511000019,M30=0100180099,K01=2,V00=201,W00=9998,T52=15.000000,T50=15.000000,S16=15.000000,T16=20200511,T17=10:05:17?";
		parser.Init(testMsg);
		parser.GotoNextKey();
		parser.GotoNextKey();
		parser.GotoNextKey();
		parser.GotoNextKey();

		data::CSpotPositionNotify spotPos;
		spotPos.ExchangeID = "SGE";

		type::ClientID clientID;
		double weight;
		int varityCode;
		StrongCharArray <3, struct TranTypeTag> storageTransType;

		for (auto hasKey = parser.GotoNextKey(); hasKey; hasKey = parser.GotoNextKey()) {
			auto key = parser.GetKey();
			COPY_CONTINUE("M30", clientID);
			COPY_CONTINUE("T52", weight);
			COPY_CONTINUE("V00", varityCode);
			COPY_CONTINUE("K01", storageTransType);
			COPY_CONTINUE("T17", spotPos.NotifyTime);
		}
		if (storageTransType == "1") { //Èë¿â
			spotPos.OffsetFlag = type::OffsetFlag::Open;
		} else if (storageTransType == "K") { //³ö¿â
			spotPos.OffsetFlag = type::OffsetFlag::Close;
		} else {
			Log::Info(fmt::format("SGEHandler pasreStorageRtn: unsupport storage trans type varityCode[{}] type[{}]", varityCode, storageTransType));
			return;
		}
		Log::Info(fmt::format("SGEHandler parseStorageRtn: InvestorID[{}] InstrumentID[{}] OffsetFlag[{}] Volume[{}] weight[{}]",
			spotPos.InvestorID, spotPos.InstrumentID, spotPos.OffsetFlag, spotPos.Volume, weight));
	}

}