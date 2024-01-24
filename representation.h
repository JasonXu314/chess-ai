#ifndef REPRESENTATION_H
#define REPRESENTATION_H

#include <map>

#include "engine/chess.h"
#include "json.hpp"
#include "rng.h"

struct Individual {
	Individual(bool random = false);
	Individual(const Individual& other);
	Individual(const nlohmann::json& serialized);

	double evaluatePosition(const Game& game, Players perspective) const;

	nlohmann::json serialize() const;

	Individual& operator=(const Individual& other);

	~Individual();

	std::map<PieceTypes, double*> pieceMaps;

	// Evolution operators
	friend Individual mutate(const Individual& source, double freq);
	friend Individual cross(const Individual& a, const Individual& b);
};

#endif