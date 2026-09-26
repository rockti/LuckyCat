#include "stdafx.h"
#include <catch.hpp>
#include "RateAlgorithms.h"

#include <sstream>
#include <fmt/format.h>

template<std::size_t Size = 256>
class bumping_memory_resource {
public:
	char buffer[Size];
	char* _ptr;

	explicit bumping_memory_resource()
		: _ptr(&buffer[0]) {
	}

	void* allocate(std::size_t size) noexcept {
		auto ret = _ptr;
		_ptr += size;
		return ret;
	}

	void deallocate(void*) noexcept {}
};

template <typename T, typename Resource = bumping_memory_resource<256>>
class bumping_allocator {
	Resource* _res;

public:
	using value_type = T;

	explicit bumping_allocator(Resource& res)
		: _res(&res) {
	}

	bumping_allocator(const bumping_allocator&) = default;
	template <typename U>
	bumping_allocator(const bumping_allocator<U, Resource>& other)
		: bumping_allocator(other.resource()) {
	}

	Resource& resource() const { return *_res; }

	T* allocate(std::size_t n) { return static_cast<T*>(_res->allocate(sizeof(T) * n)); }
	void deallocate(T* ptr, std::size_t) { _res->deallocate(ptr); }

	friend bool operator==(const bumping_allocator& lhs, const bumping_allocator& rhs) {
		return lhs._res == rhs._res;
	}

	friend bool operator!=(const bumping_allocator& lhs, const bumping_allocator& rhs) {
		return lhs._res != rhs._res;
	}
};

TEST_CASE("Margin & Fee Test") {

	SECTION("arena allocator test") {
		using big_memory_buffer = fmt::basic_memory_buffer<char, 2014>;
		big_memory_buffer bmb;
		fmt::format_to(bmb, "ahhaha {} {}", "dsfasdfsd", 10.0);
		char* t = bmb.data();
		int len = strlen(t);
		CHECK(len == bmb.size());

	}

	SECTION("Margin Test") {
		{
			CAlgorithms algo;
			data::CInvestorMargin margin;
			margin.LongMarginAmt = 10;
			margin.LongMarginRate = 0.1;
			margin.ShortMarginAmt = 9;
			margin.ShortMarginRate = 0.09;
			CHECK(Approx(algo.CalculateMargin(1, 1000, type::Direction::Buy, 1000, &margin)) == 100010.0);
			CHECK(Approx(algo.CalculateMargin(1, 1000, type::Direction::Sell, 1000, &margin)) == 90009.0);
		}

	}
	/*
	//开仓
	#define type::OffsetFlag::Open '0'
	//平仓
	#define type::OffsetFlag::Close '1'
	//强平
	#define type::OffsetFlag::ForceClose '2'
	//平今
	#define type::OffsetFlag::CloseToday '3'
	//平昨
	#define type::OffsetFlag::CloseYesterday '4'
	*/
	SECTION("Fee Test") {
		{
			CAlgorithms algo;
			data::CInvestorFee fee;
			fee.OpenFeeRate = 0.01;
			fee.OpenFeeAmt = 10;
			fee.OffsetFeeRate = 0.02;
			fee.OffsetFeeAmt = 12;
			fee.OffsetTodayFeeRate = 0.015;
			fee.OffsetTodayFeeAmt = 9;
			CHECK(Approx(algo.CalculateFee(1, 1000, type::OffsetFlag::Open, 10, &fee)) == 110);
			CHECK(Approx(algo.CalculateFee(1, 1000, type::OffsetFlag::Close, 10, &fee)) == 212);
			CHECK(Approx(algo.CalculateFee(1, 1000, type::OffsetFlag::CloseToday, 10, &fee)) == 159);
//			CHECK(Approx(algo.CalculateFee(1, 1000, type::OffsetFlag::CloseYesterday, 10, &fee)) == 159);
		}

	}
}