#pragma once

namespace algorithm {
	class stMAlgorithm;
	class closerModule {
		friend class stMAlgorithm;
	public:
		closerModule(stMAlgorithm& aAlgorithm);
		bool check();

	private:
		void updateState(bool& aState, bool aAdd) const;
		bool checkStates(bool aLong) const;

		stMAlgorithm& algorithm;
	};
}
