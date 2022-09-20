#pragma once
#include "algorithm/order.h"

namespace algorithm {
	class stMAlgorithm;
	class closerModule {
		friend class stMAlgorithm;
	public:
		closerModule(stMAlgorithm& aAlgorithm);
		bool check();
		bool isNeedToClose(bool aLong) const;

	private:
		void updateState(bool& aState, bool aAdd) const;
		bool updateTrail();
		const order& getOrder() const;

		stMAlgorithm& algorithm;
	};
}
