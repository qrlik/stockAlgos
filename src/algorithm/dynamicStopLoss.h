#pragma once

namespace calculation {
	class calculationSystem;
}

namespace algorithm {
	class moneyMaker;
	class dynamicStopLoss {
		friend class moneyMaker;
		friend class calculation::calculationSystem;
	public:
		dynamicStopLoss(moneyMaker* aMm, double aPercent, bool aTrendMode);
		bool check();
	private:
		bool checkTrend();
		bool checkDynamic();

		moneyMaker* mm = nullptr;
		const double dynamicSLPercent;
		const bool trendMode;
	};
}
