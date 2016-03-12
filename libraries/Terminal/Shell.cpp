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

#include "Shell.h"
#include "LoginShell.h"
#include <string.h>
#include <stddef.h>

/**
 * \class Shell Shell.h <Shell.h>
 * \brief Command-line shell access.
 *
 * This class provides a command-line shell via serial ports, TCP connections,
 * or any other type of Stream.
 *
 * The following example is the minimal setup for a command-line shell
 * on a serial port.  The application calls begin() to set the underlying
 * Stream, and periodically calls loop() to manage shell-related events.
 *
 * \code
 * Shell shell;
 *
 * void setup() {
 *     Serial.begin(9600);
 *     shell.setPrompt("$ ");
 *     shell.begin(Serial);
 * }
 *
 * void loop() {
 *     shell.loop();
 * }
 * \endcode
 *
 * Commands can be registered with the shell by the application to be
 * invoked when the user types in the corresponding command.  Each
 * command is associated with a handler function:
 *
 * \code
 * void cmdMotor(Shell &shell, int argc, const ShellArguments &argv)
 * {
 *     ...
 * }
 *
 * ShellCommand(motor, "Turn the motor on or off", cmdMotor);
 * \endcode
 *
 * There are two standard commands built into Shell: "help" and "exit".
 * The "help" command provides a list of all registered commands with
 * the short help string from the ShellCommand() registration.
 * The "exit" command logs the user out and returns to the login prompt,
 * or stops the underlying connection in the case of TCP streams.
 *
 * The F1 key can be used as a synonym for "help" and CTRL-D can be used
 * as a synonym for "exit".
 *
 * Shell provides some limited history editing for scrolling back through
 * previous commands.  The size of the history stack is provided in the
 * second argument to begin():
 *
 * \code
 * shell.begin(Serial, 5);
 * \endcode
 *
 * \sa LoginShell, Terminal
 */

/**
 * \def SHELL_MAX_CMD_LEN
 * \brief Maximum command length for the shell, including the terminating NUL.
 */

/**
 * \typedef ShellCommandFunc
 * \brief Type of functions that provide shell command handlers.
 *
 * \param shell Points to the shell instance that executed the command,
 * which can be used to print command results or read more input.
 * \param argc Number of arguments to the command, including the
 * command's name.
 * \param argv The arguments to the command.
 *
 * \sa ShellCommand()
 * \relates Shell
 */

// Modes for line editing (flags).
#define LINEMODE_NORMAL     0x01
#define LINEMODE_ECHO       0x02
#define LINEMODE_USERNAME   0x04
#define LINEMODE_PASSWORD   0x08
#define LINEMODE_PROMPT     0x10
#define LINEMODE_DELAY      0x20

// Delay to insert after a failed login to slow down brute force attacks (ms).
#define LOGIN_SHELL_DELAY   3000

/**
 * \brief Constructs a new Shell instance.
 *
 * This constructor must be followed by a call to begin() to specify
 * the underlying I/O stream.
 */
Shell::Shell()
    : maxHistory(0)
    , curStart(0)
    , curLen(0)
    , curMax(sizeof(buffer))
    , history(0)
    , historyWrite(0)
    , historyPosn(0)
    , prom("$ ")
    , isClient(false)
    , lineMode(LINEMODE_NORMAL | LINEMODE_ECHO)
    , timer(0)
{
}

/**
 * \brief Destroys this Shell object.
 */
Shell::~Shell()
{
    clearHistory();
    delete [] history;
}

/**
 * \brief Begin shell handling on an underlying character stream.
 *
 * \param stream The stream to apply the shell to.  Usually this is a
 * serial port or TCP network connection.
 * \param maxHistory The number of commands to allocate in the history
 * stack for scrolling back through using Up/Down arrow keys.
 * \param mode The terminal mode to operate in, Terminal::Serial or
 * Terminal::Telnet.  Default is Terminal::Serial.
 * \return Returns false if there is insufficient memory for the history
 * stack.  The session will continue but without command history.
 *
 * This function will print the prompt() in preparation for entry of
 * the first command.  The default prompt is "$ "; call setPrompt()
 * before begin() to change this:
 *
 * \code
 * Serial.begin(9600);
 * shell.setPrompt("Command: ");
 * shell.begin(Serial);
 * \endcode
 *
 * \sa end(), setPrompt()
 */
