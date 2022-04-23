#pragma once

namespace calculation {
	class calculationSystem;
}

namespace algorithm {
	class stAlgorithm;
	class trendBreakOpener {
		friend class stAlgorithm;
		friend class calculation::calculationSystem;
	public:
		trendBreakOpener(stAlgorithm* aMm, bool aEnabled, bool aActivationWaitMode, bool aAlwaysUseNewTrend);
		bool isNewTrendAllowed();
		bool check();
	private:
		stAlgorithm* mm = nullptr;
		const bool enabled;
		const bool activationWaitMode;
		const bool alwaysUseNewTrend;
	};
}
