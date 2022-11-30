#ifndef CHESS_ENGINE_BOARD_HPP
#define CHESS_ENGINE_BOARD_HPP

#include "Piece.hpp"
#include "Square.hpp"
#include "Move.hpp"
#include "CastlingRights.hpp"

#include <optional>
#include <iosfwd>
#include <vector>
#include <bitset>


//Default ctor bitset sets all bits to 0
struct PiecePositions {
    std::bitset<64> pawns;
    std::bitset<64> knights;
    std::bitset<64> bishops;
    std::bitset<64> rooks;
    std::bitset<64> queen;
    std::bitset<64> king;

    void andMask(std::bitset<64> mask) {
        pawns &= mask;
        knights &= mask;
        bishops &= mask;
        rooks &= mask;
        queen &= mask;
        king &= mask;
    }
};

struct ColorPositions {
    std::bitset<64> white;
    std::bitset<64> black;
};

class Board {
public:

    using Optional = std::optional<Board>;
    using MoveVec = std::vector<Move>;

    Board();

    void setPiece(const Square& square, const Piece::Optional& piece);
    Piece::Optional piece(const Square& square) const;
    void setTurn(PieceColor turn);
    PieceColor turn() const;
    void setCastlingRights(CastlingRights cr);
    CastlingRights castlingRights() const;
    void setEnPassantSquare(const Square::Optional& square);
    Square::Optional enPassantSquare() const;

    void makeMove(const Move& move);

    void pseudoLegalMoves(MoveVec& moves) const;
    void pseudoLegalMovesFrom(const Square& from, MoveVec& moves) const;

private:

    PiecePositions piecePositions;

    ColorPositions colorPositions;

    PieceColor current_turn;

    CastlingRights castling_rights;

    Square::Optional en_passant_square;


    Square::Index frontIndex(Square::Index from) const;
    Square::Index frontLeftIndex(Square::Index from) const;
    Square::Index frontRightIndex(Square::Index from) const;



    void pseudoLegalPawnMovesFrom(Square::Index index, Board::MoveVec& moves) const;


    std::optional<PieceColor> checkOccupation(Square::Index index) const;
};

std::ostream& operator<<(std::ostream& os, const Board& board);

#endif
