// pch.h: 这是预编译标头文件。
// 下方列出的文件仅编译一次，提高了将来生成的生成性能。
// 这还将影响 IntelliSense 性能，包括代码完成和许多代码浏览功能。
// 但是，如果此处列出的文件中的任何一个在生成之间有更新，它们全部都将被重新编译。
// 请勿在此处添加要频繁更新的文件，这将使得性能优势无效。

#ifndef PCH_H
#define PCH_H

#pragma warning( disable : 5053 )
// 添加要在此处预编译的标头
#include "myafx.h"
#include <fmt/format.h>
#include <vector>
#include <map>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <array>
#include <cassert>
#include <sstream>
#include <fstream>
#include <afxcontrolbars.h>
#include <afxcontrolbars.h>
#include <afxcontrolbars.h>
#include <afxcontrolbars.h>
#include <afxcontrolbars.h>
#include <afxcontrolbars.h>
#include <afxcontrolbars.h>

#include "type/Money.h"
#include "type/CharArray.h"


template<> struct fmt::formatter<money> {
	formatter<double> double_formatter;

	template <typename ParseContext>
	constexpr auto parse(ParseContext& ctx) {
		return double_formatter.parse(ctx);
	}

	template <typename FormatContext>
	auto format(const money& s, FormatContext& ctx) {
		return double_formatter.format(s.value(), ctx);
	}
};
#endif //PCH_H
