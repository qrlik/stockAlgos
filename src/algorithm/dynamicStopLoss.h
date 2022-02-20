#pragma once

namespace algorithm {
	class moneyMaker;
	class dynamicStopLoss {
	public:
		dynamicStopLoss(moneyMaker* aMm, double aPercent, bool aTrendMode);
		bool check();
	private:
		bool checkTrend();
		bool checkDynamic();

		moneyMaker* mm = nullptr;
		const double dynamicSLPercent = 0.0;
		const bool trendMode = false;
	};
}
