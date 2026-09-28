#include "VulkanApp.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>

int main()
{
    VulkanApp app;

    try
    {
        app.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Erro: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}