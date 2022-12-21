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

    using SearchResult = std::tuple<PrincipalVariation::MoveVec ,PrincipalVariation::Score>;

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

    //TODO: Hash-table linking boards to their repetition for three-fold repetition

    SearchResult negamaxSearch(const Board &board, unsigned depth, PrincipalVariation::Score alpha, PrincipalVariation::Score beta, int turn) const;

    Board::MoveVec generateLegalMoves(const Board &board) const;

    PrincipalVariation::Score evalPosition(const Board &board) const;

    PrincipalVariation::Score getMaterialScore(const Board& board) const;

    PrincipalVariation::Score getSpaceScore(const Board& board) const;
};


#endif //CPLCHESS_CHEESSENGINE_HPP
