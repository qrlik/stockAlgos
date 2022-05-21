#pragma once

namespace algorithm {
	class stAlgorithm;
	class stopLossWaiter {
		friend class stAlgorithm;
	public:
		stopLossWaiter(stAlgorithm& aAlgorithm);
		bool operator==(const stopLossWaiter& aOther) const;
		void onNewTrend();
		void start();
		bool check();
		int getCounter() const;
	private:
		stAlgorithm& algorithm;
		int stopLossWaitCounter = 0;
	};
}
