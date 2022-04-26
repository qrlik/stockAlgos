#pragma once

namespace calculation {
	class calculationSystem;
}

namespace algorithm {
	class stAlgorithm;
	class trendBreakOpener {
		friend class stAlgorithm;
		friend class calculation::calculationSystem; // TO DO fix this everywhere;
	public:
		trendBreakOpener(stAlgorithm& aAlgorithm);
		bool isNewTrendAllowed();
		bool check();
	private:
		stAlgorithm& algorithm;
	};
}
