#pragma once
// 
// #include "type/Math.h"
// class money {
// public:
// 	money() : val(0.00) {}
// 	money(double v) : val(v) {}
// 	explicit money(int v) : val(v) {}
// 	money(money const& other) :money(other.val) {}
// 	money operator=(double v) { val = v; return *this; }
// 
// 	money& operator += (double v) { val += v; return *this; }
// 	money& operator -= (double v) { val -= v; return *this; }
// 	money& operator *= (double v) { val *= v; return *this; }
// 	money& operator /= (double v) { val /= v; return *this; }
// 
// 	money& operator += (money const& v) { val += v.value(); return *this; }
// 	money& operator -= (money const& v) { val -= v.value(); return *this; }
// 	money& operator *= (money const& v) { val *= v.value(); return *this; }
// 	money& operator /= (money const& v) { val /= v.value(); return *this; }
// 
// 	bool operator == (double v) { return DoubleComparer::equal(val, v); }
// 	bool operator > (double v) { return DoubleComparer::greater(val, v); }
// 	bool operator >= (double v) { return DoubleComparer::greater_equal(val, v); }
// 	bool operator < (double v) { return DoubleComparer::less(val, v); }
// 	bool operator <= (double v) { return DoubleComparer::less_equal(val, v); }
// 
// 	explicit operator int() { return int(val); }
// 	explicit operator double() { return val; }
// 
// 	double value() const { return val; }
// private:
// 	double val;
// };
// 
// inline money operator + (money const& m, money const& v) { return money(m.value() + v.value()); }
// inline money operator - (money const& m, money const& v) { return money(m.value() - v.value()); }
// inline money operator * (money const& m, money const& v) { return money(m.value() * v.value()); }
// inline money operator / (money const& m, money const& v) { return money(m.value() / v.value()); }
// 
// inline money operator + (money const& m, double v) { return money(m.value() + v); }
// inline money operator - (money const& m, double v) { return money(m.value() - v); }
// inline money operator * (money const& m, double v) { return money(m.value() * v); }
// inline money operator / (money const& m, double v) { return money(m.value() / v); }
// 
// inline money operator + (double m, money const& v) { return money(m + v.value()); }
// inline money operator - (double m, money const& v) { return money(m - v.value()); }
// inline money operator * (double m, money const& v) { return money(m * v.value()); }
// inline money operator / (double m, money const& v) { return money(m / v.value()); }
// 
// inline bool operator == (money const& r, money const& l) { return DoubleComparer::equal(r.value(), l.value()); }
// inline bool operator > (money const& r, money const& l) { return DoubleComparer::greater(r.value(), l.value()); }
// inline bool operator >= (money const& r, money const& l) { return DoubleComparer::greater_equal(r.value(), l.value()); }
// inline bool operator < (money const& r, money const& l) { return DoubleComparer::less(r.value(), l.value()); }
// inline bool operator <= (money const& r, money const& l) { return DoubleComparer::less_equal(r.value(), l.value()); }
// 
// inline bool operator == (double r, money const& l) { return DoubleComparer::equal(r, l.value()); }
// inline bool operator > (double r, money const& l) { return DoubleComparer::greater(r,l.value()); }
// inline bool operator >= (double r, money const& l) { return DoubleComparer::greater_equal(r, l.value()); }
// inline bool operator < (double r, money const& l) { return DoubleComparer::less(r, l.value()); }
// inline bool operator <= (double r, money const& l) { return DoubleComparer::less_equal(r, l.value()); }
// 
// inline bool operator == (money const& r, double l) { return DoubleComparer::equal(r.value(), l); }
// inline bool operator > (money const& r, double l) { return DoubleComparer::greater(r.value(), l); }
// inline bool operator >= (money const& r, double l) { return DoubleComparer::greater_equal(r.value(), l); }
// inline bool operator < (money const& r, double l) { return DoubleComparer::less(r.value(), l); }
// inline bool operator <= (money const& r, double l) { return DoubleComparer::less_equal(r.value(), l); }
