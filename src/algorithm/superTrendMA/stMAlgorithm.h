#pragma once
#include "algorithm/algorithmBase.hpp"
#include "closerModule.h"
#include "maDirectionModule.h"
#include "openerModule.h"
#include "stMAlgorithmData.h"

namespace algorithm {
	class stMAlgorithm final : public algorithmBase<stMAlgorithmData> {
	public:
		using baseClass = algorithmBase<stMAlgorithmData>;
		stMAlgorithm(const stMAlgorithmData& aData);
		bool operator==(const stMAlgorithm& aOther) const;

		const maDirectionModule& getMAModule() const { return maDirection; }

	private:
		void preLoop() override;
		bool loop() override;
		void onOpenOrder() override;
		void onCloseOrder(eOrderState aState, double aProfit) override;
		void logInternal(std::ofstream& aFile) const override;
		void initInternal() override;
		void initDataFieldInternal(const std::string& aName, const Json& aValue) override;

		maDirectionModule maDirection;
		openerModule opener;
		closerModule closer;
	};
}