/*
 * Copyright (C) 2016 Southern Storm Software, Pty Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef SHELL_h
#define SHELL_h

#include "Terminal.h"
#include <Client.h>

class Shell;
class ShellArguments;
class LoginShell;

#if defined(__arm__)
#define SHELL_MAX_CMD_LEN   256
#else
#define SHELL_MAX_CMD_LEN   64
#endif

typedef void (*ShellCommandFunc)(Shell &shell, int argc, const ShellArguments &argv);

/** @cond */

typedef struct
{
    const char *name;
    const char *help;
    ShellCommandFunc func;

} ShellCommandInfo;

class ShellCommandRegister
{
public:
    inline ShellCommandRegister(const ShellCommandInfo *_info);

    const ShellCommandInfo *info;
    ShellCommandRegister *next;
};

/** @endcond */

class Shell : public Terminal
{
public:
    Shell();
    virtual ~Shell();

    bool begin(Stream &stream, size_t maxHistory = 0, Terminal::Mode mode = Serial);
    bool begin(Client &client, size_t maxHistory = 0, Terminal::Mode mode = Telnet);
    void end();

    void loop();

    static void registerCommand(ShellCommandRegister *cmd);

    const char *prompt() const { return prom; }
    void setPrompt(const char *prompt) { prom = prompt; }

    int userid() const { return uid; }
    void setUserid(int userid) { uid = userid; }

    void help();
    void exit();

protected:
    virtual void beginSession();
    virtual void printPrompt();
    virtual void execute();

private:
    char buffer[SHELL_MAX_CMD_LEN];
    size_t curStart;
    size_t curLen;
    size_t curMax;
    char *history;
    size_t historyWrite;
    size_t historyRead;
    size_t historySize;
    const char *prom;
    bool isClient;
    uint8_t lineMode;
    int uid;
    unsigned long timer;

    // Disable copy constructor and operator=().
    Shell(const Shell &other) {}
    Shell &operator=(const Shell &) { return *this; }

    bool beginShell(Stream &stream, size_t maxHistory, Terminal::Mode mode);
    bool execute(const ShellArguments &argv);
    void executeBuiltin(const char *cmd);
    void clearCharacters(size_t len);
    void changeHistory(bool up);
    void clearHistory();

    friend class LoginShell;
};

class ShellArguments
{
    friend class Shell;
private:
    ShellArguments(char *buffer, size_t len);
    ~ShellArguments() {}
public:

    int count() const { return argc; }
    const char *operator[](int index) const;

private:
    const char *line;
    size_t size;
    int argc;
    mutable int currentIndex;
    mutable size_t currentPosn;

    // Disable copy constructor and operator=().
    ShellArguments(const ShellArguments &other) {}
    ShellArguments &operator=(const ShellArguments &) { return *this; }
};

/** @cond */

inline ShellCommandRegister::ShellCommandRegister(const ShellCommandInfo *_info)
    : info(_info)
    , next(0)
{
    Shell::registerCommand(this);
}

/** @endcond */

#define ShellCommand(name,help,function)    \
    static char const shell_id_##name[] PROGMEM = #name; \
    static char const shell_help_##name[] PROGMEM = help; \
    static ShellCommandInfo const shell_info_##name PROGMEM = { \
        shell_id_##name, \
        shell_help_##name, \
        (function) \
    }; \
    static ShellCommandRegister shell_cmd_##name(&shell_info_##name)

#endif
