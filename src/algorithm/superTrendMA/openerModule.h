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
		double getActivationPrice() const;
		double getDeactivationPrice() const;
		double getOpenPrice(bool aIsTochedThisCandle) const;
		bool checkTrendTouch();
		bool tryToOpenOrder(bool aIsTochedThisCandle);
		void onOpenOrder();
		void onCloseOrder(eOrderState aState, double aProfit);

		stMAlgorithm& algorithm;
		std::pair<std::string, eOrderState> lastClosedOrder;

		bool touchActivated = false;
	};
}
