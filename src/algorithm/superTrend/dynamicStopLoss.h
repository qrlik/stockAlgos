#pragma once

namespace calculation {
	class calculationSystem;
}

namespace algorithm {
	class stAlgorithm;
	class dynamicStopLoss {
		friend class stAlgorithm;
	public:
		dynamicStopLoss(stAlgorithm& aAlgorithm);
		bool check();
	private:
		bool checkTrend();
		bool checkDynamic();

		stAlgorithm& algorithm;
	};
}
