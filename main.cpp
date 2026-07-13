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
    raw.c_cflag &= ~(ECHO | ICANON);
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

void handleKeypress(char& c, int& cursorX, int& cursorY, Lines& buffer)
{
    if (c == '\r' || c == '\n')
    {
        std::string currentLine = buffer[cursorY];
        std::string nextLine = currentLine.substr(cursorX);

        buffer[cursorY] = currentLine.substr(0, cursorX);
        buffer.insert(buffer.begin() + cursorY + 1, nextLine);

        cursorY++;
        cursorX = 0;
    }
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
    else if (c >= 32 && c <= 126)
    {
        buffer[cursorY].insert(cursorX, 1, c);
        cursorX++;
    }
}

int main() 
{
    enableRawMode();

    Lines buffer = {""};
    int cursorX = 0;
    int cursorY = 0;

    while (true) 
    {
        renderScreen(buffer, cursorX, cursorY);

        char c;
        if (read(STDIN_FILENO, &c, 1) != 1) continue;
        if (c == 17) break;

        handleKeypress(c, cursorX, cursorY, buffer);

    }

    std::cout << "\x1b[2J\x1b[H" << std::flush;

    return 0;
}
