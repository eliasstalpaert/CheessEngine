#include "Piece.hpp"

#include <ostream>

Piece::Piece(PieceColor color, PieceType type) : piece_color{color}, piece_type{type}
{

}

Piece::Optional Piece::fromSymbol(char symbol) {
    switch(symbol) {
        case 'p':
            return Piece(PieceColor::Black, PieceType::Pawn);
        case 'n':
            return Piece(PieceColor::Black, PieceType::Knight);
        case 'b':
            return Piece(PieceColor::Black, PieceType::Bishop);
        case 'r':
            return Piece(PieceColor::Black, PieceType::Rook);
        case 'q':
            return Piece(PieceColor::Black, PieceType::Queen);
        case 'k':
            return Piece(PieceColor::Black, PieceType::King);
        case 'P':
            return Piece(PieceColor::White, PieceType::Pawn);
        case 'N':
            return Piece(PieceColor::White, PieceType::Knight);
        case 'B':
            return Piece(PieceColor::White, PieceType::Bishop);
        case 'R':
            return Piece(PieceColor::White, PieceType::Rook);
        case 'Q':
            return Piece(PieceColor::White, PieceType::Queen);
        case 'K':
            return Piece(PieceColor::White, PieceType::King);
        default:
            return std::nullopt;
    }
}

PieceColor Piece::color() const {
    return piece_color;
}

PieceType Piece::type() const {
    return piece_type;
}

bool operator==(const Piece& lhs, const Piece& rhs) {
    if(lhs.color() == rhs.color() && lhs.type() == rhs.type()) return true;
    else return false;
}

std::ostream& operator<<(std::ostream& os, const Piece& piece) {
    char output_char;
    switch(piece.type()) {
        case PieceType::Pawn:
            output_char = 'p';
            break;
        case PieceType::Knight:
            output_char = 'n';
            break;
        case PieceType::Bishop:
            output_char = 'b';
            break;
        case PieceType::Rook:
            output_char = 'r';
            break;
        case PieceType::Queen:
            output_char = 'q';
            break;
        case PieceType::King:
            output_char = 'k';
            break;
        default:
            output_char = 'X'; //TODO: throw an exception if char is unrecognized
            break;
    }
    if(piece.color() == PieceColor::White) output_char = toupper(output_char);
    os << output_char;
    return os;
}

PieceColor operator!(PieceColor color) {
    switch(color) {
        case PieceColor::White:
            return PieceColor::Black;
        case PieceColor::Black:
            return PieceColor::White;
        default:
            return PieceColor::White; //TODO: can this be omitted?
    }
}
