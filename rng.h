#ifndef RNG_H
#define RNG_H

#include <algorithm>
#include <functional>
#include <random>
#include <vector>

namespace rng {
extern std::uniform_real_distribution<double> uniform;
extern std::mt19937_64 engine;

std::size_t weightedRand(const std::vector<double>& weights);

double rand();
double rand(double max);
double rand(double min, double max);

std::size_t randu(std::size_t max);
std::size_t randu(std::size_t min, std::size_t max);

template <typename T>
T choice(T* items, std::size_t size) {
	uint idx = randu(size - 1);	 // randu is inclusive

	return items[idx];
}

template <typename T>
T choice(const std::vector<T>& items, const std::function<double(const T&)>& weight) {
	std::vector<double> weights;
	for (std::size_t i = 0; i < items.size(); i++) {
		weights.push_back(weight(items[i]));
	}

	std::discrete_distribution<std::size_t> dist(weights.begin(), weights.end());

	return items[dist(engine)];
}

template <typename T>
T choice(const std::vector<T>& items, const std::vector<double>& weights) {
	std::discrete_distribution<std::size_t> dist(weights.begin(), weights.end());

	return items[dist(engine)];
}
}  // namespace rng

#endif