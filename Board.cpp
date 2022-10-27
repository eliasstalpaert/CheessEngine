#include "Board.hpp"

#include <ostream>
#include <cassert>
#include <cmath>

Board::Board()
{
}

void Board::setPiece(const Square& square, const Piece::Optional& piece) {
    (void)square;
    (void)piece;
}

Piece::Optional Board::piece(const Square& square) const {
    (void)square;
    return std::nullopt;
}

void Board::setTurn(PieceColor turn) {
    (void)turn;
}

PieceColor Board::turn() const {
    return PieceColor::White;
}

void Board::setCastlingRights(CastlingRights cr) {
    (void)cr;
}

CastlingRights Board::castlingRights() const {
    return CastlingRights::None;
}

void Board::setEnPassantSquare(const Square::Optional& square) {
    (void)square;
}

Square::Optional Board::enPassantSquare() const {
    return std::nullopt;
}

void Board::makeMove(const Move& move) {
    (void)move;
}

void Board::pseudoLegalMoves(MoveVec& moves) const {
    (void)moves;
}

void Board::pseudoLegalMovesFrom(const Square& from,
                                 Board::MoveVec& moves) const {
    (void)from;
    (void)moves;
}

std::ostream& operator<<(std::ostream& os, const Board& board) {
    (void)board;
    return os;
}