bool Shell::begin(Stream &stream, size_t maxHistory, Terminal::Mode mode)
{
    if (!beginShell(stream, maxHistory, mode))
        return false;
    isClient = false;
    return true;
}

/**
 * \brief Begin shell handling on a connected TCP client.
 *
 * \param client The client to apply the shell to.  This must be a
 * connected TCP client.
 * \param maxHistory The number of commands to allocate in the history
 * stack for scrolling back through using Up/Down arrow keys.
 * \param mode The terminal mode to operate in, Terminal::Serial or
 * Terminal::Telnet.  Default is Terminal::Telnet.
 * \return Returns true if the shell was initialized, or false if there
 * is insufficient memory for the history stack.
 *
 * This override is provided as a convenience for starting a shell on a
 * TCP connection.  This function also modifies the behaviour of the
 * builtin "exit" command to forcibly stop the TCP connection rather
 * than returning to the login prompt.
 *
 * \sa end(), setPrompt()
 */
bool Shell::begin(Client &client, size_t maxHistory, Terminal::Mode mode)
{
    if (!beginShell(client, maxHistory, mode))
        return false;
    isClient = true;
    return true;
}

/**
 * \brief Internal implementation of begin().
 */
bool Shell::beginShell(Stream &stream, size_t maxHistory, Terminal::Mode mode)
{
    // Initialize the Terminal base class with the underlying stream.
    Terminal::begin(stream, mode);

    // Create the history buffer.
    bool ok = true;
    this->maxHistory = maxHistory;
    delete [] history;
    if (maxHistory) {
        history = new char [sizeof(buffer) * maxHistory];
        if (history) {
            memset(history, 0, sizeof(buffer) * maxHistory);
        } else {
            this->maxHistory = 0;
            ok = false;
        }
    } else {
        history = 0;
    }

    // Clear other variables.
    curStart = 0;
    curLen = 0;
    curMax = sizeof(buffer);
    historyWrite = 0;
    historyPosn = 0;

    // Begins the login session.
    beginSession();
    return ok;
}

/**
 * \brief Ends shell processing on the underlying stream.
 *
 * This function is intended to be called when a TCP network connection
 * is closed to clean up the shell state that was in use by the connection.
 *
 * \sa begin()
 */
void Shell::end()
{
    Terminal::end();
    clearHistory();
    delete [] history;
    maxHistory = 0;
    curStart = 0;
    curLen = 0;
    curMax = sizeof(buffer);
    history = 0;
    historyWrite = 0;
    historyPosn = 0;
    isClient = false;
    lineMode = LINEMODE_NORMAL | LINEMODE_ECHO;
}

/** @cond */

// Standard builtin command names.
static char const builtin_cmd_exit[] PROGMEM = "exit";
static char const builtin_cmd_help[] PROGMEM = "help";
static char const builtin_cmd_help_alt[] PROGMEM = "?";

/** @endcond */

/**
 * \brief Performs regular activities on the shell.
 *
 * This function must be called regularly from the application's main loop
 * to process input for the shell.
 */
