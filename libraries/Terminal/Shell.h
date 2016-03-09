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

class Shell;

#if defined(__arm__)
#define SHELL_MAX_CMD_LEN   256
#else
#define SHELL_MAX_CMD_LEN   64
#endif

typedef void (*ShellCommandFunc)(Shell &shell, int argc, char *argv[]);
typedef bool (*ShellPasswordCheckFunc)(const char *userid, const char *password);

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
    void end();

    void loop();

    static void registerCommand(ShellCommandRegister *cmd);

    const char *prompt() const { return prom; }
    void setPrompt(const char *prompt) { prom = prompt; }

    bool hideCharacters() const { return hideChars; }
    void setHideCharacters(bool hide);

    void help();

private:
    char buffer[SHELL_MAX_CMD_LEN];
    char *argv[SHELL_MAX_CMD_LEN / 2];
    size_t maxHistory;
    size_t curLen;
    char *history;
    size_t historyWrite;
    size_t historyPosn;
    const char *prom;
    bool hideChars;

    // Disable copy constructor and operator=().
    Shell(const Shell &other) {}
    Shell &operator=(const Shell &) { return *this; }

    void execute();
    bool execute(int argc, char **argv);
    void executeBuiltin(const char *cmd);
    void clearCharacters(size_t len);
    void changeHistory();
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
