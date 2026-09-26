#pragma once

namespace CommonFunction {
	template<typename T, int N>
	int ArraySize(T(&)[N]) {
		return N;
	}

	inline void EnableWindows(CWnd* wnd[], int count, BOOL enable = TRUE, char const* text = NULL) {
		for (int i = 0; i < count; ++i) {
			wnd[i]->EnableWindow(enable);			
			USES_CONVERSION;
			if (text) wnd[i]->SetWindowText(A2W(text));
		}
	}
	inline void SHowWindows(CWnd* wnd[], int count, BOOL enable = TRUE, char const* text = NULL) {
		for (int i = 0; i < count; ++i) {
			wnd[i]->ShowWindow(enable);			
			USES_CONVERSION;
			if (text) wnd[i]->SetWindowText(A2W(text));
		}
	}

	inline double WindowText2Double(CWnd& wnd) {
		CString text;
		wnd.GetWindowText(text);
		USES_CONVERSION;
		return atof(W2A(text));
	}

	inline uint8_t WindowText2UINT8(CWnd& wnd) {
		CString text;
		wnd.GetWindowText(text);
		USES_CONVERSION;
		return uint8_t(atoi(W2A(text)));
	}

	inline uint16_t WindowText2UINT16(CWnd& wnd) {
		CString text;
		wnd.GetWindowText(text);
		USES_CONVERSION;
		return (uint16_t)(atoi(W2A(text)));
	}

	inline uint32_t WindowText2UINT32(CWnd& wnd) {
		CString text;
		wnd.GetWindowText(text);
		USES_CONVERSION;
		return (uint32_t)atoi(W2A(text));
	}

	inline int WindowText2Int(CWnd& wnd) {
		CString text;
		wnd.GetWindowText(text);
		USES_CONVERSION;
		return atoi(W2A(text));
	}

	inline void Double2WindowText(CWnd& wnd, double value, int precision = 2) {
		char buffer[4096] = "";
		char format[10] = "%";
		sprintf_s(format + 1, 9, ".%df", precision);
		sprintf_s(buffer, format, value);
		USES_CONVERSION;
		wnd.SetWindowText(A2W(buffer));
	}

	inline void Int2WindowText(CWnd& wnd, int value) {
		char buffer[4096] = "";
		sprintf_s(buffer, "%d", value);
		USES_CONVERSION;
		wnd.SetWindowText(A2W(buffer));
	}

	//应该用模板做，这里先这样吧
	inline char const* GetPriceFormat(money unitPrice) {
		if (unitPrice >= 0.1) {
			return "{:.1f}";
		} else if (unitPrice >= 0.01 && unitPrice < 0.1) {
			return "{:.2f}";
		}
		return "{:.3f}";
	}
// 	inline char const* GetDeltaFormat(money unitPrice) {
// 		if (unitPrice >= 1) {
// 			return "{:.1f} s|b {:.1f}";
// 		} else if (unitPrice >= 0.1 && unitPrice < 1) {
// 			return "{:.1f} s|b {:.1f}";
// 		} else if (unitPrice >= 0.01 && unitPrice < 0.1) {
// 			return "{:.2f} s|b {:.2f}";
// 		}
// 		return "{:.3f} s|b {:.3f}";
// 	}

	inline char const* GetDeltaFormat(money unitPrice) {
		if (unitPrice >= 1) {
			return "{:.0f} s| {:.0f} |b {:.0f}";
		}
		else if (unitPrice >= 0.1 && unitPrice < 1) {
			return "{:.1f} s| {:.1f} |b {:.1f}";
		}
		else if (unitPrice >= 0.01 && unitPrice < 0.1) {
			return "{:.2f} s| {:.2f} |b {:.2f}";
		}
		return "{:.3f} s| {:.3f} |b {:.3f}";
	}

	inline char const* GetLinesFormat(money unitPrice) {
		if (unitPrice >= 1) {
			return "{:.0f}-{:.0f} | {:.0f}-{:.0f}";
		} else if (unitPrice >= 0.1 && unitPrice < 1) {
			return "{:.1f}-{:.1f} | {:.1f}-{:.2f}";
		} else if (unitPrice > 0.01 && unitPrice < 0.1) {
			return "{:.2f}-{:.2f} | {:.2f}-{:.2f}";
		}
		return "{:.3f}-{:.3f} | {:.3f}-{:.3f}";
	}
	inline int GetPricePrecision(money unitPrice) {
		if (unitPrice>=0.1 && unitPrice<1 ) {
			return 1;
		} else if (unitPrice>=0.01 && unitPrice<0.1) {
			return 2;
		} else if (unitPrice<0.01) {
			return 3;
		}
		return 0;
	}
}
