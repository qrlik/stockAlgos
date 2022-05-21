#pragma once

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
	private:
		stAlgorithm& algorithm;
		int activationWaitCounter = 0;
	};
}
