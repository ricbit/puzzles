#ifndef PRINTERS_H
#define PRINTERS_H

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <cctype>
#include <cwchar>
#include <locale>
#include <codecvt>
#include <locale>
       
// Strip ANSI escape codes and compute visual width
int visible_width(const std::string& s) {
    int width = 0;
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    std::wstring ws;

    setlocale(LC_ALL, "");
    for (size_t i = 0; i < s.size();) {
        // Handle ANSI escape codes like \033[31m
        if (s[i] == '\033' && i + 1 < s.size() && s[i + 1] == '[') {
            i += 2;
            while (i < s.size() && (isdigit(s[i]) || s[i] == ';')) ++i;
            if (i < s.size()) ++i;  // Skip final 'm' or other code
        } else {
            unsigned char c = s[i];
            size_t len = 1;
            if ((c & 0xE0) == 0xC0) len = 2;
            else if ((c & 0xF0) == 0xE0) len = 3;
            else if ((c & 0xF8) == 0xF0) len = 4;

            if (i + len <= s.size()) {
                std::string ch = s.substr(i, len);
                try {
                    std::wstring wch = conv.from_bytes(ch);
                    if (!wch.empty()) {
                        int w = wcwidth(wch[0]);
                        width += (w > 0) ? w : 0;
                    }
                } catch (...) {
                    // Invalid UTF-8, ignore
                }
            }
            i += len;
        }
    }
    return width;
}

class GroupPrinter {
public:    
    GroupPrinter(int h_, int w_, 
                 const std::vector<std::vector<char>>& groups_,
                 const std::map<int, std::string>& cell_symbols_,
                 int cell_width_ = 2)  // Default cell width is 2
        : h(h_), w(w_), groups(groups_), cell_symbols(cell_symbols_), 
          cell_width(cell_width_) {}

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
        std::string hline;
        for (int i = 0; i < cell_width; i++) hline += "─";
        std::string hspace(cell_width, ' ');

        // Top border
        std::cout << "┌";
        for (int x = 0; x < w - 1; ++x)
            std::cout << hline << (diff_right(0, x) ? "┬" : "─");
        std::cout << hline << "┐\n";

        // Rows
        for (int y = 0; y < h; ++y) {
            // Content row
            std::cout << "│";
            for (int x = 0; x < w; ++x) {
                auto it = cell_symbols.find(solution[y][x]);
                if (it != cell_symbols.end()) {
                    std::cout << it->second;
                    int width = visible_width(it->second);
                    for (int i = 0; i < cell_width - width; ++i) {
                        std::cout << " ";
                    }
                } else {
                    std::cout << hspace;
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
                        std::cout << (diff_down(y, x) ? hline : hspace);
                }
                std::cout << '\n';
            }        
        }

        // Bottom border
        std::cout << "└";
        for (int x = 0; x < w - 1; ++x)
            std::cout << hline << (diff_right(h - 1, x) ? "┴" : "─");
        std::cout << hline << "┘\n";
    }

private:
    int h, w;
    std::vector<std::vector<char>> groups;
    std::map<int, std::string> cell_symbols;
    int cell_width;  // Width of each cell in spaces
};

#endif // PRINTERS_H 