#ifndef CHESS_ENGINE_BOARD_HPP
#define CHESS_ENGINE_BOARD_HPP

#include "Piece.hpp"
#include "Square.hpp"
#include "Move.hpp"
#include "CastlingRights.hpp"

#include <optional>
#include <iosfwd>
#include <vector>
#include <bitset>


//bitset: Default ctor bitset sets all bits to 0

//All black squares on the board to aid calculations
constexpr std::bitset<64> square_color = 0xAA55AA55AA55AA55;

//Positions of the chess pieces independent of color
struct PiecePositions {
    std::bitset<64> pawns;
    std::bitset<64> knights;
    std::bitset<64> bishops;
    std::bitset<64> rooks;
    std::bitset<64> queen;
    std::bitset<64> king;


    void clearBit(Square::Index index) {
        pawns[index] = 0;
        knights[index] = 0;
        bishops[index] = 0;
        rooks[index] = 0;
        queen[index] = 0;
        king[index] = 0;
    }
};

template<>
struct std::hash<PiecePositions>
{
    std::size_t operator()(const PiecePositions& pieces) const {
        return (std::hash<std::bitset<64>>{}(pieces.pawns) + std::hash<std::bitset<64>>{}(pieces.knights) + std::hash<std::bitset<64>>{}(pieces.bishops) + std::hash<std::bitset<64>>{}(pieces.rooks) + std::hash<std::bitset<64>>{}(pieces.queen) + std::hash<std::bitset<64>>{}(pieces.king));
    }
};


struct ColorPositions {
    std::bitset<64> white;
    std::bitset<64> black;
};

template<>
struct std::hash<ColorPositions>
{
    std::size_t operator()(const ColorPositions& color) const {
        return (std::hash<std::bitset<64>>{}(color.white) + std::hash<std::bitset<64>>{}(color.black));
    }
};


struct Repetition {
    PiecePositions piece_positions;
    CastlingRights castling_rights;
    Square::Optional en_passant_square;
    PieceColor turn;
};

template<>
struct std::hash<Repetition>
{
    std::size_t operator()(const Repetition& rep) const {
        return (std::hash<PiecePositions>{}(rep.piece_positions) + std::hash<CastlingRights>{}(rep.castling_rights) + std::hash<Square::Optional>{}(rep.en_passant_square) + std::hash<PieceColor>{}(rep.turn));
    }
};

class Board {
public:

    using Optional = std::optional<Board>;
    using MoveVec = std::vector<Move>;

    void setPiece(const Square& square, const Piece::Optional& piece);
    Piece::Optional piece(const Square& square) const;
    void setTurn(PieceColor turn);
    PieceColor turn() const;
    void setCastlingRights(CastlingRights cr);
    CastlingRights castlingRights() const;
    void setEnPassantSquare(const Square::Optional& square);
    Square::Optional enPassantSquare() const;
    void setHalfMoveCounter(int count);
    signed halfMoveCounter() const;
    PiecePositions piecePositions() const;
    ColorPositions colorPositions() const;

    std::bitset<64> getColorPositions(PieceColor turn) const;
    Repetition getRepetition() const;

    bool isSquareAttacked(PieceColor turn, Square::Index index) const;
    bool isPlayerChecked(PieceColor turn) const;

    unsigned getAmountOfPiece(PieceColor color, PieceType piece_type) const;

    void makeMove(const Move& move);

    void pseudoLegalMoves(MoveVec& moves) const;
    void pseudoLegalMovesFrom(const Square& from, MoveVec& moves) const;

private:

    PiecePositions piece_positions;

    ColorPositions color_positions;

    PieceColor current_turn;

    CastlingRights castling_rights;

    Square::Optional en_passant_square;

    int halfmove_counter; //signed because of std::stoi

    Square::Index frontIndex(Square::Index from, std::optional<PieceColor> turn = std::nullopt) const;
    Square::Index backIndex(Square::Index from, std::optional<PieceColor> turn = std::nullopt) const;
    Square::Index leftIndex(Square::Index from, std::optional<PieceColor> turn = std::nullopt) const;
    Square::Index rightIndex(Square::Index from, std::optional<PieceColor> turn = std::nullopt) const;
    Square::Index backLeftIndex(Square::Index from, std::optional<PieceColor> turn = std::nullopt) const;
    Square::Index backRightIndex(Square::Index from, std::optional<PieceColor> turn = std::nullopt) const;
    Square::Index frontLeftIndex(Square::Index from, std::optional<PieceColor> turn = std::nullopt) const;
    Square::Index frontRightIndex(Square::Index from, std::optional<PieceColor> turn = std::nullopt) const;

    bool promotionCandidate(Square::Index index) const;

    bool doublePushCandidate(Square::Index index) const;
    Square::Index doublePushIndex(Square::Index from, std::optional<PieceColor> turn = std::nullopt)  const;

    bool firstRankCheck(Square::Index index, std::optional<PieceColor> turn = std::nullopt)  const;
    bool lastRankCheck(Square::Index index, std::optional<PieceColor> turn = std::nullopt)  const;
    bool isOutOfRange(Square::Index index) const;



    void pseudoLegalPawnMovesFrom(Square::Index index, Board::MoveVec& moves) const;
    void pseudoLegalKingMovesFrom(Square::Index index, Board::MoveVec& moves) const;
    void pseudoLegalKnightMovesFrom(Square::Index index, Board::MoveVec& moves) const;
    void pseudoLegalRookMovesFrom(Square::Index index, Board::MoveVec& moves) const;
    void pseudoLegalBishopMovesFrom(Square::Index index, Board::MoveVec& moves) const;
    void pseudoLegalQueenMovesFrom(Square::Index index, Board::MoveVec& moves) const;


    std::optional<PieceType> clearCapturePiece(const Square& square, bool capture);

    std::optional<PieceColor> checkOccupation(Square::Index index) const;
};

template<>
struct std::hash<Board>
{
    std::size_t operator()(const Board& board) const {
        return (std::hash<PiecePositions>{}(board.piecePositions()) +
        std::hash<ColorPositions>{}(board.colorPositions()) +
        std::hash<PieceColor>{}(board.turn()) +
        std::hash<CastlingRights>{}(board.castlingRights()) +
        std::hash<Square::Optional>{}(board.enPassantSquare()) +
        std::hash<int>{}(board.halfMoveCounter()));
    }
};


bool operator==(const Board &b1, const Board& b2);
bool operator==(const Repetition &r1, const Repetition &r2);
bool operator==(const PiecePositions &p1, const PiecePositions &p2);
bool operator==(const ColorPositions &c1, const PiecePositions &c2);




std::ostream& operator<<(std::ostream& os, const Board& board);

#endif
