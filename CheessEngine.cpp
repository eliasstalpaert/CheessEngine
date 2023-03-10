//
// Created by eliass on 13/12/22.
//

#include "CheessEngine.hpp"
#include <tuple>
#include <map>
#include <algorithm>

CheessEngine::CheessEngine() : max_transpo_size{50000000} { //Estimated based on 2GB / 40 bytes per entry

}


std::string CheessEngine::name() const {
    return "Cheess Engine";
}

std::string CheessEngine::version() const {
    return "1";
}

std::string CheessEngine::author() const {
    return "Elias Stalpaert";
}

void CheessEngine::newGame() {
  //Reset state of the engine
  repetition_map.clear();
  transposition_table.clear();
}

/*****************
 *
 * MOVE SEARCHING
 *
 * ******************/

PrincipalVariation CheessEngine::pv(const Board &board, const TimeInfo::Optional &timeInfo) {
    timeInfo.has_value(); //Time control currently not implemented

    //Iterative deepening of fixed depth of 5
    SearchResult negamax_result;
    for(int i = 0; i < 6; i++) {
        negamax_result = negamaxSearch(board, i, -150000, 100000, 1);
        if(abs(std::get<1>(negamax_result)) == 100000) {
            std::reverse(std::get<0>(negamax_result).begin(), std::get<0>(negamax_result).end());
            return PrincipalVariation(std::move(std::get<0>(negamax_result)), i, true);
        }
    }

    //Search until no longer losing
    if(std::get<1>(negamax_result) < 0) {
        int i = 6;
        while(true) {
            negamax_result = negamaxSearch(board, i, -150000, 100000, 1);
            if(std::get<1>(negamax_result) >= 0) break; //can maybe cause unnecessary draws
            else i++;
        }
    }

    std::reverse(std::get<0>(negamax_result).begin(), std::get<0>(negamax_result).end());
    return PrincipalVariation(std::move(std::get<0>(negamax_result)), std::get<1>(negamax_result), false);
}

CheessEngine::SearchResult CheessEngine::negamaxSearch(const Board &board, unsigned depth, PrincipalVariation::Score alpha, PrincipalVariation::Score beta, int turn) {

    //Generate moves, if no legal moves, check for stalemate/checkmate and assign score
    Board::MoveVec possible_moves = generateLegalMoves(board);

    //No legal moves, checkmate or stalemate
    if(possible_moves.empty()) {
        if(board.isPlayerChecked(board.turn())) return std::make_tuple(PrincipalVariation::MoveVec(),-100000); //checkmate
        else return std::make_tuple(PrincipalVariation::MoveVec(),0); //stalemate
    }

    if(depth == 0) return std::make_tuple(PrincipalVariation::MoveVec(), evalPosition(board)); //Return negamax score from current player's viewpoint



    std::optional<Move> best_move = std::nullopt;
    PrincipalVariation::MoveVec best_pv;

    //Check for previous best move and put it as first element
    if(transposition_table.contains(board)) {
        Move best_prev = transposition_table[board];
        for(auto iter = possible_moves.begin(); iter != possible_moves.end(); iter++) {
            if(*iter == best_prev) {
                possible_moves.erase(iter);
                break;
            }
        }
        possible_moves.push_back(best_prev);
        std::reverse(possible_moves.begin(), possible_moves.end());
    }


    for(const Move& current_move : possible_moves){
        Board copy_board(board);
        //MAKE MOVE
        copy_board.makeMove(current_move);


        Repetition rep = copy_board.getRepetition();
        repetition_map[rep]++; //inserts a new element initialized to 0 if key doesn't exist

        auto opponent_score = negamaxSearch(copy_board, depth - 1, -beta, -alpha, -turn);
        PrincipalVariation::Score new_score = -1 * std::get<1>(opponent_score);

        if(new_score < 0 && (copy_board.halfMoveCounter() >= 100 || repetition_map.at(rep) >= 3)) new_score = 0; //Claim draw if not winning using draw conditions

        if(new_score > alpha) {
            alpha = new_score;
            best_move = current_move; //Remember potential best move belonging to new_score
            best_pv = PrincipalVariation::MoveVec(std::get<0>(opponent_score)); //Remember pv that led to the score
        }

        //UNMAKE MOVE
        repetition_map[rep]--;

        if(alpha >= beta) break; //other moves shouldn't be considered (fail-hard beta cutoff)
    }
    if(best_move.has_value()) {
        if(transposition_table.contains(board)) {
            transposition_table[board] = best_move.value();
        }
        else if(transposition_table.size() < max_transpo_size) {
            try {
                transposition_table[board] = best_move.value();
            } catch(const std::exception& exception) {
                //Exceptions could be thrown due to limited memory issues if my size constraint fails
                //Documentation: "If an exception is thrown by any operation, the insertion has no effect" (cppreference)

            }
        }
        best_pv.push_back(best_move.value());
    }
    return std::make_tuple(best_pv, alpha);
}

