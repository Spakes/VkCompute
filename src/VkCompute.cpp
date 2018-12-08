#include <iostream>

#include "Application.h"

int main(int argc, char* args[])
{
    std::cout << "Running VkCompute!\n";

    Application app;
    app.mValidation = true;
    app.Run();
}