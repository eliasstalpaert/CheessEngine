#include "Board.hpp"

#include <ostream>
#include <cassert>
#include <cmath>

Board::Board() : en_passant_square{Square::A1}
{
}

void Board::setPiece(const Square& square, const Piece::Optional& piece) {
    std::bitset<64> piece_mask; // 0 initialized
    piece_mask.set(square.index(), true);
    std::bitset<64> inverted_piece_mask = piece_mask;
    inverted_piece_mask.flip();

    switch (piece->color()) {
        case PieceColor::White:
            colorPositions.white |= piece_mask;
            colorPositions.black &= inverted_piece_mask;
            break;
        case PieceColor::Black:
            colorPositions.black |= piece_mask;
            colorPositions.white &= inverted_piece_mask;
            break;
        default: //Never occurs
            break;
    }

    //Clear bit in all bitboards (avoid checking for each one) //TODO: check if it is faster than checking
    piecePositions.andMask(inverted_piece_mask);

    switch (piece->type()) {
        case PieceType::Pawn:
            piecePositions.pawns |= piece_mask;
            break;
        case PieceType::Knight:
            piecePositions.knights |= piece_mask;
            break;
        case PieceType::Bishop:
            piecePositions.bishops |= piece_mask;
            break;
        case PieceType::Rook:
            piecePositions.rooks |= piece_mask;
            break;
        case PieceType::Queen:
            piecePositions.queen |= piece_mask;
            break;
        case PieceType::King:
            piecePositions.king |= piece_mask;
            break;
        default: //Never occurs
            break;
    }
}

Piece::Optional Board::piece(const Square& square) const {
    unsigned index = square.index();
    PieceColor color = PieceColor::White;

    if(colorPositions.white[index]) color = PieceColor::White;
    else if(colorPositions.black[index]) color = PieceColor::Black;
    else return std::nullopt;

    char candidate_symbol = 'x';
    if(piecePositions.pawns[index]) candidate_symbol = 'p';
    else if(piecePositions.knights[index]) candidate_symbol = 'n';
    else if(piecePositions.bishops[index]) candidate_symbol = 'b';
    else if(piecePositions.rooks[index]) candidate_symbol = 'r';
    else if(piecePositions.queen[index]) candidate_symbol = 'q';
    else if(piecePositions.king[index]) candidate_symbol = 'k';

    if(color == PieceColor::White) return Piece::fromSymbol(toupper(candidate_symbol));
    else return Piece::fromSymbol(candidate_symbol);
}

void Board::setTurn(PieceColor turn) {
    current_turn = turn;
}

PieceColor Board::turn() const {
    return current_turn;
}

void Board::setCastlingRights(CastlingRights cr) {
    castling_rights = cr;
}

CastlingRights Board::castlingRights() const {
    return castling_rights;
}

void Board::setEnPassantSquare(const Square::Optional& square) {
    en_passant_square = square;
}

Square::Optional Board::enPassantSquare() const {
    return en_passant_square;
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
