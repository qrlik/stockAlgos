#pragma once

namespace algorithm {
	class moneyMaker;
	class stopLossWaiter {
		friend class moneyMaker;
	public:
		stopLossWaiter(moneyMaker* aMm, int aStopLossWaitRange, bool aEnabled, bool aResetAllowed, bool aFullCandleCheck);
		bool operator==(const stopLossWaiter& other);
		void onNewTrend();
		void start();
		bool check();
		int getCounter() const;
		void setCounter(int aAmount);
	private:
		moneyMaker* mm = nullptr;
		const int stopLossWaitRange;
		int stopLossWaitCounter = 0;
		const bool enabled;
		const bool resetAllowed;
		const bool fullCandleCheck;
	};
}
