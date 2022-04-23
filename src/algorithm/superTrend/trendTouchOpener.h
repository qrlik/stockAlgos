#pragma once

namespace calculation {
	class calculationSystem;
}

namespace algorithm {
	class stAlgorithm;
	class trendTouchOpener {
		friend class stAlgorithm;
		friend class calculation::calculationSystem;
	public:
		trendTouchOpener(stAlgorithm* aMm, bool aActivationWaitMode);
		bool check();
	private:
		stAlgorithm* mm = nullptr;
		const bool activationWaitMode;
	};
}
