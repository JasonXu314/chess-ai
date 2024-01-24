#include "rng.h"

using namespace std;

uniform_real_distribution<double> rng::uniform;
mt19937_64 rng::engine;

size_t rng::weightedRand(const vector<double>& weights) {
	discrete_distribution<size_t> dist(weights.begin(), weights.end());

	return dist(engine);
}

double rng::rand() { return uniform(engine); }

double rng::rand(double max) { return uniform(engine) * max; }

double rng::rand(double min, double max) { return uniform(engine) * (max - min) + min; }

size_t rng::randu(size_t max) { return uniform_int_distribution<size_t>(0, max)(engine); }

size_t rng::randu(size_t min, size_t max) { return uniform_int_distribution<size_t>(min, max)(engine); }