#pragma once
#include "market/candle.h"
#include "market/indicatorsSystem.h"
#include "order.h"
#include "statistic.h"
#include <type_traits>

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
	public:
		using algorithmDataType = dataType;
		algorithmBase(const dataType& aData) :
			data(aData), stats(data), indicators(data)
		{
			cash = data.getStartCash();
			statesMap[getIntState(eBaseState::NONE)] = "NONE";
			statesMap[getIntState(eBaseState::LONG)] = "LONG";
			statesMap[getIntState(eBaseState::SHORT)] = "SHORT";
		}
		bool operator==(const algorithmBase<dataType>& aOther) const {
			auto result = true;
			result &= data == aOther.data;
			result &= order == aOther.order;
			if (data.getFullCheck()) {
				result &= stats == aOther.stats;
				result &= utils::isEqual(cash, aOther.cash, market::marketData::getInstance()->getQuotePrecision());
			}
			return result;
		}
		void setWithLogs(bool aState) { withLogs = aState; }
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

		bool calculate(std::vector<market::candle> aCandles) {
			for (auto& candle : aCandles) {
				if (!doAction(candle)) {
					return false;
				}
			}
			return true;
		}
		bool doAction(market::candle& aCandle) {
			if (getStopCashBreak()) {
				return false;
			}
			if (isReady()) {
				curCandle = aCandle;
				preLoop();
				while (loop()) {}
				if (getWithLogs()) {
					log();
				}
			}
			indicators.processCandle(aCandle);
			prevCandle = aCandle;
			return true;
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
		virtual void log() const = 0; // TO DO add impl
		virtual void initDataFieldInternal(const std::string& aName, const Json& aValue) = 0;

		void setState(int aState) { state = aState; }
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

		order order;
	protected: // TO DO fix 
		statistic stats;
		double cash = 0.0;
		bool inited = false;
		bool stopCashBreak = false;

	private:
		bool isReady() const { return indicators.isInited() && !prevCandle.time.empty(); }

		const dataType data;
		market::indicatorsSystem indicators;
		std::unordered_map<int, std::string> statesMap;

		market::candle curCandle;
		market::candle prevCandle;

		int state = 0;
		bool withLogs = false;
	};
}