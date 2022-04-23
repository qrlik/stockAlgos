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
		dynamicStopLoss(stAlgorithm* aMm, double aPercent, bool aTrendMode);
		bool check();
	private:
		bool checkTrend();
		bool checkDynamic();

		stAlgorithm* mm = nullptr;
		const double dynamicSLPercent;
		const bool trendMode;
	};
}
