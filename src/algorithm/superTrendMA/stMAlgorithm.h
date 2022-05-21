#pragma once
#include "algorithm/algorithmBase.hpp"
#include "openerModule.h"
#include "closerModule.h"
#include "stMAlgorithmData.h"

namespace algorithm {
	class stMAlgorithm final : public algorithmBase<stMAlgorithmData> {
	public:
		using baseClass = algorithmBase<stMAlgorithmData>;
		stMAlgorithm(const stMAlgorithmData& aData);

	private:
		void preLoop() override;
		bool loop() override;
		void onOpenOrder() override;
		void onCloseOrder(double aProfit) override;
		void logInternal(std::ofstream& aFile) const override;
		void initInternal() override;
		void initDataFieldInternal(const std::string& aName, const Json& aValue) override;

		openerModule opener;
		closerModule closer;
	};
}