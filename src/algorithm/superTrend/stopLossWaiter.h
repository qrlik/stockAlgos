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
		stopLossWaiter(stAlgorithm& aAlgorithm);
		bool operator==(const stopLossWaiter& aOther) const;
		void onNewTrend();
		void start();
		bool check();
		int getCounter() const;
		void setCounter(int aCounter) { stopLossWaitCounter = aCounter; } // TO DO make private later
	private:
		stAlgorithm& algorithm;
		int stopLossWaitCounter = 0;
	};
}
