#ifndef CHESS_ENGINE_ENGINEFACTORY_HPP
#define CHESS_ENGINE_ENGINEFACTORY_HPP

#include "Engine.hpp"
#include "CheessEngine.hpp"
#include <memory>

class EngineFactory {
public:

    static std::unique_ptr<Engine> createEngine();
};

#endif
