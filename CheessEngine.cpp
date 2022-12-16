//
// Created by eliass on 13/12/22.
//

#include "CheessEngine.hpp"
#include <tuple>
#include <memory>
#include <map>
#include <algorithm>

CheessEngine::CheessEngine() : halfmove_counter{0}{

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
}

/*****************
 *
 * MOVE SEARCHING
 *
 * ******************/

unsigned debug_depth;

PrincipalVariation CheessEngine::pv(const Board &board, const TimeInfo::Optional &timeInfo) {
    //Compute PV of given board


    //Iterative deepening with depth-first negamax search

    std::tuple<PrincipalVariation::MoveVec ,int32_t> negamax_result;
    for(int i = 1; i < 6; i++) {
        debug_depth = i;
        negamax_result = negamaxSearch(board, i, -150000, 100000, 1);
        if(std::get<1>(negamax_result) == 100000) {
            std::reverse(std::get<0>(negamax_result).begin(), std::get<0>(negamax_result).end());
            return PrincipalVariation(std::move(std::get<0>(negamax_result)), std::get<1>(negamax_result));
        }
    }
    timeInfo.has_value();
    //TODO: Time control

    //Relative part of the time and check how much score has improved if winning, if stalemate or losing, think further

    //Compute for each legal move the negamax value
    //If no legal moves, checkmate/stalemate
    std::reverse(std::get<0>(negamax_result).begin(), std::get<0>(negamax_result).end());
    return PrincipalVariation(std::move(std::get<0>(negamax_result)), std::get<1>(negamax_result));
}

std::tuple<PrincipalVariation::MoveVec ,int32_t> CheessEngine::negamaxSearch(const Board &board, unsigned int depth, int32_t alpha, int32_t beta, int turn) const {

    if(depth == 0) {
        //Generate moves, if no legal moves, check for stalemate/checkmate and assign score
        Board::MoveVec possible_moves = generateLegalMoves(board);

        //No legal moves, checkmate or stalemate
        if(possible_moves.empty()) {
            if(board.isPlayerChecked(board.turn())) return std::make_tuple(PrincipalVariation::MoveVec(),-100000); //checkmate
            else return std::make_tuple(PrincipalVariation::MoveVec(),0); //stalemate
        }
        else return std::make_tuple(PrincipalVariation::MoveVec(), evalPosition(board)); //Return negamax score from current player's viewpoint
    }

    //Generate moves, if no legal moves, check for stalemate/checkmate and assign score
    Board::MoveVec possible_moves = generateLegalMoves(board);

    //No legal moves, checkmate or stalemate
    if(possible_moves.empty()) {
        if(board.isPlayerChecked(board.turn())) return std::make_tuple(PrincipalVariation::MoveVec(),-100000); //checkmate
        else return std::make_tuple(PrincipalVariation::MoveVec(),0); //stalemate
    }

    //TODO: order moves
    //Initialize
    Move best_move;
    bool new_pv_move = false;
    PrincipalVariation::MoveVec best_pv;


    for(size_t move_ind = 0; move_ind < possible_moves.size(); move_ind++){
        //create copy of board
        Board copy_board(board);
        Move& current_move = possible_moves[move_ind];
/*
        if(depth == 5 && current_move.from().index() == 18) {
            Move kakamove(current_move);
            kakamove.from();
        }
*/

        if(depth == debug_depth && debug_depth == 2 && current_move.from().index() == 54) {
            Move kakamove(current_move);
            kakamove.from();
        }

        copy_board.makeMove(current_move);
        copy_board.setTurn(!board.turn());


        auto opponent_score = negamaxSearch(copy_board, depth - 1, -beta, -alpha, -turn);
        int32_t new_score = -1 * std::get<1>(opponent_score);

        if(new_score > alpha) {
            alpha = new_score;
            new_pv_move = true;
            best_move = current_move; //Remember potential best move belonging to new_score
            best_pv = PrincipalVariation::MoveVec(std::get<0>(opponent_score)); //Remember pv that led to the score
        }

        if(alpha >= beta) break; //other moves shouldn't be considered (fail-hard beta cutoff)
        //TODO: Time control: also break here if time is up!
    }
    if(new_pv_move) best_pv.push_back(best_move);
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

//Results in mate or takes king not allowed

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
    //Check pinned pieces and eliminate them from the moves (pinned status should be removed after resolving checkmate)
    //When checkmate, detect attacking (xray attacks) of attacking piece (for potential blockers)
    //Then check attacked squares by other pieces to make sure king doesn't move into attacking territory
    return moves;
}
//If current player moves king, check for checkmate/stalemate in moves generated by opponent (defines the attacked square)

//If no valid moves (every move leaves king in check, meaning no legal moves anymore)
//and the opponent can take the king (current player was already checked), opponent wins otherwise stalemate

/****************
 *
 * BOARD EVALUATION
 *
 * ******************/

const std::map<PieceType, int32_t> piece_value { //Shannon point values
        {PieceType::Pawn, 100},
        {PieceType::Knight, 300},
        {PieceType::Bishop, 300},
        {PieceType::Rook, 500},
        {PieceType::Queen, 900}
};

//Static scores
//Fifty move draw using current value of halfmove counter
//Threefold repetition check
//TODO: Forcing a draw with special rules

int32_t CheessEngine::evalPosition(const Board &board) const {
    int32_t score = 0;
    score += getMaterialScore(board);
    return score;
}

int32_t CheessEngine::getMaterialScore(const Board& board) const {
    int32_t pos_score = 0;
    for(const auto& piece : piece_value) {
        pos_score += piece.second * board.getAmountOfPiece(board.turn(), piece.first);
        pos_score -= piece.second * board.getAmountOfPiece(!board.turn(), piece.first);
    }
    return pos_score;
}

/**************
 *
 * TRANSPOSITION TABLES
 *
 * *****************/

std::optional<HashInfo> CheessEngine::hashInfo() const {
    //Only relevant if transposition tables are used
    return Engine::hashInfo();
}

void CheessEngine::setHashSize(std::size_t size) {
    //Only relevant if transposition tables are used
    Engine::setHashSize(size);
}

