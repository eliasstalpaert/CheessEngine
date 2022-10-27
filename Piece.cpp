#include "Piece.hpp"

#include <ostream>

Piece::Piece(PieceColor color, PieceType type)
{
    (void)color;
    (void)type;
}

Piece::Optional Piece::fromSymbol(char symbol) {
    (void)symbol;
    return std::nullopt;
}

PieceColor Piece::color() const {
    return PieceColor::Black;
}

PieceType Piece::type() const {
    return PieceType::Pawn;
}

bool operator==(const Piece& lhs, const Piece& rhs) {
    (void)lhs;
    (void)rhs;
    return false;
}

std::ostream& operator<<(std::ostream& os, const Piece& piece) {
    (void)piece;
    return os;
}

PieceColor operator!(PieceColor color) {
    (void)color;
    return PieceColor::White;
}
