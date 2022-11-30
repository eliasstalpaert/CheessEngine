#include "Board.hpp"

#include <ostream>
#include <cassert>
#include <cmath>

Board::Board() : en_passant_square{std::nullopt}
{
}

void Board::setPiece(const Square& square, const Piece::Optional& piece) {
    std::bitset<64> piece_mask; // 0 initialized
    piece_mask[square.index()] = true;
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


//Generate pseudolegal moves for the current player
void Board::pseudoLegalMoves(MoveVec& moves) const {
    std::bitset<64> current_turn_pieces;

    switch (current_turn) {
        case PieceColor::White:
            current_turn_pieces = colorPositions.white;
            break;
        case PieceColor::Black:
            current_turn_pieces = colorPositions.black;
            break;
    }

    for(Square::Index piece_index = 0; piece_index < 63 ; piece_index++) {
        if(current_turn_pieces[piece_index]) {
            if(piecePositions.pawns[piece_index]) pseudoLegalPawnMovesFrom(piece_index, moves);
        }

    }
}

//Generate pseudolegal moves from a square for the current player
void Board::pseudoLegalMovesFrom(const Square& from,
                                 Board::MoveVec& moves) const {
    //Check piecetype and color and call appropriate function
    Piece::Optional from_piece = piece(from);

    if(from_piece.has_value()) {
        if(from_piece.value().color() == current_turn) {
            switch (from_piece->type()) {
                case PieceType::Pawn :
                    pseudoLegalPawnMovesFrom(from.index(), moves);
                    break;
                case PieceType::Knight :
                    break;
                case PieceType::Bishop :
                    break;
                case PieceType::Rook :
                    break;
                case PieceType::Queen :
                    break;
                case PieceType::King :
                    break;
            }
        }
    }

}

std::optional<PieceColor> Board::checkOccupation(Square::Index index) const {
    //TODO: Check performance impact of set/test (bound checking is safer)
    //CAREFUL!: no index bound checking is done, so an empty return value could also mean it is out of bounds (not so safe access also, performance reasons)
    if(colorPositions.white[index]) return PieceColor::White;
    else if(colorPositions.black[index]) return PieceColor::Black;
    else return std::nullopt;
}

Square::Index Board::frontIndex(Square::Index from) const {
    switch (current_turn) {
        case PieceColor::White :
            return from + 8;
        case PieceColor::Black :
            return from - 8;
        default :
            return 64; //Never occurs
    }
}

Square::Index Board::doublePushIndex(Square::Index from) const {
    switch (current_turn) {
        case PieceColor::White :
            return from + 16;
        case PieceColor::Black :
            return from - 16;
        default :
            return 64; //Never occurs
    }
}

Square::Index Board::frontLeftIndex(Square::Index from) const {
    switch (current_turn) {
        case PieceColor::White :
            return from + 8 - 1;
        case PieceColor::Black :
            return from - 8 + 1;
        default :
            return 64; //Never occurs
    }
}

Square::Index Board::frontRightIndex(Square::Index from) const {
    switch (current_turn) {
        case PieceColor::White :
            return from + 8 + 1;
        case PieceColor::Black :
            return from - 8 - 1;
        default :
            return 64; //Never occurs
    }
}

bool Board::promotionCandidate(Square::Index index) const {
    switch (current_turn) {
        case PieceColor::White :
            return index > 47;
        case PieceColor::Black :
            return index < 16;
        default : //Never occurs
            return false;
    }
}

bool Board::doublePushCandidate(Square::Index index) const {
    switch (current_turn) {
        case PieceColor::White :
            return index > 7 && index < 16;
        case PieceColor::Black :
            return index < 56 && index > 47;
        default : //Never occurs
            return false;
    }
}

bool Board::enPassantCheck(Square::Index index) const {
    if(!en_passant_square.has_value()) return false;
    Square::Index en_passant_index = en_passant_square->index();
    if(index % 8 == 0) {
        //Only check white-right
        return index + 1 == en_passant_index;
    }
    else if (index % 8 == 7) {
        //Only check white-left
        return index - 1 == en_passant_index;
    }
    else {
        return index + 1 == en_passant_index || index - 1 == en_passant_index;
    }
}

void Board::pseudoLegalPawnMovesFrom(Square::Index pawn_index, Board::MoveVec& moves) const {
    //TODO: code duplication

    //Check occupation in front of pawn (pawn will never be resting in last rank)
    Square current_square = Square::fromIndex(pawn_index).value();
    Square::Index front_index = frontIndex(pawn_index);
    std::optional<PieceColor> front_pawn = checkOccupation(front_index);
    if(!front_pawn.has_value()){
        Square front_square = Square::fromIndex(front_index).value();
        if(promotionCandidate(pawn_index)) { //Second to last rank pawns
            //Promotion to queen
            moves.push_back(Move(current_square, front_square, PieceType::Queen));
            //Promotion to rook
            moves.push_back(Move(current_square, front_square, PieceType::Rook));
            //Promotion to bishop
            moves.push_back(Move(current_square, front_square, PieceType::Bishop));
            //Promotion to knight
            moves.push_back(Move(current_square, front_square, PieceType::Knight));
        }
        else { //Promotion is mandatory
            moves.push_back(Move(current_square, front_square));
            if(doublePushCandidate(pawn_index)) {
                Square::Index double_push_index = doublePushIndex(pawn_index);
                if(!checkOccupation(double_push_index).has_value()) {
                    moves.push_back(Move(current_square, Square::fromIndex(doublePushIndex(pawn_index)).value()));
                }
            }
        }

    }

    Square::Index front_left_index = frontLeftIndex(pawn_index);
    Square::Index front_right_index = frontRightIndex(pawn_index);

    signed front_left_rank_distance = abs(static_cast<signed>((front_left_index / 8)) - static_cast<signed>((pawn_index / 8)));
    signed front_right_rank_distance = abs(static_cast<signed>((front_right_index / 8)) - static_cast<signed>((pawn_index / 8)));

    //Front left captures check
    if(front_left_rank_distance == 1) {
        std::optional<PieceColor> front_left_pawn = checkOccupation(front_left_index);

        if(front_left_pawn.has_value() && current_turn != front_left_pawn.value()) { //Capture possible
            Square front_left_square = Square::fromIndex(front_left_index).value();
            if(promotionCandidate(pawn_index)) { //Promotion
                //Promotion to queen
                moves.push_back(Move(current_square, front_left_square, PieceType::Queen));
                //Promotion to rook
                moves.push_back(Move(current_square, front_left_square, PieceType::Rook));
                //Promotion to bishop
                moves.push_back(Move(current_square, front_left_square, PieceType::Bishop));
                //Promotion to knight
                moves.push_back(Move(current_square, front_left_square, PieceType::Knight));
            }
            else { //Promotion is mandatory
                moves.push_back(Move(current_square, front_left_square));
            }
        }
    }

    //Front right captures check
    if(front_right_rank_distance == 1) {
        std::optional<PieceColor> front_right_pawn = checkOccupation(front_right_index);

        if(front_right_pawn.has_value() && current_turn != front_right_pawn.value()) { //Capture possible
            Square front_right_square = Square::fromIndex(front_right_index).value();
            if(promotionCandidate(pawn_index)) { //Promotion
                //Promotion to queen
                moves.push_back(Move(current_square, front_right_square, PieceType::Queen));
                //Promotion to rook
                moves.push_back(Move(current_square, front_right_square, PieceType::Rook));
                //Promotion to bishop
                moves.push_back(Move(current_square, front_right_square, PieceType::Bishop));
                //Promotion to knight
                moves.push_back(Move(current_square, front_right_square, PieceType::Knight));
            }
            else { //Promotion is mandatory
                moves.push_back(Move(current_square, front_right_square));
            }
        }
    }

    //Check if en-passant move possible
    if(en_passant_square.has_value()) {
        Square::Index en_passant_index = en_passant_square->index();

        if(front_right_index == en_passant_index && front_right_rank_distance == 1) {
            moves.push_back(Move(current_square, Square::fromIndex(front_right_index).value()));
        }
        if (front_left_index == en_passant_index && front_left_rank_distance == 1) {
            moves.push_back(Move(current_square, Square::fromIndex(front_left_index).value()));
        }

    }

}




std::ostream& operator<<(std::ostream& os, const Board& board) {
    (void)board;
    return os;
}
