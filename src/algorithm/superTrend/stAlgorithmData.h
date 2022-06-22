#pragma once
#include "algorithm/algorithmDataBase.h"
#include "market/indicatorsData.h"

namespace algorithm {
	class stAlgorithmData final : public algorithmDataBase {
	public:
		stAlgorithmData();
		bool operator==(const stAlgorithmData& aOther) const;

		double getDynamicSLPercent() const { return dynamicSLPercent; }
		bool getDynamicSLTrendMode() const { return dynamicSLTrendMode; }

		bool getTouchOpenerActivationWaitMode() const { return touchOpenerActivationWaitMode; }

		bool getBreakOpenerEnabled() const { return breakOpenerEnabled; }
		bool getBreakOpenerActivationWaitMode() const { return breakOpenerActivationWaitMode; }
		bool getAlwaysUseNewTrend() const { return alwaysUseNewTrend; }

		bool getActivationWaiterResetAllowed() const { return activationWaiterResetAllowed; }
		int getActivationWaiterRange() const { return activationWaiterRange; }
		bool getActivationWaiterFullCandleCheck() const { return activationWaiterFullCandleCheck; }

		bool getStopLossWaiterEnabled() const { return stopLossWaiterEnabled; }
		bool getStopLossWaiterResetAllowed() const { return stopLossWaiterResetAllowed; }
		int getStopLossWaiterRange() const { return stopLossWaiterRange; }
		bool getStopLossWaiterFullCandleCheck() const { return stopLossWaiterFullCandleCheck; }

	protected:
		size_t getCustomHash() const override;

	private:
		bool initDataFieldInternal(const std::string& aName, const Json& aValue) override;
		bool checkCriteriaInternal(const std::string& aName, const Json& aValue) const override;
		bool isValidInternal() const override;
		void addJsonDataInternal(Json& aData) const override;

		double dynamicSLPercent = -1.0;
		bool dynamicSLTrendMode = false;

		bool touchOpenerActivationWaitMode = false;

		bool breakOpenerEnabled = false;
		bool breakOpenerActivationWaitMode = false;
		bool alwaysUseNewTrend = false;

		bool activationWaiterResetAllowed = false;
		int activationWaiterRange = -1;
		bool activationWaiterFullCandleCheck = false;

		bool stopLossWaiterEnabled = false;
		bool stopLossWaiterResetAllowed = false;
		int stopLossWaiterRange = -1;
		bool stopLossWaiterFullCandleCheck = false;
	};
}