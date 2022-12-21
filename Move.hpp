#ifndef CHESS_ENGINE_MOVE_HPP
#define CHESS_ENGINE_MOVE_HPP

#include "Square.hpp"
#include "Piece.hpp"

#include <iosfwd>
#include <optional>
#include <string>

class Move {
private:
    Square from_square;
    Square to_square;
    std::optional<PieceType> move_promotion;

public:

    using Optional = std::optional<Move>;

    Move(const Square& from, const Square& to,
         const std::optional<PieceType>& promotion = std::nullopt);
    Move();

    void setFrom(const Square& from);
    void setTo(const Square& to);
    void setPromotion(std::optional<PieceType> promotion);

    static Optional fromUci(const std::string& uci);

    Square from() const;
    Square to() const;
    std::optional<PieceType> promotion() const;

};

std::ostream& operator<<(std::ostream& os, const Move& move);

// Needed for std::map, std::set
bool operator<(const Move& lhs, const Move& rhs);
bool operator==(const Move& lhs, const Move& rhs);

#endif
