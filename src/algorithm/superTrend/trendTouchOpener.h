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
		trendTouchOpener(stAlgorithm& aAlgorithm);
		bool check();
	private:
		stAlgorithm& algorithm;
	};
}
