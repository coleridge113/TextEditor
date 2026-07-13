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

void renderScreen(const Lines& buffer) 
{
    std::cout << "\x1b[2J";
    std::cout << "\x1b[H";

    for (size_t i = 0; i < buffer.size(); ++i)
    {
        std::cout << buffer[i];

        if (i < buffer.size() - 1)
            std::cout << '\n';
    }

    std::cout << std::flush;
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

int main() 
{
    enableRawMode();

    std::cout << "Now in raw mode" << '\n';
    std::cout << "Press 'q' to quite" << '\n';

    char c;

    while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q')
    {
        std::cout << "You pressed: " << c << " (ASCII: " << (int)c << ")\r\n";
    }

    // std::string targetFile = "test.txt";
    // Lines editorBuffer = loadFile(targetFile);
    // renderScreen(editorBuffer);

    return 0;
}
