#pragma once

namespace calculation {
	class calculationSystem;
}

namespace algorithm {
	class moneyMaker;
	class activationWaiter {
		friend class moneyMaker;
		friend class calculation::calculationSystem;
	public:
		activationWaiter(moneyMaker* aMm, int aActivationWaitRange, bool aResetAllowed, bool aFullCandleCheck);
		bool operator==(const activationWaiter& other);
		void onNewTrend();
		void start();
		bool check();
		int getCounter() const;
		void setCounter(int aAmount);
	private:
		moneyMaker* mm = nullptr;
		const int activationWaitRange;
		int activationWaitCounter = 0;
		const bool resetAllowed;
		const bool fullCandleCheck;
	};
}
