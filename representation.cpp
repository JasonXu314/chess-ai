#include "representation.h"

using namespace std;

using json = nlohmann::json;

Individual::Individual(bool random) {
	for (const PieceTypes type : PIECE_TYPES) {
		pieceMaps[type] = new double[64];

		for (int i = 0; i < 64; i++) {
			pieceMaps[type][i] = random ? rng::rand() : 0;
		}
	}
}

Individual::Individual(const Individual& other) {
	for (const PieceTypes type : PIECE_TYPES) {
		pieceMaps[type] = new double[64];

		for (int i = 0; i < 64; i++) {
			pieceMaps[type][i] = other.pieceMaps.at(type)[i];
		}
	}
}

Individual::Individual(const json& serialized) {
	for (const PieceTypes type : PIECE_TYPES) {
		json arr;

		switch (type) {
			case PieceTypes::PAWN:
				arr = serialized["pawn"];
				break;
			case PieceTypes::KNIGHT:
				arr = serialized["knight"];
				break;
			case PieceTypes::BISHOP:
				arr = serialized["bishop"];
				break;
			case PieceTypes::ROOK:
				arr = serialized["rook"];
				break;
			case PieceTypes::QUEEN:
				arr = serialized["queen"];
				break;
			case PieceTypes::KING:
				arr = serialized["king"];
				break;
		}

		pieceMaps[type] = new double[64];
		for (int i = 0; i < 64; i++) {
			pieceMaps[type][i] = arr[i];
		}
	}
}

double Individual::evaluatePosition(const Game& game, Players perspective) const {
	double advantage = 0;

	for (const Files file : FILES) {
		for (const uint rank : RANKS) {
			if (game.hasPiece({.file = file, .rank = rank})) {
				Piece piece = game.getPiece({.file = file, .rank = rank});

				if (piece.player() == perspective) {
					advantage += pieceMaps.at(piece.type())[file * 8 + rank - 1];
				} else {
					advantage -= pieceMaps.at(piece.type())[file * 8 + rank - 1];
				}
			}
		}
	}

	return advantage;
}

json Individual::serialize() const {
	json out;

	for (const PieceTypes type : PIECE_TYPES) {
		json arr = json::array();

		for (int i = 0; i < 64; i++) {
			arr.push_back(pieceMaps.at(type)[i]);
		}

		switch (type) {
			case PieceTypes::PAWN:
				out["pawn"] = arr;
				break;
			case PieceTypes::KNIGHT:
				out["knight"] = arr;
				break;
			case PieceTypes::BISHOP:
				out["bishop"] = arr;
				break;
			case PieceTypes::ROOK:
				out["rook"] = arr;
				break;
			case PieceTypes::QUEEN:
				out["queen"] = arr;
				break;
			case PieceTypes::KING:
				out["king"] = arr;
				break;
		}
	}

	return out;
}

Individual mutate(const Individual& source, double freq) {
	Individual out = source;

	for (const PieceTypes type : PIECE_TYPES) {
		for (const Files file : FILES) {
			for (const uint rank : RANKS) {
				if (rng::rand() < freq) {
					out.pieceMaps[type][file * 8 + rank - 1] += rng::rand(0.2) - 0.1;
				}
			}
		}
	}

	return out;
}

Individual cross(const Individual& a, const Individual& b) {
	Individual out;

	for (const PieceTypes type : PIECE_TYPES) {
		uint quadrant = rng::randu(1, 4);
		bool top = quadrant == 1 || quadrant == 2, right = quadrant == 1 || quadrant == 4;
		Files pivotFile = rng::choice(FILES, 8);
		uint pivotRank = rng::choice(RANKS, 8);

		for (const Files file : FILES) {
			for (const uint rank : RANKS) {
				if ((top ? rank >= pivotRank : rank <= pivotRank) && (right ? file >= pivotFile : rank <= pivotFile)) {
					out.pieceMaps[type][file * 8 + rank - 1] = a.pieceMaps.at(type)[file * 8 + rank - 1];
				} else {
					out.pieceMaps[type][file * 8 + rank - 1] = b.pieceMaps.at(type)[file * 8 + rank - 1];
				}
			}
		}
	}

	return out;
}

Individual& Individual::operator=(const Individual& other) {
	for (const PieceTypes type : PIECE_TYPES) {
		for (int i = 0; i < 64; i++) {
			pieceMaps[type][i] = other.pieceMaps.at(type)[i];
		}
	}

	return *this;
}

Individual::~Individual() {
	for (const PieceTypes type : PIECE_TYPES) {
		delete[] pieceMaps[type];
	}
}