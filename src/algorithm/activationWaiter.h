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
	private:
		moneyMaker* mm;
		const size_t activationWaitRange = 0;
		size_t activationWaitCounter = 0;
		const bool resetAllowed = false;
		const bool fullCandleCheck = false;
	};
}
