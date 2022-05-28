#pragma once
#include <string>
#include <utility>

namespace algorithm {
	enum class eOrderState;
	class stMAlgorithm;
	class openerModule {
		friend class stMAlgorithm;
	public:
		openerModule(stMAlgorithm& aAlgorithm);
		bool check();
	private:
		stMAlgorithm& algorithm;
		std::pair<std::string, eOrderState> lastClosedOrder;
	};
}
