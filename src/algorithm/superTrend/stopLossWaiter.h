#pragma once

namespace calculation {
	class calculationSystem;
}

namespace algorithm {
	class stAlgorithm;
	class stopLossWaiter {
		friend class stAlgorithm;
		friend class calculation::calculationSystem;
	public:
		stopLossWaiter(stAlgorithm* aMm, int aStopLossWaitRange, bool aEnabled, bool aResetAllowed, bool aFullCandleCheck);
		bool operator==(const stopLossWaiter& other);
		void onNewTrend();
		void start();
		bool check();
		int getCounter() const;
		void setCounter(int aAmount);
	private:
		stAlgorithm* mm = nullptr;
		const int stopLossWaitRange;
		int stopLossWaitCounter = 0;
		const bool enabled;
		const bool resetAllowed;
		const bool fullCandleCheck;
	};
}
