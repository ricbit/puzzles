#ifndef PRINTERS_BRANCHES_H
#define PRINTERS_BRANCHES_H

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include "printers.h"

struct Group {
  int size, j, i;
};

// Function to print the Branches puzzle solution grid
inline void print_branches_grid(
    int h, int w,
    const std::vector<Group>& groups, // Need group sizes for symbols
    const std::vector<std::vector<int>>& solution_grid,
    const std::vector<std::vector<char>>& groupmap)
{
    // Define special IDs used in solution_grid
    const int EMPTY_CELL = -1;
    const int VERTICAL_LINE = -2;
    const int HORIZONTAL_LINE = -3;

    // ANSI color codes
    const std::string green = "\033[32m";
    const std::string reset = "\033[0m";

    // Define symbols for the cells (using Unicode circled numbers/letters)
    const std::string circled[36] = {
        "⓪","①","②","③","④","⑤","⑥","⑦","⑧","⑨",
        "Ⓐ","Ⓑ","Ⓒ","Ⓓ","Ⓔ","Ⓕ","Ⓖ","Ⓗ","Ⓘ","Ⓙ",
        "Ⓚ","Ⓛ","Ⓜ","Ⓝ","Ⓞ","Ⓟ","Ⓠ","Ⓡ","Ⓢ","Ⓣ",
        "Ⓤ","Ⓥ","Ⓦ","Ⓧ","Ⓨ","Ⓩ"
    };

    // Map the integer group identifiers to their string symbols
    std::map<int, std::string> cell_symbols;

    // Add symbols for lines and empty cells
    cell_symbols[EMPTY_CELL] = "  "; // Two spaces for empty
    cell_symbols[VERTICAL_LINE] = green + "│" + reset; // Use green for lines
    cell_symbols[HORIZONTAL_LINE] = green + "─" + reset; // Use green for lines

    // Add symbols for the seed cells based on their group index (0 to gs-1)
    int gs = groups.size();
    for (int g_idx = 0; g_idx < gs; g_idx++) {
        if (groups[g_idx].size >= 0 && groups[g_idx].size < 36) {
            // Map the group index (g_idx) to the symbol representing its size
            cell_symbols[g_idx] = circled[groups[g_idx].size];
        } else {
            cell_symbols[g_idx] = "??"; // Fallback for unexpected sizes
            std::cerr << "Warning: Group " << g_idx << " has unexpected size " << groups[g_idx].size << std::endl;
        }
    }

    // Create and use the GroupPrinter
    GroupPrinter printer(h, w, groupmap, cell_symbols, 2); // Cell width 2
    printer.print(solution_grid); // Pass the grid containing integer identifiers
}

#endif // PRINTERS_BRANCHES_H
