#pragma once

namespace algorithm {
	class moneyMaker;
	class stopLossWaiter {
	public:
		stopLossWaiter(moneyMaker* aMm, size_t aStopLossWaitRange, bool aEnabled, bool aResetAllowed, bool aFullCandleCheck);
		bool operator==(const stopLossWaiter& other);
		void onNewTrend();
		void start();
		bool check();
	private:
		moneyMaker* mm = nullptr;
		const size_t stopLossWaitRange = 0;
		size_t stopLossWaitCounter = 0;
		const bool enabled = false;
		const bool resetAllowed = false;
		const bool fullCandleCheck = false;
	};
}
