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
		double getOpenOffsetPrice() const;
		double getOpenPrice() const;
		bool isAllowedToOpen() const;
		bool tryToOpenOrder();
		void onOpenOrder();
		void onCloseOrder(eOrderState aState, double aProfit);

		stMAlgorithm& algorithm;
		std::pair<std::string, eOrderState> lastClosedOrder;
	};
}
