#ifdef UNITTEST

#include <catch2/catch.hpp>
#include "model/dealing/ExchangeRequest.h"


using namespace std;
TEST_CASE("LocalIDACC Test") {
	SECTION("IsMine Test") {
		CHECK(LocalIdACC::IsMine(88000001));
		CHECK_FALSE(LocalIdACC::IsMine(8800000));
		CHECK_FALSE(LocalIdACC::IsMine(8800012));
	}
}

#endif
