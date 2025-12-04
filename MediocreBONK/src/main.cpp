#include "Core/Game.h"
#include "States/MenuState.h"
#include "Utils/Logger.h"
#include <memory>

int main()
{
    using namespace MediocreBONK;

    Utils::Logger::info("MediocreBONK starting...");

    try
    {
        Core::Game game;

        // Push initial state (Menu)
        game.getStateMachine().pushState(std::unique_ptr<States::State>(new States::MenuState()));

        // Run game loop
        game.run();
    }
    catch (const std::exception& e)
    {
        Utils::Logger::error(std::string("Exception: ") + e.what());
        return 1;
    }

    Utils::Logger::info("MediocreBONK shut down successfully");
    return 0;
}
