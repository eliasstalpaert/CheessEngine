#include "Board.hpp"

#include <ostream>
#include <cassert>
#include <cmath>

Board::Board() : checked_player{std::nullopt}, en_passant_square{std::nullopt}
{
}

Board::Board(const Board &board) : checked_player{board.checked_player}, piecePositions{board.piecePositions}, colorPositions{board.colorPositions}, current_turn{board.current_turn}, castling_rights{board.castling_rights}, en_passant_square{board.en_passant_square}
{

}

Board &Board::operator=(const Board &board) {
    checked_player = board.checked_player;
    piecePositions = board.piecePositions;
    colorPositions = board.colorPositions;
    current_turn = board.current_turn;
    castling_rights = board.castling_rights;
    en_passant_square = board.en_passant_square;
    return *this;
}

void Board::setPiece(const Square& square, const Piece::Optional& piece) {
    Square::Index square_index = square.index();

    switch (piece->color()) {
        case PieceColor::White:
            colorPositions.white[square_index] = 1;
            colorPositions.black[square_index] = 0;
            break;
        case PieceColor::Black:
            colorPositions.black[square_index] = 1;
            colorPositions.white[square_index] = 0;
            break;
        default: //Never occurs
            break;
    }

    //Clear bit in all bitboards (avoid checking for each one)
    piecePositions.clearBit(square_index);

    switch (piece->type()) {
        case PieceType::Pawn:
            piecePositions.pawns[square_index] = 1;
            break;
        case PieceType::Knight:
            piecePositions.knights[square_index] = 1;
            break;
        case PieceType::Bishop:
            piecePositions.bishops[square_index] = 1;
            break;
        case PieceType::Rook:
            piecePositions.rooks[square_index] = 1;
            break;
        case PieceType::Queen:
            piecePositions.queen[square_index] = 1;
            break;
        case PieceType::King:
            piecePositions.king[square_index] = 1;
            break;
        default: //Never occurs
            break;
    }
}


