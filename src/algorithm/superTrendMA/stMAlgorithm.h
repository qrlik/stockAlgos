#pragma once
#include "algorithm/algorithmBase.hpp"
#include "closerModule.h"
#include "openerModule.h"
#include "stMAlgorithmData.h"

namespace algorithm {
	class stMAlgorithm final : public algorithmBase<stMAlgorithmData> {
	public:
		using baseClass = algorithmBase<stMAlgorithmData>;
		stMAlgorithm(const stMAlgorithmData& aData, market::eCandleInterval aTimeframe);
		bool operator==(const stMAlgorithm& aOther) const;

		const closerModule& getCloserModule() const { return closer; }
		void updateOrderStopLoss(double aStopLoss);
	private:
		void preLoop() override;
		bool loop() override;
		void onOpenOrder() override;
		void onCloseOrder(eOrderState aState, double aProfit) override;

		openerModule opener;
		closerModule closer;
	};
}