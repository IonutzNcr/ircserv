#include "../../includes/Debugger.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <ctime>
#include <string>
#include <cstdio>   // snprintf

int* Debugger::fd = NULL;

void Debugger::createLogFile()
{
    if (!Debugger::fd)
    {
        Debugger::fd = new int;
        *Debugger::fd = open(".log.md", O_CREAT | O_WRONLY, 0644);
    }
}

static std::string sanitizeForMarkdown(std::string s)
{
    // remplace \r\n et \n par espace (évite de casser _italique_ / **gras**)
    for (size_t i = 0; i < s.size(); ++i)
    {
        if (s[i] == '\n' || s[i] == '\r')
            s[i] = ' ';
    }
    return s;
}

void Debugger::storeLog(int type, std::string msg)
{
    if (!Debugger::fd || *Debugger::fd == -1)
        return;

    std::time_t now = std::time(nullptr);
    std::tm* ltm = std::localtime(&now);

    char timeBuffer[64];
    size_t n = std::strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d::%H:%M:%S", ltm);
    if (n == 0)
        std::snprintf(timeBuffer, sizeof(timeBuffer), "unknown-time");

    msg = sanitizeForMarkdown(msg);

    std::string formattedMsg = "[" + std::string(timeBuffer) + "] ";

    switch (type)
    {
        case 1: formattedMsg += msg; break;                 // normal
        case 2: formattedMsg += "_" + msg + "_"; break;     // italique
        case 3: formattedMsg += "**" + msg + "**"; break;   // bold
        default: formattedMsg += msg; break;
    }

    formattedMsg += "\n";
    write(*Debugger::fd, formattedMsg.c_str(), formattedMsg.size());
}

void Debugger::destroyFD()
{
    if (Debugger::fd)
    {
        if (*Debugger::fd != -1)
            close(*Debugger::fd);   // ✅ fermer le fichier

        delete Debugger::fd;
        Debugger::fd = NULL;        // ✅ éviter pointeur dangling
    }
}
