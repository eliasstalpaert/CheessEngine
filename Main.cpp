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
/*
    TimeInfo time;
    time.white.timeLeft = std::chrono::milliseconds(3000);
    time.black.timeLeft = std::chrono::milliseconds(3000);
    auto debug_board = Fen::createBoard("r1r3k1/p3nppp/5q2/3pp3/3nQ1P1/P1P2N1P/P2PPPB1/R1B1KR2 w Q - 0 15");
    if(debug_board.has_value()) {
        auto pv = engine->pv(debug_board.value(),time);
        std::cout << "PV: " << pv << '\n';
    }
*/
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
