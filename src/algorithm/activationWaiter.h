#pragma once

namespace algorithm {
	class moneyMaker;
	class activationWaiter {
	public:
		activationWaiter(moneyMaker* aMm, size_t aActivationWaitRange, bool aResetAllowed, bool aFullCandleCheck);
		bool operator==(const activationWaiter& other);
		void onNewTrend();
		void start();
		bool check();
		size_t getCounter() const;
	private:
		moneyMaker* mm = nullptr;
		const int activationWaitRange;
		int activationWaitCounter = 0;
		const bool resetAllowed;
		const bool fullCandleCheck;
	};
}
