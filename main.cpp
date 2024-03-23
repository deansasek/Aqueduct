#include "./Src/Common.h"
#include "./Src/Engine.h"  

int main()
{
    try
    {
        Engine::Run();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}