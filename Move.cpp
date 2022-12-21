#include "Move.hpp"

#include <ostream>

Move::Move(const Square& from, const Square& to,
           const std::optional<PieceType>& promotion) : from_square{from}, to_square{to}, move_promotion{promotion}
{
}

Move::Optional Move::fromUci(const std::string& uci) {
    if(uci.length() > 5 || uci.length() < 4) return std::nullopt; //Invalid UCI length
    Square::Optional from = Square::fromName(uci.substr(0,2));
    if(!from.has_value()) return std::nullopt;

    Square::Optional to = Square::fromName(uci.substr(2,2));
    if(!to.has_value()) return std::nullopt;


    if(uci.length() == 5) {
       Piece::Optional promotion_piece = Piece::fromSymbol(uci[4]);
       if(!promotion_piece.has_value()) return std::nullopt;
       PieceType promotion_type = promotion_piece.value().type();
       if(promotion_type == PieceType::Pawn || promotion_type == PieceType::King) return std::nullopt; //Promotion check
       return Move(from.value(), to.value(), promotion_type);
    }
    else return Move(from.value(), to.value());
}

Square Move::from() const {
    return from_square;
}

Square Move::to() const {
    return to_square;
}

std::optional<PieceType> Move::promotion() const {
    return move_promotion;
}

std::ostream& operator<<(std::ostream& os, const Move& move) {
    os << move.from() << move.to();
    if(move.promotion().has_value()) os << pieceTypeToChar(move.promotion().value());
    return os;
}


bool operator<(const Move& lhs, const Move& rhs) {
    return (lhs.from().index() + lhs.to().index() + (lhs.promotion().has_value() ? static_cast<unsigned>(lhs.promotion().value()) : 0))
            <
             (rhs.from().index() + rhs.to().index() + (rhs.promotion().has_value() ? static_cast<unsigned>(rhs.promotion().value()) : 0));
}

bool operator==(const Move& lhs, const Move& rhs) {
    return (lhs.from() == rhs.from() && lhs.to() == rhs.to() && lhs.promotion() == rhs.promotion());
}
