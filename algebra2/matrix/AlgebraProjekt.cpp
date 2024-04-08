#include <algorithm>    // Standard library header for algorithms
#include <cstdio>       // Standard I/O operations
#include <expected>     // Expected utilities
#include <filesystem>   // Filesystem operations
#include <format>       // Formatting utilities
#include <iomanip>      // Input/output manipulators
#include <iostream>     // Standard I/O streams
#include <mdspan>       // Multidimensional array span
#include <ranges>       // Range-based utilities
#include <sstream>      // String stream
#include <string_view>  // String view
#include <fstream>      // File operations (added for file writing)
#include <span>         // Span (added for span support)
#include <print>        // Print utilities (needed for writing out)
#include <d:\\AlgebraProjekt\\header.h> //Your Path 




int main()
{
    // Load matrix from file
    auto dane = load<int, std::layout_right>("matrixload.txt");
    ::ranges::matrix_view<int, std::layout_right> m = dane.second;

    // Output matrix before modification
    std::cout << "Matrix before modification:\n";
    std::cout << to_string(m, ',', '\n');

    // Subtract 2 from each element in the matrix (example)
    for (auto& row : m) {
        row -= 2;
    }

    // Output matrix after modification
    std::cout << "Matrix after modification:\n";
    std::cout << to_string(m, ',', '\n');

    // Save modified matrix to file
    save(m, "d:\\AlgebraProjekt\\matrix.txt");

    return 0;
}