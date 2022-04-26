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
		activationWaiter(stAlgorithm& aAlgorithm);
		bool operator==(const activationWaiter& other);
		void onNewTrend();
		void start();
		bool check();
		int getCounter() const { return activationWaitCounter; }
		void setCounter(int aCounter) { activationWaitCounter = aCounter; } // TO DO make private later
	private:
		stAlgorithm& algorithm;
		int activationWaitCounter = 0;
	};
}
