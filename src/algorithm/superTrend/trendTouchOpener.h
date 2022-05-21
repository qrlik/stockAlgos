#pragma once

namespace algorithm {
	class stAlgorithm;
	class trendTouchOpener {
		friend class stAlgorithm;
	public:
		trendTouchOpener(stAlgorithm& aAlgorithm);
		bool check();
	private:
		stAlgorithm& algorithm;
	};
}
