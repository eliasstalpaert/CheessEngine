#include "Square.hpp"

#include <ostream>

Square::Square(Index index) : square_file{index % 8}, square_rank{index / 8}, square_index{index}
{

}

Square::Optional Square::fromCoordinates(Coordinate file, Coordinate rank) {
    Index calc_index = rank*8 + file;
    if(calc_index < 0 || calc_index > 63) return std::nullopt;
    else return Square(calc_index);
}

Square::Optional Square::fromIndex(Index index) {
    if(index < 0 || index > 63) return std::nullopt;
    else return Square(index);
}

Square::Optional Square::fromName(const std::string& name) {
    if(name.length() != 2) return std::nullopt; //invalid name length
    //if(name[0] < 97 || name[0] > 104) return std::nullopt; //invalid ASCII file character
    //if(name[1] < 49 || name[1] > 56) return std::nullopt; //invalid ASCII rank character

    Index calc_index = (static_cast<Index>(name[0]) - 97) + ((static_cast<Index>(name[1]) - 49) * 8);

    return fromIndex(calc_index);
}

Square::Coordinate Square::file() const {
    return square_file;
}

Square::Coordinate Square::rank() const {
    return square_rank;
}

Square::Index Square::index() const {
    return square_index;
}


const Square Square::A1 = Square( 0 + 0);
const Square Square::B1 = Square( 0 + 1);
const Square Square::C1 = Square( 0 + 2);
const Square Square::D1 = Square( 0 + 3);
const Square Square::E1 = Square( 0 + 4);
const Square Square::F1 = Square( 0 + 5);
const Square Square::G1 = Square( 0 + 6);
const Square Square::H1 = Square( 0 + 7);

const Square Square::A2 = Square( 8 + 0);
const Square Square::B2 = Square( 8 + 1);
const Square Square::C2 = Square( 8 + 2);
const Square Square::D2 = Square( 8 + 3);
const Square Square::E2 = Square( 8 + 4);
const Square Square::F2 = Square( 8 + 5);
const Square Square::G2 = Square( 8 + 6);
const Square Square::H2 = Square( 8 + 7);

const Square Square::A3 = Square(16 + 0);
const Square Square::B3 = Square(16 + 1);
const Square Square::C3 = Square(16 + 2);
const Square Square::D3 = Square(16 + 3);
const Square Square::E3 = Square(16 + 4);
const Square Square::F3 = Square(16 + 5);
const Square Square::G3 = Square(16 + 6);
const Square Square::H3 = Square(16 + 7);

const Square Square::A4 = Square(24 + 0);
const Square Square::B4 = Square(24 + 1);
const Square Square::C4 = Square(24 + 2);
const Square Square::D4 = Square(24 + 3);
const Square Square::E4 = Square(24 + 4);
const Square Square::F4 = Square(24 + 5);
const Square Square::G4 = Square(24 + 6);
const Square Square::H4 = Square(24 + 7);

const Square Square::A5 = Square(32 + 0);
const Square Square::B5 = Square(32 + 1);
const Square Square::C5 = Square(32 + 2);
const Square Square::D5 = Square(32 + 3);
const Square Square::E5 = Square(32 + 4);
const Square Square::F5 = Square(32 + 5);
const Square Square::G5 = Square(32 + 6);
const Square Square::H5 = Square(32 + 7);

const Square Square::A6 = Square(40 + 0);
const Square Square::B6 = Square(40 + 1);
const Square Square::C6 = Square(40 + 2);
const Square Square::D6 = Square(40 + 3);
const Square Square::E6 = Square(40 + 4);
const Square Square::F6 = Square(40 + 5);
const Square Square::G6 = Square(40 + 6);
const Square Square::H6 = Square(40 + 7);

const Square Square::A7 = Square(48 + 0);
const Square Square::B7 = Square(48 + 1);
const Square Square::C7 = Square(48 + 2);
const Square Square::D7 = Square(48 + 3);
const Square Square::E7 = Square(48 + 4);
const Square Square::F7 = Square(48 + 5);
const Square Square::G7 = Square(48 + 6);
const Square Square::H7 = Square(48 + 7);

const Square Square::A8 = Square(56 + 0);
const Square Square::B8 = Square(56 + 1);
const Square Square::C8 = Square(56 + 2);
const Square Square::D8 = Square(56 + 3);
const Square Square::E8 = Square(56 + 4);
const Square Square::F8 = Square(56 + 5);
const Square Square::G8 = Square(56 + 6);
const Square Square::H8 = Square(56 + 7);

std::ostream& operator<<(std::ostream& os, const Square& square) {
    os << char(square.rank() + 97) << char(square.file() + 49);
    return os;
}

bool operator<(const Square& lhs, const Square& rhs) {
    return lhs.index() < rhs.index();
}

bool operator==(const Square& lhs, const Square& rhs) {
    return lhs.index() == rhs.index();
}
