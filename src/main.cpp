#define SDL_MAIN_HANDLED
#include "View/view.h"

#include <iostream>

int main(int argc, char* argv[])
{
    try {
        // Create application controller
        ViewManager vm;
        
        // Initialize all components
        if (!vm.initialize()) {
            std::cerr << "Failed to initialize application. Exiting." << std::endl;
            return 1;
        }
        
        // Run the application - this starts the main loop
        vm.run();
        
        
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    catch (...) {
        std::cerr << "Unknown fatal error occurred." << std::endl;
        return 1;
    }
}