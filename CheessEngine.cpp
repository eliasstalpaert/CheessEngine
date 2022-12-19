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

constexpr PrincipalVariation::Score CHECKMATE_SCORE = 100000;
constexpr PrincipalVariation::Score SCORE_THRESHOLD = 50;

constexpr unsigned ESTIMATED_MOVES = 30; //Fixed

PrincipalVariation CheessEngine::pv(const Board &board, const TimeInfo::Optional &timeInfo) {
    //Iterative deepening with depth-first negamax search
    if(timeInfo.has_value()) {
        TimeOptional start_time = std::chrono::steady_clock::now();
        Duration max_duration(0);
        switch(board.turn()) {
            case PieceColor::White :
                max_duration = timeInfo->white.timeLeft / ESTIMATED_MOVES;
                break;
            case PieceColor::Black :
                max_duration = timeInfo->black.timeLeft / ESTIMATED_MOVES;
                break;
        }
        std::optional<Result> current_result = std::nullopt;

        signed search_depth = 0;
        while(true) {
            auto negamax_result = negamaxSearch(board, search_depth, CHECKMATE_SCORE - 50000, CHECKMATE_SCORE, 1, start_time, max_duration);
            PrincipalVariation::Score score = std::get<1>(negamax_result);

            //check deadline expiration
            if(std::chrono::steady_clock::now() - start_time.value() > max_duration) break;

            //Check for checkmate
            if(abs(score) == CHECKMATE_SCORE) {
                std::reverse(std::get<0>(negamax_result).begin(), std::get<0>(negamax_result).end());
                return PrincipalVariation(std::move(std::get<0>(negamax_result)), search_depth, true);
            }

            current_result = negamax_result;

            search_depth++;
        }

        if(!current_result.has_value()) current_result = Result(PrincipalVariation::MoveVec(), 0); //avoid std::optional exception

        std::reverse(std::get<0>(current_result.value()).begin(), std::get<0>(current_result.value()).end());
        return PrincipalVariation(std::move(std::get<0>(current_result.value())), std::get<1>(current_result.value()), false);

    } else {
        //No time control, just to depth of 5
        Result negamax_result;
        for(int i = 0; i < 6; i++) {
            negamax_result = negamaxSearch(board, i, CHECKMATE_SCORE - 50000, CHECKMATE_SCORE, 1, std::nullopt, Duration(0));
            if(abs(std::get<1>(negamax_result)) == CHECKMATE_SCORE) {
                std::reverse(std::get<0>(negamax_result).begin(), std::get<0>(negamax_result).end());
                return PrincipalVariation(std::move(std::get<0>(negamax_result)), i, true);
            }
        }

        std::reverse(std::get<0>(negamax_result).begin(), std::get<0>(negamax_result).end());
        return PrincipalVariation(std::move(std::get<0>(negamax_result)), std::get<1>(negamax_result), false);
    }
}

CheessEngine::Result CheessEngine::negamaxSearch(const Board &board, unsigned depth, PrincipalVariation::Score alpha, PrincipalVariation::Score beta, int turn, const TimeOptional& start_time, const Duration& max_duration) const {

    //Generate moves, if no legal moves, check for stalemate/checkmate and assign score
    Board::MoveVec possible_moves = generateLegalMoves(board);

    //No legal moves, checkmate or stalemate
    if(possible_moves.empty()) {
        if(board.isPlayerChecked(board.turn())) return std::make_tuple(PrincipalVariation::MoveVec(),-100000); //checkmate
        else return std::make_tuple(PrincipalVariation::MoveVec(),0); //stalemate
    }

    if(depth == 0) return std::make_tuple(PrincipalVariation::MoveVec(), evalPosition(board)); //Return negamax score from current player's viewpoint

    //TODO: order moves
    std::optional<Move> best_move = std::nullopt;
    PrincipalVariation::MoveVec best_pv;

    for(size_t move_ind = 0; move_ind < possible_moves.size(); move_ind++){
        Board copy_board(board);
        Move& current_move = possible_moves[move_ind];

        copy_board.makeMove(current_move);

        auto opponent_score = negamaxSearch(copy_board, depth - 1, -beta, -alpha, -turn, start_time, max_duration);
        PrincipalVariation::Score new_score = -1 * std::get<1>(opponent_score);

        if(new_score > alpha) {
            alpha = new_score;
            best_move = current_move; //Remember potential best move belonging to new_score
            best_pv = PrincipalVariation::MoveVec(std::get<0>(opponent_score)); //Remember pv that led to the score
        }

        //Deadline reached, still returns a solution from the requested depth tho, so there will be some delay in returning from deadline expiration
        if(start_time.has_value() && std::chrono::steady_clock::now() - start_time.value() >= max_duration) {
            if(!best_move.has_value()) best_move = current_move; //Never occurs I think
            break;
        }

        if(alpha >= beta) break; //other moves shouldn't be considered (fail-hard beta cutoff)
    }
    if(best_move.has_value()) best_pv.push_back(best_move.value());
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

//Fifty move draw using current value of halfmove counter
//Threefold repetition check
//TODO: Forcing a draw with special rules

PrincipalVariation::Score CheessEngine::evalPosition(const Board &board) const {
    PrincipalVariation::Score score = 0;
    score += getMaterialScore(board);
    score += getSpaceScore(board);
    return score;
}

PrincipalVariation::Score CheessEngine::getMaterialScore(const Board& board) const {
    PrincipalVariation::Score pos_score = 0;
    for(const auto& piece : piece_value) {
        pos_score += piece.second * board.getAmountOfPiece(board.turn(), piece.first);
        pos_score -= piece.second * board.getAmountOfPiece(!board.turn(), piece.first);
    }
    return pos_score;
}

PrincipalVariation::Score CheessEngine::getSpaceScore(const Board &board) const {

    std::bitset<64> black_mask = 4294967295;
    std::bitset<64> white_mask(black_mask);
    white_mask.flip();
    auto positions = board.getColorPositions(board.turn());
    auto opponent_positions = board.getColorPositions(!board.turn());

    std::bitset<64> center_mask = 103481868288;

    PrincipalVariation::Score center_score = ((positions & center_mask).count() * square_value * 5) - ((opponent_positions & center_mask).count() * square_value * 5);

    switch (board.turn()) {
        case PieceColor::White :
            positions &= white_mask;
            opponent_positions &= black_mask;
            break;
        case PieceColor::Black :
            positions &= black_mask;
            opponent_positions &= white_mask;
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
    return Engine::hashInfo();
}

void CheessEngine::setHashSize(std::size_t size) {
    //Only relevant if transposition tables are used
    Engine::setHashSize(size);
}