void Shell::loop()
{
    // If the stream is a TCP client, then check for disconnection.
    if (isClient && !((Client *)stream())->connected()) {
        end();
        return;
    }

    // If the login delay is active, then suppress all input.
    if (lineMode & LINEMODE_DELAY) {
        if ((millis() - timer) >= LOGIN_SHELL_DELAY) {
            lineMode &= ~LINEMODE_DELAY;
            timer = 0;
        } else {
            readKey();
            return;
        }
    }

    // Print the prompt if necessary.
    if (lineMode & LINEMODE_PROMPT)
        printPrompt();

    // Read the next key and bail out if none.  We only process a single
    // key each time we enter this function to prevent other tasks in the
    // system from becoming starved of time resources if the bytes are
    // arriving rapidly from the underyling stream.
    int key = readKey();
    if (key == -1)
        return;

    // Process the key.
    switch (key) {
    case KEY_BACKSPACE:
        // Backspace over the last character.
        clearCharacters(1);
        break;

    case KEY_RETURN:
        // CR, LF, or CRLF pressed, so execute the current command.
        execute();
        break;

    case 0x15:
        // CTRL-U - clear the entire command.
        clearCharacters(curLen);
        break;

    case 0x04:
        // CTRL-D - equivalent to the "exit" command.
        if (lineMode & LINEMODE_NORMAL)
            executeBuiltin(builtin_cmd_exit);
        break;

    case KEY_UP_ARROW:
        // Go back one item in the command history.
        if ((lineMode & LINEMODE_NORMAL) != 0 &&
                history && historyPosn < maxHistory) {
            ++historyPosn;
            changeHistory();
        }
        break;

    case KEY_DOWN_ARROW:
        // Go forward one item in the command history.
        if ((lineMode & LINEMODE_NORMAL) != 0 &&
                history && historyPosn > 0) {
            --historyPosn;
            changeHistory();
        }
        break;

    case KEY_F1:
        // F1 is equivalent to the "help" command.
        if (lineMode & LINEMODE_NORMAL)
            executeBuiltin(builtin_cmd_help);
        break;

    case KEY_UNICODE: {
        // Add the Unicode code point to the buffer if it will fit.
        long code = unicodeKey();
        size_t size = Terminal::utf8Length(code);
        if (size && (curLen + size) < (curMax - 1)) {
            Terminal::utf8Format((uint8_t *)(buffer + curLen), code);
            if (lineMode & LINEMODE_ECHO)
                write((uint8_t *)(buffer + curLen), size);
            curLen += size;
        }
    } break;

    default:
        if (key >= 0x20 && key <= 0x7E) {
            // Printable ASCII character - echo and add it to the buffer.
            if (curLen < (curMax - 1)) {
                if (lineMode & LINEMODE_ECHO)
                    write((uint8_t)key);
                buffer[curLen++] = (char)key;
            }
        }
        break;
    }
}

#if defined(__AVR__)

// String compare of two strings in program memory.
static int progmem_strcmp(const char *str1, const char *str2)
{
    uint8_t ch1, ch2;
    for (;;) {
        ch1 = pgm_read_byte((const uint8_t *)str1);
        ch2 = pgm_read_byte((const uint8_t *)str2);
        if (!ch1) {
            if (ch2)
                return -1;
            else
                break;
        } else if (!ch2) {
            return 1;
        } else if (ch1 != ch2) {
            return ((int)ch1) - ((int)ch2);
        }
        ++str1;
        ++str2;
    }
    return 0;
}

#else

#define progmem_strcmp(str1,str2) (strcmp((str1), (str2)))

#endif

// Reads the "name" field from a command information block in program memory.
static const char *readInfoName(const ShellCommandInfo *info)
{
#if defined(__AVR__)
    return (const char *)pgm_read_word
        (((const uint8_t *)info) + offsetof(ShellCommandInfo, name));
#else
    return info->name;
#endif
}

// Reads the "help" field from a command information block in program memory.
static const char *readInfoHelp(const ShellCommandInfo *info)
{
#if defined(__AVR__)
    return (const char *)pgm_read_word
        (((const uint8_t *)info) + offsetof(ShellCommandInfo, help));
#else
    return info->help;
#endif
}

// Reads the "func" field from a command information block in program memory.
static ShellCommandFunc readInfoFunc(const ShellCommandInfo *info)
{
#if defined(__AVR__)
    if (sizeof(ShellCommandFunc) == 2) {
        return (ShellCommandFunc)pgm_read_word
            (((const uint8_t *)info) + offsetof(ShellCommandInfo, func));
    } else {
        return (ShellCommandFunc)pgm_read_dword
            (((const uint8_t *)info) + offsetof(ShellCommandInfo, func));
    }
#else
    return info->func;
#endif
}

static ShellCommandRegister *firstCmd = 0;

