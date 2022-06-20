#pragma once

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

		stMAlgorithm& algorithm;
	};
}
