#pragma once

namespace calculation {
	class calculationSystem;
}

namespace algorithm {
	class moneyMaker;
	class trendBreakOpener {
		friend class moneyMaker;
		friend class calculation::calculationSystem;
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
