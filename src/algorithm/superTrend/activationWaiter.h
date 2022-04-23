#pragma once

namespace calculation {
	class calculationSystem;
}

namespace algorithm {
	class stAlgorithm;
	class activationWaiter {
		friend class stAlgorithm;
		friend class calculation::calculationSystem;
	public:
		activationWaiter(stAlgorithm* aMm, int aActivationWaitRange, bool aResetAllowed, bool aFullCandleCheck);
		bool operator==(const activationWaiter& other);
		void onNewTrend();
		void start();
		bool check();
		int getCounter() const;
		void setCounter(int aAmount);
	private:
		stAlgorithm* mm = nullptr;
		const int activationWaitRange;
		int activationWaitCounter = 0;
		const bool resetAllowed;
		const bool fullCandleCheck;
	};
}
