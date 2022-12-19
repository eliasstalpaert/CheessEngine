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

    TimeInfo time;
    time.white.timeLeft = std::chrono::milliseconds(300000000);
    time.black.timeLeft = std::chrono::milliseconds(300000000);
    auto debug_board = Fen::createBoard("6k1/r4p2/6p1/4B3/p4P2/5r1p/K1R5/8 b - - 5 43");
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
