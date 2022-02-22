#pragma once

namespace algorithm {
	class moneyMaker;
	class trendBreakOpener {
	public:
		trendBreakOpener(moneyMaker* aMm, bool aEnabled, bool aActivationWaitMode, bool aAlwaysUseNewTrend);
		bool isNewTrendAllowed();
		bool check();
	private:
		moneyMaker* mm = nullptr;
		const bool enabled;
		const bool activationWaitMode;
		const bool alwaysUseNewTrend;
	};
}
