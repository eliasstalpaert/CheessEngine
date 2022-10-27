#include "Move.hpp"

#include <ostream>

Move::Move(const Square& from, const Square& to,
           const std::optional<PieceType>& promotion)
{
    (void)from;
    (void)to;
    (void)promotion;
}

Move::Optional Move::fromUci(const std::string& uci) {
    (void)uci;
    return std::nullopt;
}

Square Move::from() const {
    return Square::A1;
}

Square Move::to() const {
    return Square::A1;
}

std::optional<PieceType> Move::promotion() const {
    return std::nullopt;
}

std::ostream& operator<<(std::ostream& os, const Move& move) {
    (void)move;
    return os;
}


bool operator<(const Move& lhs, const Move& rhs) {
    (void)lhs;
    (void)rhs;
    return false;
}

bool operator==(const Move& lhs, const Move& rhs) {
    (void)lhs;
    (void)rhs;
    return false;
}