/**************
 *
 * MOVE ORDERING
 *
 * **************/

/*************
 *
 * LEGAL MOVE GENERATION (uses isSquareAttacked and MakeMove)
 *
 * ****************/

Board::MoveVec CheessEngine::generateLegalMoves(const Board &board) const {
    Board::MoveVec moves;
    board.pseudoLegalMoves(moves);

    auto it = moves.begin();

    while(it != moves.end()) {
        Board copy_board(board);
        copy_board.makeMove(*it);
        PieceColor current_turn = board.turn();
        if(copy_board.isPlayerChecked(current_turn)){
            it = moves.erase(it);
        }
        else it++;
    }
    return moves;
}

/****************
 *
 * BOARD EVALUATION
 *
 * ******************/

const std::map<PieceType, PrincipalVariation::Score> piece_value { //Shannon point values
        {PieceType::Pawn, 100},
        {PieceType::Knight, 300},
        {PieceType::Bishop, 300},
        {PieceType::Rook, 500},
        {PieceType::Queen, 900}
};


const PrincipalVariation::Score square_value = 10;

PrincipalVariation::Score CheessEngine::evalPosition(const Board &board) const {
    PrincipalVariation::Score score = 0;
    score += getMaterialScore(board);
    score += getSpaceScore(board);
    return score;
}

PrincipalVariation::Score CheessEngine::getMaterialScore(const Board& board) const {
    PrincipalVariation::Score pos_score = 0;
    for(const auto& piece : piece_value) {
        //Current turn's pieces
        pos_score += piece.second * board.getAmountOfPiece(board.turn(), piece.first);
        //Current opponent's pieces
        pos_score -= piece.second * board.getAmountOfPiece(!board.turn(), piece.first);
    }
    return pos_score;
}

PrincipalVariation::Score CheessEngine::getSpaceScore(const Board &board) const {

    std::bitset<64> white_half = 4294967295;
    std::bitset<64> black_half(white_half);
    black_half.flip();

    auto positions = board.getColorPositions(board.turn());
    auto opponent_positions = board.getColorPositions(!board.turn());

    std::bitset<64> center_mask = 103481868288; //D4/E4/D5/E5

    PrincipalVariation::Score center_score = ((positions & center_mask).count() * square_value * 5) - ((opponent_positions & center_mask).count() * square_value * 5);

    switch (board.turn()) {
        case PieceColor::White :
            positions &= black_half;
            opponent_positions &= white_half;
            break;
        case PieceColor::Black :
            positions &= white_half;
            opponent_positions &= black_half;
            break;
    }

    PrincipalVariation::Score occupation_score = (positions.count() * square_value) - (opponent_positions.count() * square_value);

    return center_score + occupation_score;
}

/**************
 *
 * TRANSPOSITION TABLES
 *
 * *****************/

std::optional<HashInfo> CheessEngine::hashInfo() const {
    //Only relevant if transposition tables are used
    HashInfo hash_info;
    hash_info.defaultSize = 2000000000; //2GB
    hash_info.maxSize = 2000000000;
    hash_info.minSize = 128000000; //128MB
    return hash_info;
}

void CheessEngine::setHashSize(std::size_t size) {
    //Only relevant if transposition tables are used
    max_transpo_size = size / 40;
}

