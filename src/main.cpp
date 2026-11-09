#include "pfv/App.hpp"

#include <exception>
#include <iostream>

int main() {
    try {
        pfv::App app;
        return app.run();
    } catch (const std::exception& ex) {
        std::cerr << "Fatal error: " << ex.what() << '\n';
        return 1;
    } catch (...) {
        std::cerr << "Fatal error: An unexpected exception occurred.\n";
        return 1;
    }
}
