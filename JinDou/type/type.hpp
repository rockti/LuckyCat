#pragma once
#include <cstdint>
#include <cstddef>
#include <cmath>
#include <cstring>
#include <string>

#define DEFINE_HAS(Name) \
template <typename T, typename = int>								\
struct Has_##Name : std::false_type {};								\
																	\
template <typename T>												\
struct Has_##Name<T, decltype((void)T::x, 0)> : std::true_type {};	

DEFINE_HAS(InvestorID);

namespace DoubleComparer {
	const double min_epsilon = 0.000001;
	typedef bool (*CompareFunction)(double l, double r, double epsilon);

	inline bool equal(double l, double r, double epsilon = min_epsilon) {
		return fabs(l - r) < epsilon;
	}

	inline bool less(double l, double r, double epsilon = min_epsilon) {
		double difference = r - l;
		return difference > epsilon;
	}

	inline bool greater(double l, double r, double epsilon = min_epsilon) {
		double difference = l - r;
		return difference > epsilon;
	}

	inline bool less_equal(double l, double r, double epsilon = min_epsilon) {
		return less(l, r, epsilon) || equal(l, r, epsilon);
	}

	inline bool greater_equal(double l, double r, double epsilon = min_epsilon) {
		return greater(l, r, epsilon) || equal(l, r, epsilon);
	}

	inline double GetLess(double l, double r, double espilon = min_epsilon) {
		if (less(l, r, espilon)) {
			return l;
		}
		return r;
	}

	inline double GetGreater(double l, double r, double espilon = min_epsilon) {
		if (greater(l, r, espilon)) {
			return l;
		}
		return r;
	}
}

template<uint16_t size, typename tag>
struct StrongCharArray {

	StrongCharArray() {}
	explicit StrongCharArray(char const* other) {
		set_value(other);
	}
	template<int size>
	explicit StrongCharArray(char other[size]) {
		set_value(other);
	}
	StrongCharArray(StrongCharArray const& other) {
		set_value(other.data_);
	}

	inline char at(int index) const {
		return data_[index];
	}

	inline char const* value() const {
		return data_;
	}
	inline auto& data() {
		return data_;
	}
	inline uint16_t length() const {
		return static_cast<uint16_t>(strlen(data_));
	}
	inline bool empty() const {
		return strlen(data_) <= 0;
	}
	inline StrongCharArray& operator=(char const* other) {
		set_value(other);
		return *this;
	}
	inline StrongCharArray& operator=(std::string const& other) {
		set_value(other.c_str());
		return *this;
	}
	inline StrongCharArray& operator=(StrongCharArray const& other) {
		set_value(other.data_);
		return *this;
	}

	inline bool operator==(char const* other) const {
		return std::strcmp(data_, other) == 0;
	}

	inline bool operator!=(char const* other) const {
		return  std::strcmp(data_, other) != 0;
	}

	inline bool operator==(std::string const& other) const {
		return other == data_;
	}

	inline bool operator!=(std::string const& other) const {
		return  other != data_;
	}

	inline bool operator==(StrongCharArray const& other) const {
		return std::strcmp(data_, other.data_) == 0;
	}

	inline bool operator!=(StrongCharArray const& other) const {
		return  std::strcmp(data_, other.data_) != 0;
	}
	inline bool operator<(StrongCharArray const& other) const {
		return  std::strcmp(data_, other.data_) < 0;
	}

	inline bool operator>(StrongCharArray const& other) const {
		return  std::strcmp(data_, other.data_) > 0;
	}
	inline bool operator<=(StrongCharArray const& other) const {
		return  std::strcmp(data_, other.data_) <= 0;
	}

	inline bool operator>=(StrongCharArray const& other) const {
		return  std::strcmp(data_, other.data_) >= 0;
	}

private:
	void set_value(char const* v) {
		if (v == nullptr) {
			data_[0] = '\0';
		}
		strncpy_s(data_, v, size-1);
		data_[size - 1] = '\0';
	}
	char data_[size] = "";
};

template<typename BaseT, typename tag>
struct NumberType {

	NumberType() : value_(0) {}
	NumberType(BaseT v) : value_(v) {}
	NumberType(NumberType const& other) :value_(other.value_) {}
	inline NumberType& operator=(BaseT v) { value_ = v; return *this; }

	inline NumberType& operator += (BaseT v) { value_ += v; return *this; }
	inline NumberType& operator -= (BaseT v) { value_ -= v; return *this; }
	inline NumberType& operator *= (BaseT v) { value_ *= v; return *this; }
	inline NumberType& operator /= (BaseT v) { value_ /= v; return *this; }

