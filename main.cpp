#include "./src/engine.cpp"  

static int Main()
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