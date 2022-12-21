//
// Created by eliass on 13/12/22.
//

#ifndef CPLCHESS_CHEESSENGINE_HPP
#define CPLCHESS_CHEESSENGINE_HPP

#include <memory>
#include "Engine.hpp"
#include "Board.hpp"
#include <unordered_map>

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

    //unsigned values are initialized to zero if key doesn't exist yet (by definition)
    std::unordered_map<Repetition, unsigned, std::hash<Repetition>> repetition_map; //Possible error due to comparison being different than hashed variables?

    //transposition table keeping best move of previous iterations
    std::unordered_map<Board, Move, std::hash<Board>> transposition_table;

    size_t max_transpo_size;

    SearchResult negamaxSearch(const Board &board, unsigned depth, PrincipalVariation::Score alpha, PrincipalVariation::Score beta, int turn);

    Board::MoveVec generateLegalMoves(const Board &board) const;

    PrincipalVariation::Score evalPosition(const Board &board) const;

    PrincipalVariation::Score getMaterialScore(const Board& board) const;

    PrincipalVariation::Score getSpaceScore(const Board& board) const;
};


#endif //CPLCHESS_CHEESSENGINE_HPP
