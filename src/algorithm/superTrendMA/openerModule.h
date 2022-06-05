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
		bool checkTrendTouch();
		bool tryToOpenOrder(bool aIsTochedThisCandle);

		stMAlgorithm& algorithm;
		std::pair<std::string, eOrderState> lastClosedOrder;

		bool touchActivated = false;
	};
}