/**
 * \brief Registers a command with the shell.
 *
 * \note This function is internal.  The ShellCommand() macro should be
 * used instead.
 */
void Shell::registerCommand(ShellCommandRegister *cmd)
{
    // Insert the command into the list in alphanumeric order.
    // We cannot rely upon the construction order to sort the list for us.
    ShellCommandRegister *prev = 0;
    ShellCommandRegister *current = firstCmd;
    while (current != 0) {
        if (progmem_strcmp(readInfoName(cmd->info), readInfoName(current->info)) < 0)
            break;
        prev = current;
        current = current->next;
    }
    if (prev)
        prev->next = cmd;
    else
        firstCmd = cmd;
    cmd->next = current;
}

/**
 * \fn const char *Shell::prompt() const
 * \brief Gets the prompt string to display in the shell.
 *
 * \return The current prompt.  The default is "$ ".
 *
 * \sa setPrompt()
 */

/**
 * \fn void Shell::setPrompt(const char *prompt)
 * \brief Sets the prompt string to display in the shell.
 *
 * \param prompt The new prompt string.  The caller is responsible to ensure
 * that the string persists after this call returns.  The Shell class does
 * not make a copy of the string.
 *
 * Calling this function will change the prompt for the next line of input.
 *
 * \sa prompt()
 */

/**
 * \brief Displays help for all supported commands.
 */
void Shell::help()
{
    // Find the command with the maximum length.
    ShellCommandRegister *current = firstCmd;
    size_t maxLen = 0;
    size_t len;
    while (current != 0) {
        len = strlen_P(readInfoName(current->info));
        if (len > maxLen)
            maxLen = len;
        current = current->next;
    }
    maxLen += 2;

    // Print the commands with the help strings aligned on the right.
    current = firstCmd;
    while (current != 0) {
        writeProgMem(readInfoName(current->info));
        len = maxLen - strlen_P(readInfoName(current->info));
        while (len > 0) {
            write(' ');
            --len;
        }
        writeProgMem(readInfoHelp(current->info));
        println();
        current = current->next;
    }
}

/**
 * \brief Exit from the shell back to the login prompt.
 *
 * If the underlying stream is a TCP client, then this function will
 * stop the client, causing disconnection.
 */
void Shell::exit()
{
    Stream *stream = this->stream();
    if (isClient) {
        end();
        ((Client *)stream)->stop();
    } else {
        clearHistory();
        println();
        beginSession();
    }
}

/**
 * \brief Begins a login session.
 */
void Shell::beginSession()
{
    // No login support in the base class, so enter normal mode immediately.
    lineMode = LINEMODE_NORMAL | LINEMODE_ECHO | LINEMODE_PROMPT;
}

/**
 * \brief Prints the current prompt string.
 */
void Shell::printPrompt()
{
    if (prom)
        print(prom);
    lineMode &= ~LINEMODE_PROMPT;
}

/**
 * \brief Executes the command in the buffer.
 */
void Shell::execute()
{
    // Terminate the current line.
    println();

    // Make sure the command is properly NUL-terminated.
    buffer[curLen] = '\0';

    // If we have a history stack and the new command is different from
    // the previous command, then copy the command into the stack.
    if (history && curLen > curStart) {
        char *hist = history + sizeof(buffer) * historyWrite;
        if (strcmp(hist, buffer) != 0) {
            historyWrite = (historyWrite + 1) % maxHistory;
            hist = history + sizeof(buffer) * historyWrite;
            strcpy(hist, buffer + curStart);
        }
    }

    // Reset the history read position to the top of the stack.
    historyPosn = 0;

    // Break the command up into arguments and populate the argument array.
    ShellArguments argv(buffer + curStart, curLen - curStart);

    // Clear the line buffer.
    curLen = 0;

    // Execute the command.
    if (argv.count() > 0) {
        if (!execute(argv)) {
            // Could not find a matching command, try the builtin "help".
            const char *argv0 = argv[0];
            if (!strcmp_P(argv0, builtin_cmd_help) ||
                    !strcmp_P(argv0, builtin_cmd_help_alt)) {
                help();
            } else if (!strcmp_P(argv0, builtin_cmd_exit)) {
                exit();
            } else {
                static char const unknown_cmd[] PROGMEM = "Unknown command: ";
                writeProgMem(unknown_cmd);
                print(argv0);
                println();
            }
        }
    }

    // Prepare to print the prompt for the next command.
    lineMode |= LINEMODE_PROMPT;
}

