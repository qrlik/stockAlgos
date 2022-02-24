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
		size_t getCounter() const;
		void setCounter(size_t aAmount);
	private:
		moneyMaker* mm = nullptr;
		const int stopLossWaitRange;
		int stopLossWaitCounter = 0;
		const bool enabled;
		const bool resetAllowed;
		const bool fullCandleCheck;
	};
}
