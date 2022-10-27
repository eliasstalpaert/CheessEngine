#ifndef CHESS_ENGINE_UCI_HPP
#define CHESS_ENGINE_UCI_HPP

#include "Board.hpp"
#include "TimeInfo.hpp"

#include <string>
#include <iosfwd>
#include <memory>
#include <map>

class Engine;
class PrincipalVariation;
class UciOptionBase;

class Uci {
public:

    Uci(std::unique_ptr<Engine> engine,
        std::istream& cmdIn,
        std::ostream& cmdOut,
        std::ostream& log);
    ~Uci();

    void run();

private:

    void runCommand(const std::string& line);
    void uciCommand(std::istream& stream);
    void isreadyCommand(std::istream& stream);
    void ucinewgameCommand(std::istream& stream);
    void positionCommand(std::istream& stream);
    void goCommand(std::istream& stream);
    void quitCommand(std::istream& stream);
    void setoptionCommand(std::istream& stream);
    TimeInfo::Optional readTimeInfo(std::istream& stream);
    void sendPvInfo(const PrincipalVariation& pv);
    void sendOptions();
    void sendCommand(const std::string& line);
    void error(const std::string& msg);

    std::unique_ptr<Engine> engine_;
    Board board_;
    std::istream& cmdIn_;
    std::ostream& cmdOut_;
    std::ostream& log_;
    std::map<std::string, std::unique_ptr<UciOptionBase>> options_;
};

#endif
