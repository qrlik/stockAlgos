#pragma once

namespace algorithm {
	class stMAlgorithm;
	class closerModule {
		friend class stMAlgorithm;
	public:
		closerModule(stMAlgorithm& aAlgorithm);
		bool check();
	private:
		stMAlgorithm& algorithm;
	};
}
