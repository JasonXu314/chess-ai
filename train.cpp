#include <algorithm>
#include <fstream>
#include <iostream>
#include <vector>

#include "engine/chess.h"
#include "representation.h"
#include "rng.h"

using namespace std;

using json = nlohmann::json;

#define POP_SIZE 1000
#define GENERATIONS 10000
#define MUTATION_FREQ 0.2

struct MatchResults {
	int a, b;
};

MatchResults match(const Individual& a, const Individual& b);

vector<double> evaluate(const vector<Individual>& population);

int main() {
	vector<Individual> population;
	vector<double> fitnesses;

	for (int i = 0; i < POP_SIZE; i++) {
		population.push_back(Individual());
	}

	for (int gen = 0; gen < GENERATIONS; gen++) {
		fitnesses = evaluate(population);

		if (gen % 100 == 0) {
			double maxFitness = *max_element(fitnesses.begin(), fitnesses.end());

			cout << (gen / 100) << "%% complete, max ffitness " << maxFitness << endl;
		}

		vector<Individual> descendants;

		for (int i = 0; i < POP_SIZE; i++) {
			Individual parentA = rng::choice(population, fitnesses);
			Individual parentB = rng::choice(population, fitnesses);

			descendants.push_back(mutate(cross(parentA, parentB), MUTATION_FREQ));
		}

		// kill parents strategy
		population = descendants;
	}

	fitnesses = evaluate(population);

	json report = json::array();
	for (int i = 0; i < POP_SIZE; i++) {
		json individual = population[i].serialize();

		individual["fitness"] = fitnesses[i];
	}

	ofstream dump("report.json");
	dump << report;
	dump.close();

	return 0;
}

