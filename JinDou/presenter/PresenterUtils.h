#pragma once

#include "service/Service.h"
namespace ui {
	template<class P>
	class ViewBase {
	public:
		virtual void SetPresenter(P* presenter) {
			presenter_ = presenter;
		}
	protected:
		P* presenter_;
	};
}