/**
 * \brief Executes a command that has been parsed into arguments.
 *
 * \param argv The arguments.
 *
 * \return Returns true if the command was found; false if not found.
 */
bool Shell::execute(const ShellArguments &argv)
{
    const char *argv0 = argv[0];
    ShellCommandRegister *current = firstCmd;
    while (current != 0) {
        if (!strcmp_P(argv0, readInfoName(current->info))) {
            ShellCommandFunc func = readInfoFunc(current->info);
            (*func)(*this, argv.count(), argv);
            return true;
        }
        current = current->next;
    }
    return false;
}

/**
 * \brief Executes a builtin command like "exit" or "help".
 *
 * \param cmd The command to execute, which must point to program memory.
 */
void Shell::executeBuiltin(const char *cmd)
{
    clearCharacters(curLen);
    curLen = strlen_P(cmd);
    strncpy_P(buffer + curStart, cmd, curLen);
    write((const uint8_t *)(buffer + curStart), curLen);
    curLen += curStart;
    execute();
}

/**
 * \brief Clears characters from the input line by backspacing over them.
 *
 * \param len The number of characters to clear.
 */
void Shell::clearCharacters(size_t len)
{
    // If the characters are hidden, then there's nothing to backspace over.
    if (!(lineMode & LINEMODE_ECHO))
        return;

    // Backspace over all characters in the buffer.
    while (len > 0 && curLen > curStart) {
        uint8_t ch = (uint8_t)(buffer[curLen - 1]);
        if (ch < 0x80) {
            backspace();
        } else {
            // UTF-8 character sequence.  Back up some more and
            // determine the value of the Unicode code point.
            long code = (ch & 0x3F);
            uint8_t shift = 6;
            while (curLen > 1) {
                --curLen;
                ch = (uint8_t)(buffer[curLen - 1]);
                if ((ch & 0xC0) != 0x80)
                    break;
                code |= ((long)(ch & 0x3F)) << shift;
                shift += 6;
            }
            if ((ch & 0xE0) == 0xC0)
                ch &= 0x1F;
            else if ((ch & 0xF0) == 0xE0)
                ch &= 0x0F;
            else
                ch &= 0x07;
            code |= ((long)ch) << shift;

            // If the character is wide, we need to emit two backspaces.
            if (isWideCharacter(code))
                backspace();
            backspace();
        }
        --len;
        --curLen;
    }
}

/**
 * \brief Changes the current command to reflect a different position
 * in the history stack.
 */
void Shell::changeHistory()
{
    // Replace the command with the historyPosn item from the stack.
    // A historyPosn of 1 is the top of the history stack and a
    // historyPosn of maxHistory is the bottom of the history stack.
    // A historyPosn of 0 means that the down arrow has navigated
    // off the history stack, so clear the command only.
    if (historyPosn) {
        size_t posn = (historyWrite + maxHistory - (historyPosn - 1)) % maxHistory;
        char *hist = history + sizeof(buffer) * posn;
        if (*hist != '\0') {
            // Copy the line from the history into the command buffer.
            clearCharacters(curLen);
            curLen = strlen(hist);
            if (curLen > (curMax - curStart))
                curLen = curMax - curStart;
            memcpy(buffer + curStart, hist, curLen);
            if (lineMode & LINEMODE_ECHO)
                write((uint8_t *)hist, curLen);
            curLen += curStart;
        } else {
            // We've gone too far - the history is still smaller
            // than maxHistory in size.  So reset the position and
            // don't go any further.
            --historyPosn;
        }
    } else {
        // We've navigated off the history stack.
        clearCharacters(curLen);
    }
}

