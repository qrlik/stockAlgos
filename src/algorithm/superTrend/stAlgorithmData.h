#pragma once
#include "algorithm/algorithmDataBase.h"
#include "market/indicators.h"

namespace algorithm {
	class stAlgorithmData final : public algorithmDataBase {
	public:
		using baseClass = algorithmDataBase;
		stAlgorithmData() = default;
		stAlgorithmData(const Json& aValue);
		bool operator==(const stAlgorithmData& aOther) const;
		Json toJson() const;

		market::eAtrType getAtrType() const { return atrType; }
		int getAtrSize() const { return atrSize; }
		double getStFactor() const { return stFactor; }
		
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

		// TO DO delete setters
		void setAtrType(market::eAtrType aAtrType) { atrType = aAtrType; }
		void setAtrSize(int aAtrSize) { atrSize = aAtrSize; }
		void setStFactor(double aStFactor) { stFactor = aStFactor; }
		void setDynamicSLPercent(double aDynamicSLPercent) { dynamicSLPercent = aDynamicSLPercent; }
		void setDynamicSLTrendMode(bool aDynamicSLTrendMode) { dynamicSLTrendMode = aDynamicSLTrendMode; }
		void setTouchOpenerActivationWaitMode(bool aState) { touchOpenerActivationWaitMode = aState; }
		void setBreakOpenerEnabled(bool aState) { breakOpenerEnabled = aState; }
		void setBreakOpenerActivationWaitMode(bool aState) { breakOpenerActivationWaitMode = aState; }
		void setAlwaysUseNewTrend(bool aState) { alwaysUseNewTrend = aState; }
		void setActivationWaiterResetAllowed(bool aState) { activationWaiterResetAllowed = aState; }
		void setActivationWaiterRange(int aRange) { activationWaiterRange = aRange; }
		void setActivationWaiterFullCandleCheck(bool aState) { activationWaiterFullCandleCheck = aState; }
		void setStopLossWaiterEnabled(bool aState) { stopLossWaiterEnabled = aState; }
		void setStopLossWaiterResetAllowed(bool aState) { stopLossWaiterResetAllowed = aState; }
		void setStopLossWaiterRange(int aRange) { stopLossWaiterRange = aRange; }
		void setStopLossWaiterFullCandleCheck(bool aState) { stopLossWaiterFullCandleCheck = aState; }
	protected:
		bool initDataFieldInternal(const std::string& aName, const Json& aValue) override;
		bool checkCriteriaInternal(const std::string& aName, const Json& aValue) const override;
		bool isValidInternal() const override;
	private:
		market::eAtrType atrType = market::eAtrType::NONE;
		int atrSize = -1;
		double stFactor = -1.0;

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