#pragma once
#include "market/candle.h"
#include "market/indicatorsSystem.h"
#include "market/marketRules.h"
#include "order.h"
#include "statistic.h"
#include "utils/utils.h"
#include <fstream>
#include <type_traits>

namespace tests {
	template<typename T>
	class algorithmChecker;
}

namespace algorithm {
	enum class eBaseState {
		NONE = 0,
		LONG = 1,
		SHORT = 2
	};
	template<typename T>
	int getIntState(T aState) {
		return static_cast<int>(aState);
	}

	class algorithmDataBase;
	template<typename dataType, typename = typename std::enable_if_t<std::is_base_of_v<algorithmDataBase, dataType>>>
	class algorithmBase {
		template<typename T>
		friend class tests::algorithmChecker;
	public:
		using algorithmDataType = dataType;
		algorithmBase(const dataType& aData) :
			data(aData), stats(data), indicators(data.getIndicatorsData())
		{
			cash = data.getStartCash();
			statesMap[getIntState(eBaseState::NONE)] = "NONE";
			statesMap[getIntState(eBaseState::LONG)] = "LONG";
			statesMap[getIntState(eBaseState::SHORT)] = "SHORT";
		}
		virtual ~algorithmBase() = default;
		bool operator==(const algorithmBase<dataType>& aOther) const {
			auto result = true;
			result &= data == aOther.data;
			result &= order == aOther.order;
			result &= state == aOther.state;
			if (data.getFullCheck()) {
				result &= stats == aOther.stats;
				result &= utils::isEqual(cash, aOther.cash, MARKET_DATA->getQuotePrecision());
			}
			return result;
		}

		void setWithLogs() {
			withLogs = true;
			utils::createDir(utils::outputDir);
			logsFile.open("output/Logs.txt", std::ios::app);
		}
		void setState(int aState) { state = aState; }
		const dataType& getData() const { return data; }
		const order& getOrder() const { return order; }
		const market::candle& getCandle() const { return curCandle; }
		double getFullCash() const {
			auto curCash = cash;
			if (!order.getTime().empty()) {
				curCash += order.getMargin() + order.getProfit();
			}
			return curCash;
		}
		double getCash() const { return cash; }
		int getState() const { return state; }
		const market::indicatorsSystem& getIndicators() const { return indicators; }

		bool calculate(std::vector<market::candle> aCandles) {
			for (auto& candle : aCandles) {
				if (!doAction(candle)) {
					return false;
				}
			}
			return true;
		}
		void openOrder(eOrderState aState, double aPrice) {
			aPrice = utils::round(aPrice, MARKET_DATA->getPricePrecision());
			if (!order.openOrder(data, aState, aPrice, cash, getCandle().time)) {
				return;
			}

			setState(getIntState(aState));
			auto taxAmount = utils::round(getOrder().getNotionalValue() * MARKET_DATA->getTaxFactor(), MARKET_DATA->getQuotePrecision()); // TO DO REFACTOR
			cash = cash - getOrder().getMargin() - taxAmount;
			stats.onOpenOrder((aState == eOrderState::LONG));
			onOpenOrder();
		}
		void closeOrder(double aPrice = -1.0) {
			setState(getIntState(eBaseState::NONE));
			if (utils::isGreater(aPrice, 0.0)) {
				order.updateStopLoss(aPrice);
			}
			const auto margin = order.getMargin();
			const auto profit = order.closeOrder();
			cash += margin + profit;
			if (const bool isMaxLossStop = stats.onCloseOrder(cash, profit)) {
				stopCashBreak = true;
			}
			onCloseOrder(profit);
		}

		void initFromJson(const Json& aValue) {
			if (aValue.is_null()) {
				return;
			}
			for (const auto& [key, value] : aValue.items()) {
				if (value.is_null()) {
					continue;
				}
				if (key == "order") {
					if (value.contains("lifeState")) {
						order.reset();
					}
					order.initFromJson(data, value);
				}
				else if (key == "stats") {
					stats.initFromJson(value);
				}
				else if (key == "cash") {
					cash = value.get<double>();
				}
				else {
					initDataFieldInternal(key, value);
				}
			}
		}
		Json getJsonData() const {
			Json result;
			result["cash"] = utils::round(getFullCash(), 0.01);
			stats.addJsonData(result["stats"], cash);
			data.addJsonData(result["data"]);
			return result;
		}
	protected:
		virtual void preLoop() = 0;
		virtual bool loop() = 0;
		virtual void onOpenOrder() = 0;
		virtual void onCloseOrder(double aProfit) = 0;
		virtual void logInternal(std::ofstream& aFile) const = 0;
		virtual void initInternal() = 0;
		virtual void initDataFieldInternal(const std::string& aName, const Json& aValue) = 0;

		void addState(int aState, std::string aStr) {
			if (statesMap.count(aState) == 0) {
				statesMap[aState] = std::move(aStr);
			}
			else {
				utils::logError("algorithmBase::addState wrong state");
			}
		}
		int stateFromString(const std::string& aStr) const {
			for (const auto& [intState, str] : statesMap) {
				if (str == aStr) {
					return intState;
				}
			}
			utils::logError("algorithmBase::stateFromString wrong state string");
			return 0;
		}
		std::string stateToString(int aState) const {
			if (auto it = statesMap.find(aState); it != statesMap.end()) {
				return it->second;
			}
			utils::logError("algorithmBase::stateToString wrong state");
			return {};
		}

		bool getWithLogs() const { return withLogs; }
		bool getStopCashBreak() const { return stopCashBreak; }
		const market::candle& getPrevCandle() const { return prevCandle; }

		statistic stats;
		order order;

	private:
		bool isReady() const { return indicators.isInited() && !prevCandle.time.empty(); }
		void init() {
			if (!inited) {
				initInternal();
				inited = true;
			}
		}
		bool doAction(market::candle& aCandle) {
			if (getStopCashBreak()) {
				return false;
			}
			if (isReady()) {
				curCandle = aCandle;
				init();
				preLoop();
				while (loop()) {}
				log();
			}
			indicators.processCandle(aCandle);
			prevCandle = aCandle;
			return true;
		}
		void log() {
			if (!withLogs) {
				return;
			}
			logsFile << curCandle.time << "\tcash: " << std::setw(12) << std::to_string(cash)
				<< std::setw(18) << stateToString(getState()) << std::setw(4);
			if (!getOrder().getTime().empty()) {
				logsFile << order.toString();
			}
			logInternal(logsFile);
			logsFile << '\n';
		}

		const dataType data;
		market::indicatorsSystem indicators;
		std::unordered_map<int, std::string> statesMap;
		std::ofstream logsFile;

		market::candle curCandle;
		market::candle prevCandle;

		double cash = 0.0;
		int state = 0;
		bool inited = false;
		bool stopCashBreak = false;
		bool withLogs = false;
	};
}