Piece::Optional Board::piece(const Square& square) const {
    Square::Index index = square.index();
    PieceColor color;

    if(isOutOfRange(index)) return std::nullopt;

    if(colorPositions.white[index]) color = PieceColor::White;
    else if(colorPositions.black[index]) color = PieceColor::Black;
    else return std::nullopt;

    char candidate_symbol = 'x'; //empty square
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

unsigned Board::getAmountOfPiece(PieceColor color, PieceType piece_type) const {
    std::bitset<64> color_bits;
    switch(color) {
        case PieceColor::White :
            color_bits = colorPositions.white;
            break;
        case PieceColor::Black :
            color_bits = colorPositions.black;
            break;
    }

    switch(piece_type) {
        case PieceType::Pawn :
            return (color_bits & piecePositions.pawns).count();
        case PieceType::Rook :
            return (color_bits & piecePositions.rooks).count();
        case PieceType::Bishop :
            return (color_bits & piecePositions.bishops).count();
        case PieceType::Knight :
            return (color_bits & piecePositions.knights).count();
        case PieceType::Queen :
            return (color_bits & piecePositions.queen).count();
        case PieceType::King :
            return (color_bits & piecePositions.king).count();
    }
    // never reached
    return 0;
}

std::bitset<64> Board::getColorPositions(PieceColor turn) const {
    switch (turn) {
        case PieceColor::White :
            return colorPositions.white;
        case PieceColor::Black :
            return colorPositions.black;
    }
    //never reached
    return 0;
}


/**************
 *
 * LEGAL MOVE GENERATION
 *
 * ******************/

bool Board::isPlayerChecked(PieceColor turn) const {
    std::bitset<64> player_king;
    switch(turn) {
        case PieceColor::White :
            player_king = colorPositions.white & piecePositions.king;
            break;
        case PieceColor::Black :
            player_king = colorPositions.black & piecePositions.king;
            break;
    }
    for(size_t i = 0; i < player_king.size(); i++) {
        if(player_king[i]) return isSquareAttacked(turn, i);
    }

    //never reached
    return false;
}


/********************************************************
 *
 * MOVE MAKING
 *
 * *****************************************************************/

//Returns the type of piece captured in case of a capture
std::optional<PieceType> Board::clearCapturePiece(const Square &square, bool capture) {
    Piece::Optional occupy_piece = piece(square);
    if(occupy_piece.has_value()) {
        switch (occupy_piece->type()) {
            case PieceType::Pawn :
                piecePositions.pawns[square.index()] = 0;
                break;
            case PieceType::Knight :
                piecePositions.knights[square.index()] = 0;
                break;
            case PieceType::Bishop :
                piecePositions.bishops[square.index()] = 0;
                break;
            case PieceType::Rook :
                piecePositions.rooks[square.index()] = 0;
                break;
            case PieceType::Queen :
                piecePositions.queen[square.index()] = 0;
                break;
            case PieceType::King :
                if(capture) return occupy_piece->type(); //early return to not clear colorpositions
                else piecePositions.king[square.index()] = 0;
                break;
        }

        switch (occupy_piece->color()) {
            case PieceColor::White :
                colorPositions.white[square.index()] = 0;
                break;
            case PieceColor::Black :
                colorPositions.black[square.index()] = 0;
                break;
        }
        return occupy_piece->type();
    } else return std::nullopt;
}

//Performs the current move/capture move and if king is taken, no pieces are modified but checkmate is set
void Board::makeMove(const Move& move) {
    Square from_square = move.from();
    Square::Index from_index = from_square.index();
    Piece::Optional from_piece = piece(from_square);
    Square to_square = move.to();
    Square::Index to_index = to_square.index();
    std::optional<PieceType> promotion = move.promotion();

    //Capturecheck is in clearpiece
    std::optional<PieceType> captured_piece = clearCapturePiece(to_square, true);

    if(captured_piece.has_value() && captured_piece.value() == PieceType::King) {
        //DO NOTHING
    } else {
        clearCapturePiece(from_square, false);

        //Castling move: also moves the rook!
        if(from_piece->type() == PieceType::King) {
            signed move_distance = static_cast<signed>(to_index) - static_cast<signed>(from_index);

            switch(current_turn) {
                case PieceColor::White :
                    if(abs(move_distance) == 2) {
                        if(std::signbit(move_distance)) {
                            //Queenside
                            clearCapturePiece(Square::A1, false);
                            setPiece(Square::fromIndex(to_index + 1).value(), Piece(PieceColor::White, PieceType::Rook));
                        }
                        else {
                            //Kingside
                            clearCapturePiece(Square::H1, false);
                            setPiece(Square::fromIndex(to_index - 1).value(), Piece(PieceColor::White, PieceType::Rook));
                        }
                    }
                    //Update castling rights
                    castling_rights &= CastlingRights::Black;
                    break;
                case PieceColor::Black :
                    if(abs(move_distance) == 2) {
                        if(std::signbit(move_distance)) {
                            //Queenside
                            clearCapturePiece(Square::A8, false);
                            setPiece(Square::fromIndex(to_index + 1).value(), Piece(PieceColor::Black, PieceType::Rook));
                        }
                        else {
                            //Kingside
                            clearCapturePiece(Square::H8, false);
                            setPiece(Square::fromIndex(to_index - 1).value(), Piece(PieceColor::Black, PieceType::Rook));
                        }
                    }
                    //Update castling rights
                    castling_rights &= CastlingRights::White;
                    break;
            }

        }

        //CastlingRights change upon Rook movement
        if(from_piece->type() == PieceType::Rook) {
            switch(from_index) {
                //Color checking is not needed
                case 0 :
                    //White queenside
                    castling_rights &= ~CastlingRights::WhiteQueenside;
                    break;
                case 7 :
                    //White kingside
                    castling_rights &= ~CastlingRights::WhiteKingside;
                    break;
                case 56 :
                    //Black queenside
                    castling_rights &= ~CastlingRights::BlackQueenside;
                    break;
                case 63 :
                    //Black kingside
                    castling_rights &= ~CastlingRights::BlackKingside;
                    break;
            }
        }

        //CastlingRights change upon Rook capture/occupation
        switch(to_index) {
            case 0 :
                //White queenside
                castling_rights &= ~CastlingRights::WhiteQueenside;
                break;
            case 7 :
                //White kingside
                castling_rights &= ~CastlingRights::WhiteKingside;
                break;
            case 56 :
                //Black queenside
                castling_rights &= ~CastlingRights::BlackQueenside;
                break;
            case 63 :
                //Black kingside
                castling_rights &= ~CastlingRights::BlackKingside;
                break;
        }

        //Pawn en passant move
        if(from_piece->type() == PieceType::Pawn) {
            //eps capture
            if(en_passant_square.has_value()) {
                if(to_square == en_passant_square){
                    clearCapturePiece(Square::fromIndex(backIndex(en_passant_square->index())).value(), true); //Back square from perspective of current turn
                }
                en_passant_square = std::nullopt;
            }
            //Check for new eps
            if(square_color[to_index] == square_color[from_index]) {
                Square::Index left_index = leftIndex(to_index);
                Square::Index right_index = rightIndex(to_index);

                if(square_color[left_index] != square_color[to_index]) {
                    Piece::Optional left_piece = piece(Square::fromIndex(left_index).value());
                    if(left_piece.has_value()) {
                        if(left_piece->type() == PieceType::Pawn && left_piece->color() == !current_turn) en_passant_square = Square::fromIndex(
                                    frontIndex(from_index));
                    }
                }

                if(square_color[right_index] != square_color[to_index]) {
                    Piece::Optional right_piece = piece(Square::fromIndex(right_index).value());
                    if(right_piece.has_value()) {
                        if(right_piece->type() == PieceType::Pawn && right_piece->color() == !current_turn) en_passant_square = Square::fromIndex(
                                    frontIndex(from_index));
                    }
                }
            }

        } else if (en_passant_square.has_value()) en_passant_square = std::nullopt; //if not a pawn move and there was eps, eps is expired

        //Pawn promotion move
        if(promotion.has_value()) {
            switch (promotion.value()) {
                case PieceType::Knight :
                    setPiece(to_square, Piece(from_piece->color(), PieceType::Knight));
                    break;
                case PieceType::Rook :
                    setPiece(to_square, Piece(from_piece->color(), PieceType::Rook));
                    break;
                case PieceType::Bishop :
                    setPiece(to_square, Piece(from_piece->color(), PieceType::Bishop));
                    break;
                case PieceType::Queen :
                    setPiece(to_square, Piece(from_piece->color(), PieceType::Queen));
                    break;
                default : //never occurs with legal moves
                    break;
            }
        } else setPiece(to_square, from_piece);
    }

    //Turn changes
    current_turn = !current_turn;
}


/********************************************************
 *
 * MOVE GENERATION
 *
 * *****************************************************************/

bool Board::isSquareAttacked(PieceColor turn, Square::Index index) const {

    //signed index_rank = index / 8;

    bool index_color = square_color[index];

    //sliding pieces (Q/B/R)
    Square::Index working_index = frontIndex(index, turn);
    //N
    while(!isOutOfRange(working_index)) {
        std::optional<PieceColor> occupying = checkOccupation(working_index);
        if(checkOccupation(working_index).has_value()) {
            if(occupying != turn) {
                if(working_index == frontIndex(index, turn) && piecePositions.king[working_index]) return true;
                if(piecePositions.rooks[working_index] || piecePositions.queen[working_index]) return true;
                else break;
            }
            else break;
        }
        else working_index = frontIndex(working_index, turn);
    }
    //S
    working_index = backIndex(index, turn);
    while(!isOutOfRange(working_index)) {
        std::optional<PieceColor> occupying = checkOccupation(working_index);
        if(checkOccupation(working_index).has_value()) {
            if(occupying != turn) {
                if(working_index == backIndex(index, turn) && piecePositions.king[working_index]) return true;
                if(piecePositions.rooks[working_index] || piecePositions.queen[working_index]) return true;
                else break;
            }
            else break;
        }
        else working_index = backIndex(working_index, turn);
    }
    //E
    working_index = rightIndex(index, turn);
    while(!isOutOfRange(working_index)) {
        std::optional<PieceColor> occupying = checkOccupation(working_index);
        if(checkOccupation(working_index).has_value()) {
            if(occupying != turn) {
                if(working_index == rightIndex(index, turn) && piecePositions.king[working_index]) return true;
                if(piecePositions.rooks[working_index] || piecePositions.queen[working_index]) return true;
                else break;
            }
            else break;
        }
        else {
            Square::Index new_index = rightIndex(working_index, turn);
            if(square_color[new_index] == index_color) break;
            else working_index = new_index;
        }
    }
    //W
    working_index = leftIndex(index, turn);
    while(!isOutOfRange(working_index)) {
        std::optional<PieceColor> occupying = checkOccupation(working_index);
        if(checkOccupation(working_index).has_value()) {
            if(occupying != turn) {
                if(working_index == leftIndex(index, turn) && piecePositions.king[working_index]) return true;
                if(piecePositions.rooks[working_index] || piecePositions.queen[working_index]) return true;
                else break;
            }
            else break;
        }
        else {
            Square::Index new_index = leftIndex(working_index, turn);
            if(square_color[new_index] == index_color) break;
            else working_index = new_index;
        }
    }
    //NE
    working_index = frontRightIndex(index, turn);
    while(!isOutOfRange(working_index)) {
        std::optional<PieceColor> occupying = checkOccupation(working_index);
        if(checkOccupation(working_index).has_value()) {
            if(occupying != turn) {
                if(working_index == frontRightIndex(index, turn) && (piecePositions.pawns[working_index] || piecePositions.king[working_index])) return true;
                if(piecePositions.bishops[working_index] || piecePositions.queen[working_index]) return true;
                else break;
            }
            else break;
        }
        else {
            Square::Index new_index = frontRightIndex(working_index, turn);
            if(square_color[new_index] != index_color) break;
            else working_index = new_index;
        }
    }
    //NW
    working_index = frontLeftIndex(index, turn);
    while(!isOutOfRange(working_index)) {
        std::optional<PieceColor> occupying = checkOccupation(working_index);
        if(checkOccupation(working_index).has_value()) {
            if(occupying != turn) {
                if(working_index == frontLeftIndex(index, turn) && (piecePositions.pawns[working_index] || piecePositions.king[working_index])) return true;
                if(piecePositions.bishops[working_index] || piecePositions.queen[working_index]) return true;
                else break;
            }
            else break;
        }
        else {
            Square::Index new_index = frontLeftIndex(working_index, turn);
            if(square_color[new_index] != index_color) break;
            else working_index = new_index;
        }
    }
    //SE
    working_index = backRightIndex(index, turn);
    while(!isOutOfRange(working_index)) {
        std::optional<PieceColor> occupying = checkOccupation(working_index);
        if(checkOccupation(working_index).has_value()) {
            if(occupying != turn) {
                if(working_index == backRightIndex(index, turn) && piecePositions.king[working_index]) return true;
                if(piecePositions.bishops[working_index] || piecePositions.queen[working_index]) return true;
                else break;
            }
            else break;
        }
        else {
            Square::Index new_index = backRightIndex(working_index, turn);
            if(square_color[new_index] != index_color) break;
            else working_index = new_index;
        }
    }
    //SW
    working_index = backLeftIndex(index, turn);
    while(!isOutOfRange(working_index)) {
        std::optional<PieceColor> occupying = checkOccupation(working_index);
        if(checkOccupation(working_index).has_value()) {
            if(occupying != turn) {
                if(working_index == backLeftIndex(index, turn) && piecePositions.king[working_index]) return true;
                if(piecePositions.bishops[working_index] || piecePositions.queen[working_index]) return true;
                else break;
            }
            else break;
        }
        else {
            Square::Index new_index = backLeftIndex(working_index, turn);
            if(square_color[new_index] != index_color) break;
            else working_index = new_index;
        }
    }
    ///knights
    //front
    if(!isOutOfRange(frontIndex(frontIndex(index, turn), turn))) {
        Square::Index front_left_index = frontLeftIndex(frontIndex(index, turn), turn);
        Square::Index front_right_index = frontRightIndex(frontIndex(index, turn), turn);

        if(square_color[front_left_index] != index_color) {
            std::optional<PieceColor> occupying = checkOccupation(front_left_index);
            if(occupying.has_value() && occupying != turn) {
                if(piecePositions.knights[front_left_index]) return true;
            }
        }

        if(square_color[front_right_index] != index_color) {
            std::optional<PieceColor> occupying = checkOccupation(front_right_index);
            if(occupying.has_value() && occupying != turn) {
                if(piecePositions.knights[front_right_index]) return true;
            }
        }

    }
    //back
    if(!isOutOfRange(backIndex(backIndex(index, turn), turn))) {
        Square::Index back_left_index = backLeftIndex(backIndex(index, turn), turn);
        Square::Index back_right_index = backRightIndex(backIndex(index, turn), turn);

        if(square_color[back_left_index] != index_color) {
            std::optional<PieceColor> occupying = checkOccupation(back_left_index);
            if(occupying.has_value() && occupying != turn) {
                if(piecePositions.knights[back_left_index]) return true;
            }
        }

        if(square_color[back_right_index] != index_color) {
            std::optional<PieceColor> occupying = checkOccupation(back_right_index);
            if(occupying.has_value() && occupying != turn) {
                if(piecePositions.knights[back_right_index]) return true;
            }
        }

    }
    //left and right
    Square::Index left_front_index = frontLeftIndex(leftIndex(index, turn), turn);
    Square::Index left_back_index = backLeftIndex(leftIndex(index, turn), turn);
    Square::Index right_front_index = frontRightIndex(rightIndex(index, turn), turn);
    Square::Index right_back_index = backRightIndex(rightIndex(index, turn), turn);

    if(!isOutOfRange(left_front_index)) {
        if(square_color[left_front_index] != index_color) {
            std::optional<PieceColor> occupying = checkOccupation(left_front_index);
            if(occupying.has_value() && occupying != turn) {
                if(piecePositions.knights[left_front_index]) return true;
            }
        }
    }

    if(!isOutOfRange(left_back_index)) {
        if(square_color[left_back_index] != index_color) {
            std::optional<PieceColor> occupying = checkOccupation(left_back_index);
            if(occupying.has_value() && occupying != turn) {
                if(piecePositions.knights[left_back_index]) return true;
            }
        }
    }

    if(!isOutOfRange(right_front_index)) {
        if(square_color[right_front_index] != index_color) {
            std::optional<PieceColor> occupying = checkOccupation(right_front_index);
            if(occupying.has_value() && occupying != turn) {
                if(piecePositions.knights[right_front_index]) return true;
            }
        }
    }

    if(!isOutOfRange(right_back_index)) {
        if(square_color[right_back_index] != index_color) {
            std::optional<PieceColor> occupying = checkOccupation(right_back_index);
            if(occupying.has_value() && occupying != turn) {
                if(piecePositions.knights[right_back_index]) return true;
            }
        }
    }

    //en passant
    if(en_passant_square->index() == backIndex(index, turn)) return true;

    //not attacked
    return false;
}

//Generate pseudolegal moves for the current player
void Board::pseudoLegalMoves( MoveVec& moves ) const {
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

    bool diag_color = square_color[pawn_index];

    //Front left captures check
    if(square_color[front_left_index] == diag_color) {
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
    if(square_color[front_right_index] == diag_color) {
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

        if(front_right_index == en_passant_index && square_color[front_right_index] == diag_color) {
            moves.push_back(Move(current_square, Square::fromIndex(front_right_index).value()));
        }
        if (front_left_index == en_passant_index && square_color[front_left_index] == diag_color) {
            moves.push_back(Move(current_square, Square::fromIndex(front_left_index).value()));
        }

    }

}

void Board::pseudoLegalKingMovesFrom(Square::Index king_index, Board::MoveVec &moves) const {
    //TODO: code duplication (maybe sth with giving directions to check)
    Square current_square = Square::fromIndex(king_index).value();

    Square::Index front_left_index = frontLeftIndex(king_index);
    Square::Index front_right_index = frontRightIndex(king_index);
    Square::Index back_left_index = backLeftIndex(king_index);
    Square::Index back_right_index = backRightIndex(king_index);
    Square::Index left_index = leftIndex(king_index);
    Square::Index right_index = rightIndex(king_index);

    bool diag_color = square_color[king_index];

    if(!lastRankCheck(king_index)) {
        //Front
        Square::Index front_index = frontIndex(king_index);
        std::optional<PieceColor> front_pawn = checkOccupation(front_index);
        Square front_square = Square::fromIndex(front_index).value();
        if(!front_pawn.has_value()) moves.push_back(Move(current_square, front_square));
        else if (front_pawn.value() != current_turn) moves.push_back(Move(current_square, front_square));
        //Front-Left
        if(square_color[front_left_index] == diag_color) {
            std::optional<PieceColor> front_left_pawn = checkOccupation(front_left_index);
            Square front_left_square = Square::fromIndex(front_left_index).value();
            if(!front_left_pawn.has_value()) moves.push_back(Move(current_square, front_left_square));
            else if (front_left_pawn.value() != current_turn) moves.push_back(Move(current_square, front_left_square));
        }
        //Front-Right
        if(square_color[front_right_index] == diag_color) {
            std::optional<PieceColor> front_right_pawn = checkOccupation(front_right_index);
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
        if(square_color[back_left_index] == diag_color) {
            std::optional<PieceColor> back_left_pawn = checkOccupation(back_left_index);
            Square back_left_square = Square::fromIndex(back_left_index).value();
            if(!back_left_pawn.has_value()) moves.push_back(Move(current_square, back_left_square));
            else if (back_left_pawn.value() != current_turn) moves.push_back(Move(current_square, back_left_square));
        }
        //back-Right
        if(square_color[back_right_index] == diag_color) {
            std::optional<PieceColor> back_right_pawn = checkOccupation(back_right_index);
            Square back_right_square = Square::fromIndex(back_right_index).value();
            if(!back_right_pawn.has_value()) moves.push_back(Move(current_square, back_right_square));
            else if (back_right_pawn.value() != current_turn) moves.push_back(Move(current_square, back_right_square));
        }
    }

    if(square_color[left_index] != diag_color) {
        std::optional<PieceColor> left_pawn = checkOccupation(left_index);
        Square left_square = Square::fromIndex(left_index).value();
        if(!left_pawn.has_value()) moves.push_back(Move(current_square, left_square));
        else if (left_pawn.value() != current_turn) moves.push_back(Move(current_square, left_square));
    }
    if(square_color[right_index] != diag_color) {
        std::optional<PieceColor> right_pawn = checkOccupation(right_index);
        Square right_square = Square::fromIndex(right_index).value();
        if(!right_pawn.has_value()) moves.push_back(Move(current_square, right_square));
        else if (right_pawn.value() != current_turn) moves.push_back(Move(current_square, right_square));
    }

    //Castling moves
    if(!isSquareAttacked(current_turn, king_index)) {
        Square::Index right_right_index = rightIndex(right_index);
        Square::Index left_left_index = leftIndex(left_index);
        //Check rights and add move, bounds checking not needed if rights are present
        switch(current_turn) {
            case PieceColor::White :
                if( static_cast<bool>(castling_rights & CastlingRights::WhiteKingside)) {
                    if(!checkOccupation(right_index).has_value() && !isSquareAttacked(PieceColor::White, right_index)) {
                        if(!checkOccupation(right_right_index).has_value() && !isSquareAttacked(PieceColor::White, right_right_index)) {
                            moves.push_back(Move(current_square, Square::fromIndex(right_right_index).value()));
                        }
                    }
                }
                if( static_cast<bool>(castling_rights & CastlingRights::WhiteQueenside)) {
                    if(!checkOccupation(left_index).has_value() && !isSquareAttacked(PieceColor::White, left_index)) {
                        if(!checkOccupation(left_left_index).has_value() && !isSquareAttacked(PieceColor::White, left_left_index)) {
                            //Extra queenside check due to bigger distance
                            if(!checkOccupation(leftIndex(left_left_index)).has_value()) moves.push_back(Move(current_square, Square::fromIndex(left_left_index).value()));
                        }
                    }
                }
                break;
            case PieceColor::Black :
                if( static_cast<bool>(castling_rights & CastlingRights::BlackQueenside)) {
                    if(!checkOccupation(right_index).has_value() && !isSquareAttacked(PieceColor::Black, right_index)) {
                        if(!checkOccupation(right_right_index).has_value() && !isSquareAttacked(PieceColor::Black, right_right_index)) {
                            //Extra queenside check due to bigger distance
                            if(!checkOccupation(rightIndex(right_right_index)).has_value()) moves.push_back(Move(current_square, Square::fromIndex(right_right_index).value()));
                        }
                    }
                }
                if( static_cast<bool>(castling_rights & CastlingRights::BlackKingside)) {
                    if(!checkOccupation(left_index).has_value() && !isSquareAttacked(PieceColor::Black, left_index)) {
                        if(!checkOccupation(left_left_index).has_value() && !isSquareAttacked(PieceColor::Black, left_left_index)) {
                            moves.push_back(Move(current_square, Square::fromIndex(left_left_index).value()));
                        }
                    }
                }
                break;
        }
    }

}

void Board::pseudoLegalKnightMovesFrom(Square::Index knight_index, Board::MoveVec &moves) const {

    Square current_square = Square::fromIndex(knight_index).value();
    bool knight_square_color = square_color[knight_index];

    //front knight
    if(!isOutOfRange(frontIndex(frontIndex(knight_index)))) {
        Square::Index front_left_index = frontLeftIndex(frontIndex(knight_index));
        Square::Index front_right_index = frontRightIndex(frontIndex(knight_index));

        if(square_color[front_left_index] != knight_square_color) {
            std::optional<PieceColor> front_left_pawn = checkOccupation(front_left_index);
            Square front_left_square = Square::fromIndex(front_left_index).value();
            if(!front_left_pawn.has_value()) { //Capture possible
                moves.push_back(Move(current_square, front_left_square));
            } else if (front_left_pawn.value() != current_turn) {
                moves.push_back(Move(current_square, front_left_square));
            }
        }

        if(square_color[front_right_index] != knight_square_color) {
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

        if(square_color[back_left_index] != knight_square_color) {
            std::optional<PieceColor> back_left_pawn = checkOccupation(back_left_index);
            Square back_left_square = Square::fromIndex(back_left_index).value();
            if(!back_left_pawn.has_value()) { //Capture possible
                moves.push_back(Move(current_square, back_left_square));
            } else if (back_left_pawn.value() != current_turn) {
                moves.push_back(Move(current_square, back_left_square));
            }
        }

        if(square_color[back_right_index] != knight_square_color) {
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

    if(!isOutOfRange(left_front_index)) {
        if(square_color[left_front_index] != knight_square_color) {
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
        if(square_color[left_back_index] != knight_square_color) {
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
        if(square_color[right_front_index] != knight_square_color) {
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
        if(square_color[right_back_index] != knight_square_color) {
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
    bool start_color = square_color[bishop_index];

    while(!collided && !isOutOfRange(work_front_left_index) && square_color[work_front_left_index] == start_color) {
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

    while(!collided && !isOutOfRange(work_front_right_index) && square_color[work_front_right_index] == start_color) {
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

    while(!collided && !isOutOfRange(work_back_right_index) && square_color[work_back_right_index] == start_color) {
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

    while(!collided && !isOutOfRange(work_back_left_index) && square_color[work_back_left_index] == start_color) {
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

std::optional<PieceColor> Board::checkOccupation(Square::Index index) const {
    //CAREFUL!: no index bound checking is done, so an empty return value could also mean it is out of bounds (not so safe access also, performance reasons)
    if(colorPositions.white[index]) return PieceColor::White;
    else if(colorPositions.black[index]) return PieceColor::Black;
    else return std::nullopt;
}

Square::Index Board::frontIndex(Square::Index from, std::optional<PieceColor> turn) const {
    PieceColor turn_to_use = current_turn;
    if(turn.has_value()) turn_to_use = turn.value();
    switch (turn_to_use) {
        case PieceColor::White :
            return from + 8;
        case PieceColor::Black :
            return from - 8;
        default :
            return 64; //Never occurs
    }
}

Square::Index Board::backIndex(Square::Index from, std::optional<PieceColor> turn) const {
    PieceColor turn_to_use = current_turn;
    if(turn.has_value()) turn_to_use = turn.value();
    switch (turn_to_use) {
        case PieceColor::White :
            return from - 8;
        case PieceColor::Black :
            return from + 8;
        default :
            return 64; //Never occurs
    }
}

Square::Index Board::leftIndex(Square::Index from, std::optional<PieceColor> turn) const {
    PieceColor turn_to_use = current_turn;
    if(turn.has_value()) turn_to_use = turn.value();
    switch (turn_to_use) {
        case PieceColor::White :
            return from - 1;
        case PieceColor::Black :
            return from + 1;
        default :
            return 64; //Never occurs
    }
}

Square::Index Board::rightIndex(Square::Index from, std::optional<PieceColor> turn) const {
    PieceColor turn_to_use = current_turn;
    if(turn.has_value()) turn_to_use = turn.value();
    switch (turn_to_use) {
        case PieceColor::White :
            return from + 1;
        case PieceColor::Black :
            return from - 1;
        default :
            return 64; //Never occurs
    }
}

Square::Index Board::doublePushIndex(Square::Index from, std::optional<PieceColor> turn) const {
    PieceColor turn_to_use = current_turn;
    if(turn.has_value()) turn_to_use = turn.value();
    switch (turn_to_use) {
        case PieceColor::White :
            return from + 16;
        case PieceColor::Black :
            return from - 16;
        default :
            return 64; //Never occurs
    }
}

Square::Index Board::frontLeftIndex(Square::Index from, std::optional<PieceColor> turn) const {
    PieceColor turn_to_use = current_turn;
    if(turn.has_value()) turn_to_use = turn.value();
    switch (turn_to_use) {
        case PieceColor::White :
            return from + 8 - 1;
        case PieceColor::Black :
            return from - 8 + 1;
        default :
            return 64; //Never occurs
    }
}

Square::Index Board::backLeftIndex(Square::Index from, std::optional<PieceColor> turn) const {
    PieceColor turn_to_use = current_turn;
    if(turn.has_value()) turn_to_use = turn.value();
    switch (turn_to_use) {
        case PieceColor::White :
            return from - 8 - 1;
        case PieceColor::Black :
            return from + 8 + 1;
        default :
            return 64; //Never occurs
    }
}

Square::Index Board::backRightIndex(Square::Index from, std::optional<PieceColor> turn) const {
    PieceColor turn_to_use = current_turn;
    if(turn.has_value()) turn_to_use = turn.value();
    switch (turn_to_use) {
        case PieceColor::White :
            return from - 8 + 1;
        case PieceColor::Black :
            return from + 8 - 1;
        default :
            return 64; //Never occurs
    }
}

Square::Index Board::frontRightIndex(Square::Index from, std::optional<PieceColor> turn) const {
    PieceColor turn_to_use = current_turn;
    if(turn.has_value()) turn_to_use = turn.value();
    switch (turn_to_use) {
        case PieceColor::White :
            return from + 8 + 1;
        case PieceColor::Black :
            return from - 8 - 1;
        default :
            return 64; //Never occurs
    }
}

bool Board::firstRankCheck(Square::Index index, std::optional<PieceColor> turn) const {
    PieceColor turn_to_use = current_turn;
    if(turn.has_value()) turn_to_use = turn.value();
    switch (turn_to_use) {
        case PieceColor::White :
            return (index <= 7);
        case PieceColor::Black :
            return (index >= 56 && index <= 63);
        default : //Never reached
            return false;
    }
}

bool Board::lastRankCheck(Square::Index index, std::optional<PieceColor> turn) const {
    PieceColor turn_to_use = current_turn;
    if(turn.has_value()) turn_to_use = turn.value();
    switch (turn_to_use) {
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


std::ostream& operator<<(std::ostream& os, const Board& board) {
    (void)board;
    return os;
}
