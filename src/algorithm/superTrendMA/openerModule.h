#pragma once
#include <string>
#include <map>
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
		void onOpenOrder();
		void onCloseOrder(eOrderState state, double profit);

		bool isPrevPositionCorrect() const;

		stMAlgorithm& algorithm;
		std::pair<std::string, eOrderState> lastClosedOrder;
	};
}