/**
 * \brief Clears the command buffer and history.
 *
 * This clears the history so that commands from one login session do
 * not leak into the next login session.
 */
void Shell::clearHistory()
{
    if (history)
        memset(history, 0, sizeof(buffer) * maxHistory);
    historyPosn = 0;
    historyWrite = 0;
    clearCharacters(curLen);
    memset(buffer, 0, sizeof(buffer));
}

/**
 * \fn ShellCommand(name,help,function)
 * \brief Registers a command with the shell.
 *
 * \param name The name of the command.
 * \param help Help string to display that describes the command.
 * \param function The function to call to handle the command.
 *
 * The \a name and \a help parameters must be constant strings that can
 * be placed into program memory.
 *
 * \code
 * void cmdMotor(Shell &shell, int argc, const ShellArguments &argv)
 * {
 *     ...
 * }
 *
 * ShellCommand(motor, "Turn the motor on or off", cmdMotor);
 * \endcode
 *
 * If there are multiple Shell instances active in the system, then the
 * command will be registered with all of them.
 *
 * \relates Shell
 */

/**
 * \brief Constructs a new argument array.
 *
 * \param buffer Points to the command buffer to parse into arguments.
 * \param len The length of the command buffer in bytes, excluding the
 * terminating NUL.
 */
ShellArguments::ShellArguments(char *buffer, size_t len)
    : line(buffer)
    , size(0)
    , argc(0)
    , currentIndex(0)
    , currentPosn(0)
{
    // Break the command up into arguments and add NUL terminators.
    size_t posn = 0;
    size_t outposn = 0;
    char quote = 0;
    while (posn < len) {
        char ch = buffer[posn];
        if (ch == ' ') {
            ++posn;
            continue;
        }
        ++argc;
        do {
            ch = buffer[posn];
            if (ch == '"' || ch == '\'') {
                if (quote == ch) {
                    quote = 0;
                    ++posn;
                    continue;
                } else if (!quote) {
                    quote = ch;
                    ++posn;
                    continue;
                }
            } else if (!quote && ch == ' ') {
                break;
            }
            buffer[outposn++] = ch;
            ++posn;
        } while (posn < len);
        buffer[outposn++] = '\0';
        if (posn < len)
            ++posn;
    }
    size = outposn;
}

/**
 * \class ShellArguments Shell.h <Shell.h>
 * \brief Convenience class that encapsulates an array of shell
 * command arguments.
 *
 * \sa Shell
 */

/**
 * \fn ShellArguments::~ShellArguments()
 * \brief Destroys this argument array.
 */

/**
 * \fn int ShellArguments::count() const
 * \brief Returns the number of arguments, including the name of the command.
 *
 * \sa operator[]
 */

/**
 * \brief Gets a specific argument for the command.
 *
 * \param index The argument index between 0 and count() - 1.
 * \return The argument, or NULL if \a index is out of range.
 *
 * The name of the command is argument 0.  The command's remaining
 * arguments are numbered 1 to count() - 1.
 *
 * \sa count()
 */
const char *ShellArguments::operator[](int index) const
{
    if (index < 0 || index >= argc) {
        // Argument index is out of range.
        return 0;
    } else if (index == currentIndex) {
        // We already found this argument last time.
        return line + currentPosn;
    } else {
        // Search forwards or backwards for the next argument.
        const char *temp;
        while (index > currentIndex) {
            temp = (const char *)memchr
                (line + currentPosn, '\0', size - currentPosn);
            if (!temp)
                return 0;
            currentPosn = ((size_t)(temp - line)) + 1;
            ++currentIndex;
        }
        while (index < currentIndex) {
            temp = (const char *)memrchr(line, '\0', currentPosn - 1);
            if (temp)
                currentPosn = ((size_t)(temp - line)) + 1;
            else
                currentPosn = 0;
            --currentIndex;
        }
        return line + currentPosn;
    }
}

void LoginShell::beginSession()
{
    lineMode = LINEMODE_USERNAME | LINEMODE_ECHO | LINEMODE_PROMPT;
    curStart = 0;
    curLen = 0;
    curMax = sizeof(buffer) / 2;
    uid = -1;
}

