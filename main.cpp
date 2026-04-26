#include <iostream>

#include "Application.h"

/**
* @brief No idea what this is (I am lying).
*/
int main() 
{
    Application app{};

    if (app.Init())
    {
        app.Run();
    }
    else
    {
        std::cerr << "{main} Failed to initialise app" << std::endl;
    }

    app.Shutdown();
    return 0;
}