#include "Uci.hpp"
#include "EngineFactory.hpp"
#include "Fen.hpp"
#include "Engine.hpp"

#include <fstream>
#include <iostream>
#include <cstdlib>

int main(int argc, char* argv[]) {
    auto engine = EngineFactory::createEngine();

    if (engine == nullptr) {
        std::cerr << "Failed to create engine\n";
        return EXIT_FAILURE;
    }

    /************************
     *
     * DEBUGGING
     *
     * *************************/

    auto debug_board = Fen::createBoard("2bb1k1r/1p3p2/p6p/4pNp1/2q5/8/PPP2PPP/1K1RR3 w - - 0 24");
    if(debug_board.has_value()) {
        auto pv = engine->pv(debug_board.value());
        std::cout << "PV: " << pv << '\n';
    }

    if (argc > 1) {
        auto fen = argv[1];
        auto board = Fen::createBoard(fen);

        if (!board.has_value()) {
            std::cerr << "Parsing FEN failed\n";
            return EXIT_FAILURE;
        }

        auto pv = engine->pv(board.value());
        std::cout << "PV: " << pv << '\n';
    } else {
        auto uciLog = std::ofstream("uci-log.txt");
        auto uci = Uci(std::move(engine), std::cin, std::cout, uciLog);
        uci.run();
    }
}
