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
#include <string.h>
#include <stddef.h>

/**
 * \class Shell Shell.h <Shell.h>
 * \brief Command-line shell access.
 *
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

/**
 * \typedef ShellPasswordCheckFunc
 * \brief Password checking function for login shells.
 *
 * \param userid Points to the user identifier that was supplied at login.
 * \param password Points to the password that was supplied at login.
 *
 * \return Returns true if the user identifier and password combination
 * is correct, false if incorrect.
 *
 * Timing can be very important: the check should take the same amount of
 * time for valid and invalid user identifiers or passwords so that an
 * attacker cannot gain knowledge about the valid users on the system
 * based on failed login attempts.
 *
 * \relates Shell
 */

/**
 * \brief Constructs a new Shell instance.
 *
 * This constructor must be followed by a call to begin() to specify
 * the underlying I/O stream.
 */
Shell::Shell()
    : maxHistory(0)
    , curLen(0)
    , history(0)
    , historyWrite(0)
    , historyPosn(0)
    , prom("> ")
    , hideChars(false)
{
}

/**
 * \brief Destroys this Shell object.
 */
Shell::~Shell()
{
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
 * Terminal::Telnet.
 * \return Returns true if the shell was initialized, or false if there
 * is insufficient memory for the history stack.
 *
 * This function will print the prompt() in preparation for entry of
 * the first command.  The default prompt is "> "; call setPrompt()
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
    // Initialize the Terminal base class with the underlying stream.
    Terminal::begin(stream, mode);

    // Create the history buffer.
    this->maxHistory = maxHistory;
    delete [] history;
    if (maxHistory) {
        history = new char [sizeof(buffer) * maxHistory];
        if (!history) {
            Terminal::end();
            return false;
        }
        memset(history, 0, sizeof(buffer) * maxHistory);
    } else {
        history = 0;
    }

    // Clear other variables.
    curLen = 0;
    historyWrite = 0;
    historyPosn = 0;
    hideChars = false;

    // Print the initial prompt.
    if (prom)
        print(prom);
    return true;
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
    delete [] history;
    maxHistory = 0;
    curLen = 0;
    history = 0;
    historyWrite = 0;
    historyPosn = 0;
    hideChars = false;
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
        executeBuiltin(builtin_cmd_exit);
        break;

    case KEY_UP_ARROW:
        // Go back one item in the command history.
        if (!hideChars && history && historyPosn < maxHistory) {
            ++historyPosn;
            changeHistory();
        }
        break;

    case KEY_DOWN_ARROW:
        // Go forward one item in the command history.
        if (!hideChars && history && historyPosn > 0) {
            --historyPosn;
            changeHistory();
        }
        break;

    case KEY_F1:
        // F1 is equivalent to the "help" command.
        executeBuiltin(builtin_cmd_help);
        break;

    case KEY_UNICODE: {
        // Add the Unicode code point to the buffer if it will fit.
        long code = unicodeKey();
        size_t size = Terminal::utf8Length(code);
        if (size && (curLen + size) < (sizeof(buffer) - 1)) {
            Terminal::utf8Format((uint8_t *)(buffer + curLen), code);
            if (!hideChars)
                write((uint8_t *)(buffer + curLen), size);
            curLen += size;
        }
    } break;

    default:
        if (key >= 0x20 && key <= 0x7E) {
            // Printable ASCII character - echo and add it to the buffer.
            if (curLen < (sizeof(buffer) - 1)) {
                if (!hideChars)
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
 * \return The current prompt.  The default is "> ".
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
 * This function must be called before begin() or the first line will be
 * prompted with the default of "> ".  Afterwards, calling this function
 * will change the prompt for the following line of input.
 *
 * \sa prompt()
 */

/**
 * \fn bool Shell::hideCharacters() const
 * \brief Determine if character echo is hidden.
 *
 * \sa setHideCharacters()
 */

/**
 * \brief Enables or disables character hiding.
 *
 * \param hide Set to true to enable character hiding, or false to disable.
 *
 * When character hiding is enabled, all characters on a command-line are
 * hidden and suppressed from being echoed.  This is useful for the entry
 * of passwords and other sensitive information.
 *
 * \sa hideCharacters()
 */
void Shell::setHideCharacters(bool hide)
{
    if (hideChars == hide)
        return;
    hideChars = hide;
    if (hide) {
        // Hide the current command if something has already been echoed.
        size_t len = curLen;
        clearCharacters(len);
        curLen = len;
    } else {
        // Print the current in-progress command to un-hide it.
        if (curLen)
            write((const uint8_t *)buffer, curLen);
    }
}

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
 * \brief Executes the command in the buffer.
 */
void Shell::execute()
{
    size_t posn = 0;

    // Terminate the current line.
    println();

    // Make sure the command is properly NUL-terminated.
    buffer[curLen] = '\0';

    // If we have a history stack and the new command is different from
    // the previous command, then copy the command into the stack.
    if (history && curLen > 0) {
        char *hist = history + sizeof(buffer) * historyWrite;
        if (strcmp(hist, buffer) != 0) {
            historyWrite = (historyWrite + 1) % maxHistory;
            hist = history + sizeof(buffer) * historyWrite;
            strcpy(hist, buffer);
        }
    }

    // Reset the history read position to the top of the stack.
    historyPosn = 0;

    // Skip white space at the start of the line.
    while (posn < curLen && buffer[posn] == ' ')
        ++posn;

    // Break the command up into arguments and populate the argv array.
    int argc = 0;
    size_t outposn = 0;
    char quote = 0;
    while (posn < curLen) {
        char ch = buffer[posn];
        if (ch == ' ') {
            ++posn;
            continue;
        }
        argv[argc++] = buffer + outposn;
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
        } while (posn < curLen);
        buffer[outposn++] = '\0';
        if (posn < curLen)
            ++posn;
    }

    // Clear the line buffer.
    curLen = 0;

    // Execute the command.
    if (argc > 0) {
        if (!execute(argc, argv)) {
            // Could not find a matching command, try the builtin "help".
            if (!strcmp_P(argv[0], builtin_cmd_help) ||
                    !strcmp_P(argv[0], builtin_cmd_help_alt)) {
                help();
            } else {
                static char const unknown_cmd[] PROGMEM = "Unknown command: ";
                writeProgMem(unknown_cmd);
                print(argv[0]);
                println();
            }
        }
    }

    // Prepare for the next command.
    if (prom)
        print(prom);
}

/**
 * \brief Executes a command that has been parsed into arguments.
 *
 * \param argc The number of elements in \a argv.
 * \param argv The arguments.
 *
 * \return Returns true if the command was found; false if not found.
 */
bool Shell::execute(int argc, char **argv)
{
    ShellCommandRegister *current = firstCmd;
    while (current != 0) {
        if (!strcmp_P(argv[0], readInfoName(current->info))) {
            ShellCommandFunc func = readInfoFunc(current->info);
            (*func)(*this, argc, argv);
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
    strncpy_P(buffer, cmd, curLen);
    write((const uint8_t *)buffer, curLen);
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
    if (hideChars)
        return;

    // Backspace over all characters in the buffer.
    while (len > 0 && curLen > 0) {
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
            memcpy(buffer, hist, curLen);
            if (!hideChars)
                write((uint8_t *)hist, curLen);
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
 * void cmdMotor(Shell &shell, int argc, char *argv[])
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