MatchResults match(const Individual& a, const Individual& b) {
	MatchResults out = {.a = 0, .b = 0};

	uint halfMoves = 0;
	Game aWhite;

	while (aWhite.getAvailableMoves().size() > 0 && halfMoves < 200) {
		vector<Move> moves = aWhite.getAvailableMoves();
		vector<double> advantages(moves.size());

		if (aWhite.turn() == Players::WHITE) {
			transform(moves.begin(), moves.end(), advantages.begin(),
					  [&a, &aWhite](const Move& move) { return a.evaluatePosition(aWhite.branch(move), Players::WHITE); });
		} else {
			transform(moves.begin(), moves.end(), advantages.begin(),
					  [&b, &aWhite](const Move& move) { return b.evaluatePosition(aWhite.branch(move), Players::BLACK); });
		}

		size_t currMax = 0;
		for (size_t i = 1; i < moves.size(); i++) {
			if (advantages[i] > advantages[currMax]) {
				currMax = i;
			}
		}

		bool shouldPromote = aWhite.move(moves[currMax]);

		if (shouldPromote) {
			Position promotionSquare = moves[currMax].to;
			vector<double> advantages;

			if (aWhite.turn() == Players::WHITE) {
				for (PieceTypes piece = PieceTypes::KNIGHT; piece <= PieceTypes::QUEEN; piece = (PieceTypes)((int)piece + 1)) {
					advantages.push_back(a.evaluatePosition(aWhite.branchPromote(promotionSquare, piece), Players::WHITE));
				}
			} else {
				for (PieceTypes piece = PieceTypes::KNIGHT; piece <= PieceTypes::QUEEN; piece = (PieceTypes)((int)piece + 1)) {
					advantages.push_back(b.evaluatePosition(aWhite.branchPromote(promotionSquare, piece), Players::BLACK));
				}
			}

			PieceTypes promoteTo = PieceTypes::KNIGHT;
			for (PieceTypes piece = PieceTypes::BISHOP; piece <= PieceTypes::QUEEN; piece = (PieceTypes)((int)piece + 1)) {
				if (advantages[piece - PieceTypes::KNIGHT] > advantages[promoteTo - PieceTypes::KNIGHT]) {
					promoteTo = piece;
				}
			}

			aWhite.promote(promotionSquare, promoteTo);
		}

		halfMoves++;
	}

	if (aWhite.getAvailableMoves().size() == 0) {
		// black is checkmated
		if (aWhite.turn() == Players::BLACK) {
			out.a++;
		} else {
			out.b++;
		}
	} else {
		// tiebreak by materiel
		if (aWhite.materiel(Players::WHITE) > aWhite.materiel(Players::BLACK)) {
			out.a++;
		} else {
			out.b++;
		}
	}

	// repeat, for b being white
	halfMoves = 0;
	Game bWhite;

	while (bWhite.getAvailableMoves().size() > 0 && halfMoves < 200) {
		vector<Move> moves = bWhite.getAvailableMoves();
		vector<double> advantages(moves.size());

		if (bWhite.turn() == Players::WHITE) {
			transform(moves.begin(), moves.end(), advantages.begin(),
					  [&b, &bWhite](const Move& move) { return b.evaluatePosition(bWhite.branch(move), Players::WHITE); });
		} else {
			transform(moves.begin(), moves.end(), advantages.begin(),
					  [&a, &bWhite](const Move& move) { return a.evaluatePosition(bWhite.branch(move), Players::BLACK); });
		}

		size_t currMax = 0;
		for (size_t i = 1; i < moves.size(); i++) {
			if (advantages[i] > advantages[currMax]) {
				currMax = i;
			}
		}

		bool shouldPromote = bWhite.move(moves[currMax]);

		if (shouldPromote) {
			Position promotionSquare = moves[currMax].to;
			vector<double> advantages;

			if (bWhite.turn() == Players::WHITE) {
				for (PieceTypes piece = PieceTypes::KNIGHT; piece <= PieceTypes::QUEEN; piece = (PieceTypes)((int)piece + 1)) {
					advantages.push_back(b.evaluatePosition(bWhite.branchPromote(promotionSquare, piece), Players::WHITE));
				}
			} else {
				for (PieceTypes piece = PieceTypes::KNIGHT; piece <= PieceTypes::QUEEN; piece = (PieceTypes)((int)piece + 1)) {
					advantages.push_back(a.evaluatePosition(bWhite.branchPromote(promotionSquare, piece), Players::BLACK));
				}
			}

			PieceTypes promoteTo = PieceTypes::KNIGHT;
			for (PieceTypes piece = PieceTypes::BISHOP; piece <= PieceTypes::QUEEN; piece = (PieceTypes)((int)piece + 1)) {
				if (advantages[piece - PieceTypes::KNIGHT] > advantages[promoteTo - PieceTypes::KNIGHT]) {
					promoteTo = piece;
				}
			}

			bWhite.promote(promotionSquare, promoteTo);
		}

		halfMoves++;
	}

	if (bWhite.getAvailableMoves().size() == 0) {
		// black is checkmated
		if (bWhite.turn() == Players::BLACK) {
			out.b++;
		} else {
			out.a++;
		}
	} else {
		// tiebreak by materiel
		if (aWhite.materiel(Players::WHITE) > aWhite.materiel(Players::BLACK)) {
			out.b++;
		} else {
			out.a++;
		}
	}

	return out;
}

// round-robin tournament selection
vector<double> evaluate(const vector<Individual>& population) {
	vector<int> wins(population.size());
	transform(population.begin(), population.end(), wins.begin(), [](const Individual&) { return 0; });

	for (size_t i = 0; i < population.size(); i++) {
		for (size_t j = i + 1; j < population.size(); j++) {
			MatchResults results = match(population[i], population[j]);

			wins[i] += results.a;
			wins[j] += results.b;
		}
	}

	vector<double> winrates(population.size());
	transform(wins.begin(), wins.end(), winrates.begin(), [&population](const int& wins) { return (double)wins / (population.size() * 2 - 2); });

	return winrates;
}