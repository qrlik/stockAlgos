#pragma once

namespace calculation {
	class calculationSystem;
}

namespace algorithm {
	class stAlgorithm;
	class trendBreakOpener {
		friend class stAlgorithm;
	public:
		trendBreakOpener(stAlgorithm& aAlgorithm);
		bool isNewTrendAllowed();
		bool check();
	private:
		stAlgorithm& algorithm;
	};
}
