#include <cstdio>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>

using Lines = std::vector<std::string>;

struct termios orig_termios;

void disableRawMode() 
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enableRawMode()
{
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disableRawMode);

    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void renderScreen(const Lines& buffer, int cursorX, int cursorY)
{
    std::cout << "\x1b[?25l";
    std::cout << "\x1b[2J\x1b[H";

    for (size_t i = 0; i < buffer.size(); ++i)
    {
        std::cout << buffer[i];
        if (i < buffer.size() - 1) std::cout << "\r\n";
    }

    std::cout << "\x1b[" << (cursorY + 1) << ";" << (cursorX + 1) << "H";
    std::cout << "\x1b[?25h" << std::flush;
}

Lines loadFile(const std::string& filename)
{
    Lines buffer;
    std::ifstream file(filename);

    if (!file.is_open())
    {
        std::cerr << "Error: Could not open file '" << filename << "'\n";
        return buffer;
    }

    std::string line;
    while (std::getline(file, line))
    {
        buffer.push_back(line);
    }

    file.close();
    return buffer;
}

void saveFile(const std::string& filename, const Lines& buffer)
{
    std::ofstream file(filename);

    if (!file.is_open()) return;

    for (size_t i = 0; i < buffer.size(); ++i)
    {
        file << buffer[i];

        if (i < buffer.size() - 1)
            file << '\n';
    }

    file.close();
}

void handleKeypress(char& c, int& cursorX, int& cursorY, Lines& buffer)
{
    // handle return / newline
    if (c == '\r' || c == '\n')
    {
        std::string currentLine = buffer[cursorY];
        std::string nextLine = currentLine.substr(cursorX);

        buffer[cursorY] = currentLine.substr(0, cursorX);
        buffer.insert(buffer.begin() + cursorY + 1, nextLine);

        cursorY++;
        cursorX = 0;
    }
    // handle backspace
    else if (c == 127 || c == '\b')
    {
        if (cursorX > 0)
        {
            buffer[cursorY].erase(cursorX - 1, 1);
            cursorX--;
        }
        else if (cursorY > 0)
        {
            int prevLineLength = buffer[cursorY - 1].length();
            buffer[cursorY - 1] += buffer[cursorY];
            buffer.erase(buffer.begin() + cursorY);

            cursorY--;
            cursorX = prevLineLength;
        }
    }
    // handle standard alphanumerical characters
    else if (c >= 32 && c <= 126)
    {
        buffer[cursorY].insert(cursorX, 1, c);
        cursorX++;
    }
    // handle arrow keys
    else if (c == '\x1b')
    {
        char seq[2];
        if (read(STDIN_FILENO, &seq[0], 1) != 1) return;
        if (read(STDIN_FILENO, &seq[1], 1) != 1) return;

        if (seq[0] == '[')
        {
            switch (seq[1])
            {
                case 'A':   // up arrow
                    if (cursorY > 0) cursorY--;
                    break;
                case 'B':   // down arrow
                    if (cursorY < static_cast<int>(buffer.size() - 1)) ++cursorY;
                    break;
                case 'C':   // right arrow
                    if (cursorX < static_cast<int>(buffer[cursorY].length())) ++cursorX;
                    break;
                case 'D':   // left arrow
                    if (cursorX > 0) cursorX--;
                    break;
            }

            if (cursorX > static_cast<int>(buffer[cursorY].length()))
            {
                cursorX = buffer[cursorY].length();
            }

            return;
        }
    }
}

int main() 
{
    enableRawMode();

    Lines buffer = loadFile("test.txt");
    int cursorX = 0;
    int cursorY = 0;

    while (true) 
    {
        renderScreen(buffer, cursorX, cursorY);

        char c;
        if (read(STDIN_FILENO, &c, 1) != 1) continue;
        if (c == 17) break;
        if (c == 19)
        {
            saveFile("test.txt", buffer);
            break;
        }

        handleKeypress(c, cursorX, cursorY, buffer);

    }

    std::cout << "\x1b[2J\x1b[H" << std::flush;

    return 0;
}
