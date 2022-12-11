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

    for(Square::Index piece_index = 0; piece_index < 64 ; piece_index++) {
        if(current_turn_pieces[piece_index]) {
            if(piecePositions.pawns[piece_index]) pseudoLegalPawnMovesFrom(piece_index, moves);
            else if(piecePositions.king[piece_index]) pseudoLegalKingMovesFrom(piece_index, moves);
            else if(piecePositions.knights[piece_index]) pseudoLegalKnightMovesFrom(piece_index, moves);
            else if(piecePositions.rooks[piece_index]) pseudoLegalRookMovesFrom(piece_index, moves);
            else if(piecePositions.bishops[piece_index]) pseudoLegalBishopMovesFrom(piece_index, moves);
            else if(piecePositions.queen[piece_index]) pseudoLegalQueenMovesFrom(piece_index, moves);
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
                    pseudoLegalKnightMovesFrom(from.index(), moves);
                    break;
                case PieceType::Bishop :
                    pseudoLegalBishopMovesFrom(from.index(), moves);
                    break;
                case PieceType::Rook :
                    pseudoLegalRookMovesFrom(from.index(), moves);
                    break;
                case PieceType::Queen :
                    pseudoLegalQueenMovesFrom(from.index(), moves);
                    break;
                case PieceType::King :
                    pseudoLegalKingMovesFrom(from.index(), moves);
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

Square::Index Board::backIndex(Square::Index from) const {
    switch (current_turn) {
        case PieceColor::White :
            return from - 8;
        case PieceColor::Black :
            return from + 8;
        default :
            return 64; //Never occurs
    }
}

Square::Index Board::leftIndex(Square::Index from) const {
    switch (current_turn) {
        case PieceColor::White :
            return from - 1;
        case PieceColor::Black :
            return from + 1;
        default :
            return 64; //Never occurs
    }
}

Square::Index Board::rightIndex(Square::Index from) const {
    switch (current_turn) {
        case PieceColor::White :
            return from + 1;
        case PieceColor::Black :
            return from - 1;
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

Square::Index Board::backLeftIndex(Square::Index from) const {
    switch (current_turn) {
        case PieceColor::White :
            return from - 8 - 1;
        case PieceColor::Black :
            return from + 8 + 1;
        default :
            return 64; //Never occurs
    }
}

Square::Index Board::backRightIndex(Square::Index from) const {
    switch (current_turn) {
        case PieceColor::White :
            return from - 8 + 1;
        case PieceColor::Black :
            return from + 8 - 1;
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

bool Board::firstRankCheck(Square::Index index) const {
    switch (current_turn) {
        case PieceColor::White :
            return (index <= 7);
        case PieceColor::Black :
            return (index >= 56 && index <= 63);
        default : //Never reached
            return false;
    }
}

bool Board::lastRankCheck(Square::Index index) const {
    switch (current_turn) {
        case PieceColor::White :
            return (index >= 56 && index <= 63);
        case PieceColor::Black :
            return (index <= 7);
        default : //Never reached
            return false;
    }
}

bool Board::isOutOfRange(Square::Index index) const {
    return index > 63;
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

    //TODO: replace distance with black/white square change check
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

void Board::pseudoLegalKingMovesFrom(Square::Index king_index, Board::MoveVec &moves) const {
    //TODO: more efficient position check (border cases) for example using comparisons
    //TODO: code duplication (maybe sth with giving directions to check)
    Square current_square = Square::fromIndex(king_index).value();

    Square::Index front_left_index = frontLeftIndex(king_index);
    Square::Index front_right_index = frontRightIndex(king_index);
    Square::Index back_left_index = backLeftIndex(king_index);
    Square::Index back_right_index = backRightIndex(king_index);
    Square::Index left_index = leftIndex(king_index);
    Square::Index right_index = rightIndex(king_index);

    signed front_left_rank_distance = abs(static_cast<signed>((front_left_index / 8)) - static_cast<signed>((king_index / 8)));
    signed front_right_rank_distance = abs(static_cast<signed>((front_right_index / 8)) - static_cast<signed>((king_index / 8)));
    signed back_left_rank_distance = abs(static_cast<signed>((front_left_index / 8)) - static_cast<signed>((king_index / 8)));
    signed back_right_rank_distance = abs(static_cast<signed>((front_right_index / 8)) - static_cast<signed>((king_index / 8)));
    signed left_rank_distance = abs(static_cast<signed>((left_index / 8)) - static_cast<signed>((king_index / 8)));
    signed right_rank_distance = abs(static_cast<signed>((right_index / 8)) - static_cast<signed>((king_index / 8)));

    if(!lastRankCheck(king_index)) {
        //Front
        Square::Index front_index = frontIndex(king_index);
        std::optional<PieceColor> front_pawn = checkOccupation(front_index);
        Square front_square = Square::fromIndex(front_index).value();
        if(!front_pawn.has_value()) moves.push_back(Move(current_square, front_square));
        else if (front_pawn.value() != current_turn) moves.push_back(Move(current_square, front_square));
        //Front-Left
        if(front_left_rank_distance == 1) {
            std::optional<PieceColor> front_left_pawn = checkOccupation(front_index);
            Square front_left_square = Square::fromIndex(front_left_index).value();
            if(!front_left_pawn.has_value()) moves.push_back(Move(current_square, front_left_square));
            else if (front_left_pawn.value() != current_turn) moves.push_back(Move(current_square, front_left_square));
        }
        //Front-Right
        if(front_right_rank_distance == 1) {
            std::optional<PieceColor> front_right_pawn = checkOccupation(front_index);
            Square front_right_square = Square::fromIndex(front_right_index).value();
            if(!front_right_pawn.has_value()) moves.push_back(Move(current_square, front_right_square));
            else if (front_right_pawn.value() != current_turn) moves.push_back(Move(current_square, front_right_square));
        }
    }
    if(!firstRankCheck(king_index)) {
        //back
        Square::Index back_index = backIndex(king_index);
        std::optional<PieceColor> back_pawn = checkOccupation(back_index);
        Square back_square = Square::fromIndex(back_index).value();
        if(!back_pawn.has_value()) moves.push_back(Move(current_square, back_square));
        else if (back_pawn.value() != current_turn) moves.push_back(Move(current_square, back_square));
        //back-Left
        if(back_left_rank_distance == 1) {
            std::optional<PieceColor> back_left_pawn = checkOccupation(back_left_index);
            Square back_left_square = Square::fromIndex(back_left_index).value();
            if(!back_left_pawn.has_value()) moves.push_back(Move(current_square, back_left_square));
            else if (back_left_pawn.value() != current_turn) moves.push_back(Move(current_square, back_left_square));
        }
        //back-Right
        if(back_right_rank_distance == 1) {
            std::optional<PieceColor> back_right_pawn = checkOccupation(back_right_index);
            Square back_right_square = Square::fromIndex(back_right_index).value();
            if(!back_right_pawn.has_value()) moves.push_back(Move(current_square, back_right_square));
            else if (back_right_pawn.value() != current_turn) moves.push_back(Move(current_square, back_right_square));
        }
    }

    if(left_rank_distance == 0) {
        std::optional<PieceColor> left_pawn = checkOccupation(left_index);
        Square left_square = Square::fromIndex(left_index).value();
        if(!left_pawn.has_value()) moves.push_back(Move(current_square, left_square));
        else if (left_pawn.value() != current_turn) moves.push_back(Move(current_square, left_square));
    }
    if(right_rank_distance == 0) {
        std::optional<PieceColor> right_pawn = checkOccupation(right_index);
        Square right_square = Square::fromIndex(right_index).value();
        if(!right_pawn.has_value()) moves.push_back(Move(current_square, right_square));
        else if (right_pawn.value() != current_turn) moves.push_back(Move(current_square, right_square));
    }

    //Castling
    //TODO: check of destination wordt aangevallen
    switch(current_turn) {
        case PieceColor::White :
            if(static_cast<bool>(castling_rights & CastlingRights::WhiteKingside)) {
                if(!checkOccupation(right_index).has_value()) {
                    Square::Index right_right_index = rightIndex(right_index);
                    if(!checkOccupation(right_right_index).has_value()) {
                        moves.push_back(Move(current_square, Square::fromIndex(right_right_index).value()));
                    }
                }
            }
            if(static_cast<bool>(castling_rights & CastlingRights::WhiteQueenside)) {
                if(!checkOccupation(left_index).has_value()) {
                    Square::Index left_left_index = leftIndex(left_index);
                    if(!checkOccupation(left_left_index).has_value()) {
                        if(!checkOccupation(leftIndex(left_left_index)).has_value()) moves.push_back(Move(current_square, Square::fromIndex(left_left_index).value()));
                    }
                }
            }
            break;
        case PieceColor::Black :
            if(static_cast<bool>(castling_rights & CastlingRights::BlackQueenside)) {
                if(!checkOccupation(right_index).has_value()) {
                    Square::Index right_right_index = rightIndex(right_index);
                    if(!checkOccupation(right_right_index).has_value()) {
                        if(!checkOccupation(rightIndex(right_right_index)).has_value()) moves.push_back(Move(current_square, Square::fromIndex(right_right_index).value()));
                    }
                }
            }
            if(static_cast<bool>(castling_rights & CastlingRights::BlackKingside)) {
                if(!checkOccupation(left_index).has_value()) {
                    Square::Index left_left_index = leftIndex(left_index);
                    if(!checkOccupation(left_left_index).has_value()) {
                        moves.push_back(Move(current_square, Square::fromIndex(left_left_index).value()));
                    }
                }
            }
            break;
    }
}

void Board::pseudoLegalKnightMovesFrom(Square::Index knight_index, Board::MoveVec &moves) const {

    Square current_square = Square::fromIndex(knight_index).value();

    //front knight
    if(!isOutOfRange(frontIndex(frontIndex(knight_index)))) {
        Square::Index front_left_index = frontLeftIndex(frontIndex(knight_index));
        Square::Index front_right_index = frontRightIndex(frontIndex(knight_index));

        signed front_left_rank_distance = abs(static_cast<signed>((front_left_index / 8)) - static_cast<signed>((knight_index / 8)));
        signed front_right_rank_distance = abs(static_cast<signed>((front_right_index / 8)) - static_cast<signed>((knight_index / 8)));

        if(front_left_rank_distance == 2) {
            std::optional<PieceColor> front_left_pawn = checkOccupation(front_left_index);
            Square front_left_square = Square::fromIndex(front_left_index).value();
            if(!front_left_pawn.has_value()) { //Capture possible
                moves.push_back(Move(current_square, front_left_square));
            } else if (front_left_pawn.value() != current_turn) {
                moves.push_back(Move(current_square, front_left_square));
            }
        }

        if(front_right_rank_distance == 2) {
            std::optional<PieceColor> front_right_pawn = checkOccupation(front_right_index);
            Square front_right_square = Square::fromIndex(front_right_index).value();
            if(!front_right_pawn.has_value()) { //Capture possible
                moves.push_back(Move(current_square, front_right_square));
            } else if (front_right_pawn.value() != current_turn) {
                moves.push_back(Move(current_square, front_right_square));
            }
        }
    }

    //back knight
    if(!isOutOfRange(backIndex(backIndex(knight_index)))) {
        Square::Index back_left_index = backLeftIndex(backIndex(knight_index));
        Square::Index back_right_index = backRightIndex(backIndex(knight_index));

        signed back_left_rank_distance = abs(static_cast<signed>((back_left_index / 8)) - static_cast<signed>((knight_index / 8)));
        signed back_right_rank_distance = abs(static_cast<signed>((back_right_index / 8)) - static_cast<signed>((knight_index / 8)));

        if(back_left_rank_distance == 2) {
            std::optional<PieceColor> back_left_pawn = checkOccupation(back_left_index);
            Square back_left_square = Square::fromIndex(back_left_index).value();
            if(!back_left_pawn.has_value()) { //Capture possible
                moves.push_back(Move(current_square, back_left_square));
            } else if (back_left_pawn.value() != current_turn) {
                moves.push_back(Move(current_square, back_left_square));
            }
        }

        if(back_right_rank_distance == 2) {
            std::optional<PieceColor> back_right_pawn = checkOccupation(back_right_index);
            Square back_right_square = Square::fromIndex(back_right_index).value();
            if(!back_right_pawn.has_value()) { //Capture possible
                moves.push_back(Move(current_square, back_right_square));
            } else if (back_right_pawn.value() != current_turn) {
                moves.push_back(Move(current_square, back_right_square));
            }
        }
    }

    //left and right knight
    Square::Index left_front_index = frontLeftIndex(leftIndex(knight_index));
    Square::Index left_back_index = backLeftIndex(leftIndex(knight_index));
    Square::Index right_front_index = frontRightIndex(rightIndex(knight_index));
    Square::Index right_back_index = backRightIndex(rightIndex(knight_index));


    signed left_front_rank_distance = abs(static_cast<signed>((left_front_index / 8)) - static_cast<signed>((knight_index / 8)));
    signed left_back_rank_distance = abs(static_cast<signed>((left_back_index / 8)) - static_cast<signed>((knight_index / 8)));
    signed right_front_rank_distance = abs(static_cast<signed>((right_front_index / 8)) - static_cast<signed>((knight_index / 8)));
    signed right_back_rank_distance = abs(static_cast<signed>((right_back_index / 8)) - static_cast<signed>((knight_index / 8)));

    if(!isOutOfRange(left_front_index)) {
        if(left_front_rank_distance == 1) {
            std::optional<PieceColor> left_front_pawn = checkOccupation(left_front_index);
            Square left_front_square = Square::fromIndex(left_front_index).value();
            if(!left_front_pawn.has_value()) { //Capture possible
                moves.push_back(Move(current_square, left_front_square));
            } else if (left_front_pawn.value() != current_turn) {
                moves.push_back(Move(current_square, left_front_square));
            }
        }
    }

    if(!isOutOfRange(left_back_index)) {
        if(left_back_rank_distance == 1) {
            std::optional<PieceColor> left_back_pawn = checkOccupation(left_back_index);
            Square left_back_square = Square::fromIndex(left_back_index).value();
            if(!left_back_pawn.has_value()) { //Capture possible
                moves.push_back(Move(current_square, left_back_square));
            } else if (left_back_pawn.value() != current_turn) {
                moves.push_back(Move(current_square, left_back_square));
            }
        }
    }

    if(!isOutOfRange(right_front_index)) {
        if(right_front_rank_distance == 1) {
            std::optional<PieceColor> right_front_pawn = checkOccupation(right_front_index);
            Square right_front_square = Square::fromIndex(right_front_index).value();
            if(!right_front_pawn.has_value()) { //Capture possible
                moves.push_back(Move(current_square, right_front_square));
            } else if (right_front_pawn.value() != current_turn) {
                moves.push_back(Move(current_square, right_front_square));
            }
        }
    }

    if(!isOutOfRange(right_back_index)) {
        if(right_back_rank_distance == 1) {
            std::optional<PieceColor> right_back_pawn = checkOccupation(right_back_index);
            Square right_back_square = Square::fromIndex(right_back_index).value();
            if(!right_back_pawn.has_value()) { //Capture possible
                moves.push_back(Move(current_square, right_back_square));
            } else if (right_back_pawn.value() != current_turn) {
                moves.push_back(Move(current_square, right_back_square));
            }
        }
    }
}

void Board::pseudoLegalRookMovesFrom(Square::Index rook_index, Board::MoveVec &moves) const {
    Square current_square = Square::fromIndex(rook_index).value();

    bool collided = false;

    Square::Index work_front_index = frontIndex(rook_index);
    while(!collided && !isOutOfRange(work_front_index)) {
        std::optional<PieceColor> occupation = checkOccupation(work_front_index);
        Square front_square = Square::fromIndex(work_front_index).value();
        if(!occupation.has_value()) {
            moves.push_back(Move(current_square,front_square));
            work_front_index = frontIndex(work_front_index);
        } else {
            collided = true;
            if(occupation.value() != current_turn) moves.push_back(Move(current_square, front_square));
        }
    }

    collided = false;
    Square::Index work_back_index = backIndex(rook_index);
    while(!collided && !isOutOfRange(work_back_index)) {
        std::optional<PieceColor> occupation = checkOccupation(work_back_index);
        Square back_square = Square::fromIndex(work_back_index).value();
        if(!occupation.has_value()) {
            moves.push_back(Move(current_square,back_square));
            work_back_index = backIndex(work_back_index);
        } else {
            collided = true;
            if(occupation.value() != current_turn) moves.push_back(Move(current_square, back_square));
        }
    }

    collided = false;
    Square::Index work_left_index = leftIndex(rook_index);
    while(!collided && abs(static_cast<signed>((rook_index / 8)) - static_cast<signed>((work_left_index / 8))) == 0) {
        std::optional<PieceColor> occupation = checkOccupation(work_left_index);
        Square left_square = Square::fromIndex(work_left_index).value();
        if(!occupation.has_value()) {
            moves.push_back(Move(current_square,left_square));
            work_left_index = leftIndex(work_left_index);
        } else {
            collided = true;
            if(occupation.value() != current_turn) moves.push_back(Move(current_square, left_square));
        }
    }

    collided = false;
    Square::Index work_right_index = rightIndex(rook_index);
    while(!collided && abs(static_cast<signed>((rook_index / 8)) - static_cast<signed>((work_right_index / 8))) == 0) {
        std::optional<PieceColor> occupation = checkOccupation(work_right_index);
        Square right_square = Square::fromIndex(work_right_index).value();
        if(!occupation.has_value()) {
            moves.push_back(Move(current_square,right_square));
            work_right_index = rightIndex(work_right_index);
        } else {
            collided = true;
            if(occupation.value() != current_turn) moves.push_back(Move(current_square, right_square));
        }
    }
}

void Board::pseudoLegalBishopMovesFrom(Square::Index bishop_index, Board::MoveVec &moves) const {
    Square current_square = Square::fromIndex(bishop_index).value();

    bool collided = false;
    Square::Index work_front_left_index = frontLeftIndex(bishop_index);
    bool start_color = piecePositions.blacks[bishop_index];

    while(!collided && !isOutOfRange(work_front_left_index) && piecePositions.blacks[work_front_left_index] == start_color) {
        std::optional<PieceColor> occupation = checkOccupation(work_front_left_index);
        Square front_left_square = Square::fromIndex(work_front_left_index).value();
        if(!occupation.has_value()) {
            moves.push_back(Move(current_square,front_left_square));
            work_front_left_index = frontLeftIndex(work_front_left_index);
        } else {
            collided = true;
            if(occupation.value() != current_turn) moves.push_back(Move(current_square, front_left_square));
        }
    }

    collided = false;
    Square::Index work_front_right_index = frontRightIndex(bishop_index);

    while(!collided && !isOutOfRange(work_front_right_index) && piecePositions.blacks[work_front_right_index] == start_color) {
        std::optional<PieceColor> occupation = checkOccupation(work_front_right_index);
        Square front_right_square = Square::fromIndex(work_front_right_index).value();
        if(!occupation.has_value()) {
            moves.push_back(Move(current_square,front_right_square));
            work_front_right_index = frontRightIndex(work_front_right_index);
        } else {
            collided = true;
            if(occupation.value() != current_turn) moves.push_back(Move(current_square, front_right_square));
        }
    }

    collided = false;
    Square::Index work_back_right_index = backRightIndex(bishop_index);

    while(!collided && !isOutOfRange(work_back_right_index) && piecePositions.blacks[work_back_right_index] == start_color) {
        std::optional<PieceColor> occupation = checkOccupation(work_back_right_index);
        Square back_right_square = Square::fromIndex(work_back_right_index).value();
        if(!occupation.has_value()) {
            moves.push_back(Move(current_square,back_right_square));
            work_back_right_index = backRightIndex(work_back_right_index);
        } else {
            collided = true;
            if(occupation.value() != current_turn) moves.push_back(Move(current_square, back_right_square));
        }
    }

    collided = false;
    Square::Index work_back_left_index = backLeftIndex(bishop_index);

    while(!collided && !isOutOfRange(work_back_left_index) && piecePositions.blacks[work_back_left_index] == start_color) {
        std::optional<PieceColor> occupation = checkOccupation(work_back_left_index);
        Square back_left_square = Square::fromIndex(work_back_left_index).value();
        if(!occupation.has_value()) {
            moves.push_back(Move(current_square,back_left_square));
            work_back_left_index = backLeftIndex(work_back_left_index);
        } else {
            collided = true;
            if(occupation.value() != current_turn) moves.push_back(Move(current_square, back_left_square));
        }
    }
}

void Board::pseudoLegalQueenMovesFrom(Square::Index index, Board::MoveVec &moves) const {
    pseudoLegalRookMovesFrom(index, moves);
    pseudoLegalBishopMovesFrom(index, moves);
}

std::ostream& operator<<(std::ostream& os, const Board& board) {
    (void)board;
    return os;
}
