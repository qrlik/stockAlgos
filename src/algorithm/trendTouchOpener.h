#pragma once

namespace calculation {
	class calculationSystem;
}

namespace algorithm {
	class moneyMaker;
	class trendTouchOpener {
		friend class moneyMaker;
		friend class calculation::calculationSystem;
	public:
		trendTouchOpener(moneyMaker* aMm, bool aActivationWaitMode);
		bool check();
	private:
		moneyMaker* mm = nullptr;
		const bool activationWaitMode;
	};
}
