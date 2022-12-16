#include "PrincipalVariation.hpp"

#include <ostream>


PrincipalVariation::PrincipalVariation(PrincipalVariation::MoveVec &&moves, int32_t score, bool mate) : moves{moves}, eval_score{score}, mate{mate} {
}

bool PrincipalVariation::isMate() const {
    return mate;
}

int PrincipalVariation::score() const {
    return eval_score;
}

std::size_t PrincipalVariation::length() const {
    return moves.size();
}

PrincipalVariation::MoveIter PrincipalVariation::begin() const {
    return moves.begin();
}

PrincipalVariation::MoveIter PrincipalVariation::end() const {
    return moves.end();
}

std::ostream& operator<<(std::ostream& os, const PrincipalVariation& pv) {
    for(PrincipalVariation::MoveIter iter = pv.begin(); iter != pv.end(); iter++) {
        os << *iter << ", ";
    }
    os << std::endl;
    return os;
}
