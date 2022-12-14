#ifndef CHESS_ENGINE_PRINCIPALVARIATION_HPP
#define CHESS_ENGINE_PRINCIPALVARIATION_HPP

#include "Move.hpp"
#include "Piece.hpp"

#include <iosfwd>
#include <cstddef>
#include <vector>

class PrincipalVariation {
public:

    using MoveIter = std::vector<Move>::const_iterator;
    using MoveVec = std::vector<Move>;

    PrincipalVariation(MoveVec&& moves, int32_t score);

    bool isMate() const;
    int score() const;

    std::size_t length() const;
    MoveIter begin() const;
    MoveIter end() const;


private:

    MoveVec moves;
    int32_t eval_score;
    bool mate;

};

std::ostream& operator<<(std::ostream& os, const PrincipalVariation& pv);

#endif
