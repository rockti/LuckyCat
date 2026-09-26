#pragma once
#include <QString.h>
#include "type.hpp"
inline QString ft(char const* str) {
	return QString::fromLocal8Bit(str);
}

template<uint8_t size, class Tag>
QString ft(StrongCharArray<size, Tag> const& s) {
	return QString::fromLocal8Bit(s.value());
}

#include <deque>

class DigitalToChinese {
	struct Section {
		Section(char const* name) :Name(name) {}
		char const* Name;
		bool IsZero() const {
			return isZero_;
		}
		void Zero() {
			isZero_ = true;
		}
		void Value(int v) {
			if (v != 0)
				isZero_ = false;
		}
	private:
		bool isZero_ = true;
	};

	std::vector<Section> m_sections = {
		{""} ,{"万"},{"亿"},{"万"},{"亿"},{"万"},{"亿"},
	};

	enum EBitUnit { Single = 0, Ten = 1, Hundred = 2, Thousand = 3,
	};

	struct Bit {
		Bit(std::vector<Section>& sections, uint8_t v, int bit) {
			unit_ = EBitUnit(bit % 4);
			value_ = v;
			sectionPtr_ = &sections[bit / 4];
			sectionPtr_->Value(v);
		}
		void SetPrev(Bit* prev) {
			Prev = prev;
		}
		void SetFirst() {
			IsFirst = true;
		}

		QString Name() const {
			QString result;
			if (!IsZero()) {
				result += ft(ZeroName()) + ft(ValueName()) + ft(UnitName());
			}
			result += ft(SectionName());
			return result;
		}
		bool IsSection() const { return unit_ == Single; }
		inline static const char* Empty = "";

	private:
		uint8_t value_ = 0;
		EBitUnit unit_ = Single;;
		bool IsFirst = false;
		Section* sectionPtr_ = nullptr;
		Bit* Prev = nullptr;
		bool IsFirstAndTen() const { return IsFirst && value_ == 1 && unit_ == Ten; }
		bool IsZero() const { return value_ == 0; }
		bool IsZeroButSection() const { IsZero() && IsSection(); }
		bool IsPrevZero() const { return Prev && Prev->IsZero(); }
		bool SectionHasValue() const { return !sectionPtr_->IsZero(); };
		const char* ZeroName() const { return IsPrevZero() ? Zero : Empty; }
		const char* ValueName() const {	return IsFirstAndTen() ? Empty : ChineseNumber[value_];}
		const char* UnitName() const { return BitUnitName[int(unit_)]; }
		const char* SectionName() const {
			return IsSection() && SectionHasValue() ? sectionPtr_->Name : "";
		}
		inline static std::vector<char const*> BitUnitName = { "","十","百","千" };
		inline static std::vector<char const*> ChineseNumber = { "零","一","二" ,"三" ,"四" ,"五" ,"六" ,"七" ,"八" ,"九" };
		inline static const char* Zero = "零";

	};

	std::deque<Bit> m_bits;


	void toBits(long long num) {
		for (long long v = num, bit = 0; v != 0; v = v / 10, ++bit) {
			Bit* prev = m_bits.empty() ? nullptr : &m_bits.back();
			Bit b(m_sections, v%10, bit);
			m_bits.push_back(b);
			if (prev) prev->SetPrev(&m_bits.back());
		}
		if (!m_bits.empty()) m_bits.back().SetFirst();
	}
	QString toString() {
		QString number;
		for (auto iter = m_bits.rbegin(); iter != m_bits.rend(); ++iter) {
			number += iter->Name();
		}
		return number;
	}
	void clear() {
		m_bits.clear();
		for (auto& s : m_sections) {
			s.Zero();;
		}
	}
public:

	QString toChinese(long long num) {
		if (num == 0) {
			return ft(Bit::Empty);
		}
		clear();
		toBits(num);
		return toString();
	}

};