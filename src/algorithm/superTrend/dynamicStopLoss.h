#pragma once

namespace calculation {
	class calculationSystem;
}

namespace algorithm {
	class stAlgorithm;
	class dynamicStopLoss {
		friend class stAlgorithm;
		friend class calculation::calculationSystem;
	public:
		dynamicStopLoss(stAlgorithm& aAlgorithm);
		bool check();
	private:
		bool checkTrend();
		bool checkDynamic();

		stAlgorithm& algorithm;
	};
}
