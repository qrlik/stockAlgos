#pragma once

namespace algorithm {
	class stMAlgorithm;
	class openerModule {
		friend class stMAlgorithm;
	public:
		openerModule(stMAlgorithm& aAlgorithm);
		bool check();
	private:
		stMAlgorithm& algorithm;
	};
}