/**
 * \fn const char *LoginShell::machineName() const
 * \brief Gets the name of the machine to display in the login prompt.
 *
 * The default value is NULL, indicating that no machine name should be shown.
 *
 * \sa setMachineName()
 */

/**
 * \fn void LoginShell::setMachineName(const char *machineName)
 * \brief Sets the name of the machine to display in the login prompt.
 *
 * \param machineName The machine name, or NULL for no machine name.
 *
 * \sa machineName()
 */

/**
 * \fn ShellPasswordCheckFunc LoginShell::passwordCheckFunction() const
 * \brief Gets the current password checking function, or NULL if the
 * function has not been set yet.
 *
 * \sa setPasswordCheckFunction()
 */

/**
 * \fn void LoginShell::setPasswordCheckFunction(ShellPasswordCheckFunc function)
 * \brief Sets the password checking function.
 *
 * \param function The password checking function to set, or NULL to return
 * to the default rules.
 *
 * If no function is set, then LoginShell will check for a username of
 * "root" and a password of "arduino" (both values are case-sensitive).
 * This is of course not very secure.  Realistic applications should set a
 * proper password checking function.
 *
 * \sa passwordCheckFunction()
 */

void LoginShell::printPrompt()
{
    static char const loginString[] PROGMEM = "login: ";
    static char const passwordString[] PROGMEM = "Password: ";
    if (lineMode & LINEMODE_NORMAL) {
        // Print the prompt for normal command entry.
        if (prom)
            print(prom);

        // Normal commands occupy the full command buffer.
        curStart = 0;
        curLen = 0;
        curMax = sizeof(buffer);
    } else if (lineMode & LINEMODE_USERNAME) {
        // Print the machine name and the login prompt.
        if (machName) {
            print(machName);
            write((uint8_t)' ');
        }
        writeProgMem(loginString);

        // Login name is placed into the first half of the line buffer.
        curStart = 0;
        curLen = 0;
        curMax = sizeof(buffer) / 2;
    } else if (lineMode & LINEMODE_PASSWORD) {
        // Print the password prompt.
        writeProgMem(passwordString);

        // Password is placed into the second half of the line buffer.
        curStart = sizeof(buffer) / 2;
        curLen = curStart;
        curMax = sizeof(buffer);
    }
    lineMode &= ~LINEMODE_PROMPT;
}

// Default password checking function.  This is not a very good security check!
static int defaultPasswordCheckFunc(const char *username, const char *password)
{
    static char const defaultUsername[] PROGMEM = "root";
    static char const defaultPassword[] PROGMEM = "arduino";
    if (!strcmp_P(username, defaultUsername) &&
            !strcmp_P(password, defaultPassword)) {
        return 0;
    } else {
        return -1;
    }
}

void LoginShell::execute()
{
    if (lineMode & LINEMODE_NORMAL) {
        // Normal command execution.
        Shell::execute();
    } else if (lineMode & LINEMODE_USERNAME) {
        // Prompting for the login username.
        buffer[curLen] = '\0';
        lineMode = LINEMODE_PASSWORD | LINEMODE_PROMPT;
        println();
    } else if (lineMode & LINEMODE_PASSWORD) {
        // Prompting for the login password.
        buffer[curLen] = '\0';
        println();

        // Check the user name and password.
        int userid;
        if (checkFunc)
            userid = checkFunc(buffer, buffer + sizeof(buffer) / 2);
        else
            userid = defaultPasswordCheckFunc(buffer, buffer + sizeof(buffer) / 2);

        // Clear the user name and password from memory after they are checked.
        memset(buffer, 0, sizeof(buffer));

        // Go to either normal mode or back to username mode.
        if (userid >= 0) {
            uid = userid;
            lineMode = LINEMODE_NORMAL | LINEMODE_ECHO | LINEMODE_PROMPT;
        } else {
            lineMode = LINEMODE_USERNAME | LINEMODE_ECHO |
                       LINEMODE_PROMPT | LINEMODE_DELAY;
            timer = millis();
        }
    }
}
