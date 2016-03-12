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

#ifndef LOGIN_SHELL_h
#define LOGIN_SHELL_h

#include "Shell.h"

typedef int (*ShellPasswordCheckFunc)(const char *username, const char *password);

class LoginShell : public Shell
{
public:
    LoginShell();
    virtual ~LoginShell();

    const char *machineName() const { return machName; }
    void setMachineName(const char *machineName) { machName = machineName; }

    ShellPasswordCheckFunc passwordCheckFunction() const { return checkFunc; }
    void setPasswordCheckFunction(ShellPasswordCheckFunc function) { checkFunc = function; }

    int userid() const { return uid; }

protected:
    virtual void beginSession();
    virtual void printPrompt();
    virtual void execute();

private:
    const char *machName;
    ShellPasswordCheckFunc checkFunc;
    int uid;
};

#endif
