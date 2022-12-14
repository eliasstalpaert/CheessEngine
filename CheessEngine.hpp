//
// Created by eliass on 13/12/22.
//

#ifndef CPLCHESS_CHEESSENGINE_HPP
#define CPLCHESS_CHEESSENGINE_HPP

#include <memory>
#include "Engine.hpp"
#include "Board.hpp"

class CheessEngine : public Engine {
public:
    CheessEngine();

    ~CheessEngine() override = default;

    std::string name() const override;

    std::string version() const override;

    std::string author() const override;

    void newGame() override;

    PrincipalVariation pv(const Board &board, const TimeInfo::Optional &timeInfo) override;

    std::optional<HashInfo> hashInfo() const override;

    void setHashSize(std::size_t size) override;

private:

    //Fifty-move repetition rule
    unsigned halfmove_counter;

    //Hash-table linking boards to their repetition for three-fold repetition

    std::tuple<Move, int> negamaxSearch(const Board::MoveVec& opponent_prev, const Board &board, unsigned int depth, int alpha, int beta, int turn) const;

    std::unique_ptr<Board::MoveVec> generateLegalMoves(const std::vector<Move>& opponent_prev, const Board &board) const;
};


#endif //CPLCHESS_CHEESSENGINE_HPP
