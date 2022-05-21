#pragma once
#include "stMAlgorithmData.h"
#include "algorithm/algorithmBase.hpp"

namespace algorithm {
	class stMAlgorithm final : public algorithmBase<stMAlgorithmData> {
	public:


	private:
		void preLoop() override;
		bool loop() override;
		void onOpenOrder() override;
		void onCloseOrder(double aProfit) override;
		void logInternal(std::ofstream& aFile) const override;
		void initInternal() override;
		void initDataFieldInternal(const std::string& aName, const Json& aValue) override;

	};
}