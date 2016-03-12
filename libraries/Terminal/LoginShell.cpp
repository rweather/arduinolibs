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

#include "LoginShell.h"

/**
 * \class LoginShell LoginShell.h <LoginShell.h>
 * \brief Command-line shell access via a login shell.
 *
 * This class provides a command-line shell with login support.
 * The user is prompted for username and password when they connect
 * and other commands will not be available until the correct credentials
 * have been supplied.
 *
 * \sa Shell
 */

/**
 * \typedef ShellPasswordCheckFunc
 * \brief Password checking function for login shells.
 *
 * \param username Points to the user name that was supplied at login.
 * \param password Points to the password that was supplied at login.
 *
 * \return Returns zero or greater if the username and password combination
 * is correct, negative if incorrect.
 *
 * The return value is reported to the application as LoginShell::userid(),
 * which can be used by the application to restrict the set of commands
 * that are available to the user, or to restrict the behaviour of
 * those commands when acting on critical resources.
 *
 * Timing can be very important: the check should take the same amount of
 * time for valid and invalid user identifiers or passwords so that an
 * attacker cannot gain knowledge about the valid users on the system
 * based on failed login attempts.
 *
 * \relates LoginShell
 * \sa LoginShell::userid()
 */

/**
 * \brief Constructs a new login shell.
 *
 * This constructor must be followed by a call to begin() to specify
 * the underlying I/O stream.
 */
LoginShell::LoginShell()
    : machName(0)
    , checkFunc(0)
    , uid(-1)
{
}

/**
 * \brief Destroys this login shell.
 */
LoginShell::~LoginShell()
{
}

/**
 * \fn int LoginShell::userid() const
 * \brief Gets the user identifier for the currently logged in user,
 * or -1 if there is no user logged in currently.
 *
 * The user identifier can be used by applications to restrict the set of
 * commands that are available to the user, or to restrict the behaviour
 * of those commands when acting on critical resources.
 *
 * \sa ShellPasswordCheckFunc
 */
