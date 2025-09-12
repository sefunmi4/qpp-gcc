#include <iostream>
#include <string>

int main(int argc, char** argv) {
    // Print a helpful message describing how to use Q++ when invoked with
    // --help or -h.  If no arguments are given, we also show the message so
    // users learn how to invoke the compiler.
    if (argc < 2 || std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h") {
        std::cout << "Q++ compiler and simulator\n"
                  << "Usage: qpp [options] <source-file.qpp>\n\n"
                  << "Options:\n"
                  << "  -h, --help    Show this help message and exit\n\n"
                  << "A Q++ program is stored in a file with the extension .qpp.\n"
                  << "Compile and run a program with:\n"
                  << "  qpp hello.qpp\n";
        return 0;
    }

    // Placeholder for future compiler logic.
    // TODO: add real compilation and simulation steps here.
    return 0;
}

