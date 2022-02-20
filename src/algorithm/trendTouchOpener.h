#pragma once

namespace algorithm {
	class moneyMaker;
	class trendTouchOpener {
	public:
		trendTouchOpener(moneyMaker* aMm, bool aActivationWaitMode);
		bool check();
	private:
		moneyMaker* mm = nullptr;
		const bool activationWaitMode = false;
	};
}
