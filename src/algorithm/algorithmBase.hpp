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
		algorithmBase(const dataType& aData, market::eCandleInterval aTimeframe) :
			data(aData), stats(data, aTimeframe), indicators(data.getIndicatorsData(), data.getTicker()), order(data)
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
				result &= utils::isEqual(cash, aOther.cash, data.getMarketData().getQuotePrecision());
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
			return cash + order.getMargin();
		}
		double getCash() const { return cash; }
		int getState() const { return state; }
		const market::indicatorsSystem& getIndicators() const { return indicators; }

		bool calculate(const std::vector<market::candle>& aCandles) {
			for (const auto& candle : aCandles) {
				if (!doAction(candle)) {
					return false;
				}
			}
			return true;
		}
		bool openOrder(eOrderState aState, double aPrice) {
			aPrice = utils::round(aPrice, data.getMarketData().getPricePrecision(aPrice));
			if (!order.openOrder(aState, aPrice, cash, getCandle().time)) {
				return false;
			}

			setState(getIntState(aState));
			auto taxAmount = utils::round(getOrder().getNotionalValue() * MARKET_SYSTEM->getTaxFactor(), data.getMarketData().getQuotePrecision());
			cash = cash - getOrder().getMargin() - taxAmount;
			stats.onOpenOrder((aState == eOrderState::LONG), taxAmount);
			onOpenOrder();
			return true;
		}
		void closeOrder(double aPrice = -1.0) {
			setState(getIntState(eBaseState::NONE));
			if (utils::isGreater(aPrice, 0.0)) {
				order.updateStopLoss(aPrice);
			}
			const auto orderState = order.getState();
			const auto margin = order.getMargin();
			const auto profit = order.closeOrder();
			cash += margin + profit;
			if (const bool isMaxLossStop = stats.onCloseOrder(cash, profit)) {
				stopCashBreak = true;
			}
			onCloseOrder(orderState, profit);
		}

		void initFromJson(const Json& aValue) {
			if (aValue.is_null()) {
				return;
			}
			for (const auto& [key, value] : aValue.items()) {
				if (value.is_null()) {
					continue;
				}
				if (key == "state") {
					setState(stateFromString(value.get<std::string>()));
				}
				else if (key == "order") {
					if (value.contains("lifeState")) {
						order.reset();
					}
					order.initFromJson(value);
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
			stats.addJsonData(result["stats"], getFullCash(), workedCandleCounter);
			data.addJsonData(result["data"]);
			return result;
		}
	protected:
		virtual void preLoop() = 0;
		virtual bool loop() = 0;
		virtual void onOpenOrder() = 0;
		virtual void onCloseOrder(eOrderState aState, double aProfit) = 0;
		virtual void logInternal(std::ofstream& file) const {};
		virtual void initInternal() {};
		virtual void initDataFieldInternal(const std::string& name, const Json& value) {};

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
		statistic& getStats() { return stats; }
		order& getOrder() { return order; }
		const market::candle& getPrevCandle() const { return prevCandle; }

	private:
		bool isReady() const { return indicators.isInited() && !prevCandle.time.empty(); }
		void init() {
			if (!inited) {
				initInternal();
				inited = true;
			}
		}
		bool doAction(const market::candle& aCandle) {
			if (getStopCashBreak()) {
				return false;
			}
			if (isReady()) {
				curCandle = aCandle;
				init();
				preLoop();
				auto workState = true;
				while (workState) {
					workState &= loop();
					workState &= !getStopCashBreak();
				}
				++workedCandleCounter;
				log();
			}
			prevCandle = aCandle;
			indicators.processCandle(prevCandle);
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
		statistic stats;
		market::indicatorsSystem indicators;
		order order;
		std::unordered_map<int, std::string> statesMap;
		std::ofstream logsFile;

		market::candle curCandle;
		market::candle prevCandle;

		double cash = 0.0;
		int state = 0;
		size_t workedCandleCounter = 0;
		bool inited = false;
		bool stopCashBreak = false;
		bool withLogs = false;
	};
}