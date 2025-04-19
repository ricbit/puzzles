#ifndef PRINTERS_H
#define PRINTERS_H

#include <iostream>
#include <vector>
#include <string>
#include <map>

class GroupPrinter {
public:    
    GroupPrinter(int h_, int w_, 
                 const std::vector<std::vector<char>>& groups_,
                 const std::map<int, std::string>& cell_symbols_)
        : h(h_), w(w_), groups(groups_), cell_symbols(cell_symbols_) {}

    bool diff_right(int y, int x) const {
        return x + 1 < w && groups[y][x] != groups[y][x + 1];
    }

    bool diff_down(int y, int x) const {
        return y + 1 < h && groups[y][x] != groups[y + 1][x];
    }

    std::string get_corner(int y, int x) const {
        char a = (y > 0 && x > 0)     ? groups[y - 1][x - 1] : 0;
        char b = (y > 0 && x < w)     ? groups[y - 1][x    ] : 0;
        char c = (y < h && x > 0)     ? groups[y    ][x - 1] : 0;
        char d = (y < h && x < w)     ? groups[y    ][x    ] : 0;

        bool up = a != b;
        bool down = c != d;
        bool left = a != c;
        bool right = b != d;

        int count = up + down + left + right;

        if (count == 0) return " ";
        if (count == 1) {
            if (up || down) return "│";
            return "─";
        }
        if (count == 2) {
            if (up && down) return "│";
            if (left && right) return "─";
            if (up && right) return "└";
            if (up && left) return "┘";
            if (down && right) return "┌";
            if (down && left) return "┐";
        }
        if (count == 3) {
            if (!up) return "┬";
            if (!down) return "┴";
            if (!left) return "├";
            if (!right) return "┤";
        }
        return "┼";
    }

    void print(const std::vector<std::vector<int>>& solution) const {
        // Top border
        std::cout << "┌";
        for (int x = 0; x < w - 1; ++x)
            std::cout << "──" << (diff_right(0, x) ? "┬" : "─");
        std::cout << "──┐\n";

        // Rows
        for (int y = 0; y < h; ++y) {
            // Content row
            std::cout << "│";
            for (int x = 0; x < w; ++x) {
                auto it = cell_symbols.find(solution[y][x]);
                if (it != cell_symbols.end()) {
                    std::cout << it->second << " ";
                } else {
                    std::cout << "  ";
                }
                if (x < w - 1)
                    std::cout << (diff_right(y, x) ? "│" : " ");
            }
            std::cout << "│\n";

            // Border row
            if (y < h - 1) {
                for (int x = 0; x <= w; ++x) {
                    std::cout << get_corner(y + 1, x);
                    if (x < w)
                        std::cout << (diff_down(y, x) ? "──" : "  ");
                }
                std::cout << '\n';
            }        
        }

        // Bottom border
        std::cout << "└";
        for (int x = 0; x < w - 1; ++x)
            std::cout << "──" << (diff_right(h - 1, x) ? "┴" : "─");
        std::cout << "──┘\n";
    }

private:
    int h, w;
    std::vector<std::vector<char>> groups;
    std::map<int, std::string> cell_symbols;
};

#endif // PRINTERS_H 