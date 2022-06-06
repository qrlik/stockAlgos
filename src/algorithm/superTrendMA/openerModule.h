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
		bool checkTrendTouch();
		bool tryToOpenOrder(bool aIsTochedThisCandle);
		void onOpenOrder();
		void onCloseOrder(eOrderState aState, double aProfit);

		std::map<int, int> rsiProfitLongs;
		std::map<int, int> rsiUnprofitLongs;
		std::map<int, int> rsiProfitShorts;
		std::map<int, int> rsiUnprofitShorts;
		double lastOpenRsi = 0;

		stMAlgorithm& algorithm;
		std::pair<std::string, eOrderState> lastClosedOrder;

		bool touchActivated = false;
	};
}