	inline NumberType& operator += (NumberType const& v) { value_ += v.value(); return *this; }
	inline NumberType& operator -= (NumberType const& v) { value_ -= v.value(); return *this; }
	inline NumberType& operator *= (NumberType const& v) { value_ *= v.value(); return *this; }
	inline NumberType& operator /= (NumberType const& v) { value_ /= v.value(); return *this; }

// 	inline bool operator == (BaseT v) { return value_ == v; }
// 	inline bool operator != (BaseT v) { return value_ != v; }
// 	inline bool operator > (BaseT v) { return value_ > v; }
// 	inline bool operator >= (BaseT v) { return value_ >= v; }
// 	inline bool operator < (BaseT v) { return value_ < v; }
// 	inline bool operator <= (BaseT v) { return value_ <= v; }

// 	inline bool operator == (NumberType v) { return value_ == v.value_; }
// 	inline bool operator != (NumberType v) { return value_ != v.value_; }
// 	inline bool operator > (NumberType v) { return value_ > v.value_; }
// 	inline bool operator >= (NumberType v) { return value_ >= v.value_; }
// 	inline bool operator < (NumberType v) { return value_ < v.value_; }
// 	inline bool operator <= (NumberType v) { return value_ <= v.value_; }


	inline operator const BaseT() const { return value_; }
	inline BaseT value() const { return value_; }

private:
	BaseT value_ = 0;
};


template<typename tag>
class DoubleType {
public:
	DoubleType() : value_(0.00) {}
	DoubleType(double v) : value_(v) {}
	explicit DoubleType(int v) : value_(v) {}
	DoubleType(DoubleType const& other) :DoubleType(other.value_) {}
	inline DoubleType& operator=(DoubleType const& v) { value_ = v.value_; return *this; }
	inline DoubleType& operator=(double v) { value_ = v; return *this; }

	inline DoubleType& operator += (double v) { value_ += v; return *this; }
	inline DoubleType& operator -= (double v) { value_ -= v; return *this; }
	inline DoubleType& operator *= (double v) { value_ *= v; return *this; }
	inline DoubleType& operator /= (double v) { value_ /= v; return *this; }

	inline DoubleType& operator += (DoubleType const& v) { value_ += v.value(); return *this; }
	inline DoubleType& operator -= (DoubleType const& v) { value_ -= v.value(); return *this; }
	inline DoubleType& operator *= (DoubleType const& v) { value_ *= v.value(); return *this; }
	inline DoubleType& operator /= (DoubleType const& v) { value_ /= v.value(); return *this; }

	inline bool operator == (DoubleType const& v) { 
		return DoubleComparer::equal(value_, v.value()); 
	}
	inline bool operator == (double v) { return DoubleComparer::equal(value_, v); }
	inline bool operator != (double v) { return !DoubleComparer::equal(value_, v); }
	inline bool operator > (double v) { return DoubleComparer::greater(value_, v); }
	inline bool operator >= (double v) { return DoubleComparer::greater_equal(value_, v); }
	inline bool operator < (double v) { return DoubleComparer::less(value_, v); }
	inline bool operator <= (double v) { return DoubleComparer::less_equal(value_, v); }

	inline explicit operator int() { return int(value_); }
	inline operator const double() const { return value_; }

	inline double value() const { return value_; }
private:
	double value_ = 0.0;
};

template<typename tag>
class RatioType {
public:
	RatioType() : value_(0.00) {}
	RatioType(double v) : value_(v) {}
	explicit RatioType(int v) : value_(v) {}
	RatioType(RatioType const& other) :RatioType(other.value_) {}
	inline RatioType operator=(double v) { value_ = v; return *this; }

	inline RatioType& operator += (double v) { value_ += v; return *this; }
	inline RatioType& operator -= (double v) { value_ -= v; return *this; }
	inline RatioType& operator *= (double v) { value_ *= v; return *this; }
	inline RatioType& operator /= (double v) { value_ /= v; return *this; }

	inline RatioType& operator += (RatioType const& v) { value_ += v.value(); return *this; }
	inline RatioType& operator -= (RatioType const& v) { value_ -= v.value(); return *this; }
	inline RatioType& operator *= (RatioType const& v) { value_ *= v.value(); return *this; }
	inline RatioType& operator /= (RatioType const& v) { value_ /= v.value(); return *this; }

	inline bool operator == (double v) { return DoubleComparer::equal(value_, v); }
	inline bool operator != (double v) { return !DoubleComparer::equal(value_, v); }
	inline bool operator > (double v) { return DoubleComparer::greater(value_, v); }
	inline bool operator >= (double v) { return DoubleComparer::greater_equal(value_, v); }
	inline bool operator < (double v) { return DoubleComparer::less(value_, v); }
	inline bool operator <= (double v) { return DoubleComparer::less_equal(value_, v); }

	inline explicit operator int() { return int(value_); }
	inline operator const double() const { return value_; }

	inline double value() const { return value_; }
private:
	double value_;
};
