#pragma once

namespace calculation {
	class calculationSystem;
}

namespace algorithm {
	class stAlgorithm;
	class activationWaiter {
		friend class stAlgorithm;
	public:
		activationWaiter(stAlgorithm& aAlgorithm);
		bool operator==(const activationWaiter& other) const;
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
