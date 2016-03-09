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

#ifndef TELNET_DEFS_h
#define TELNET_DEFS_h

// References:
//     https://tools.ietf.org/html/rfc854
//     http://www.iana.org/assignments/telnet-options/telnet-options.xhtml

namespace TelnetDefs
{

/** Telnet commands */
enum Command
{
    EndOfFile           = 236,  /**< EOF */
    Suspend             = 237,  /**< Suspend process */
    Abort               = 238,  /**< Abort process */
    EndOfRecord         = 239,  /**< End of record command */
    SubEnd              = 240,  /**< End option sub-negotiation */
    NOP                 = 241,  /**< No operation */
    DataMark            = 242,  /**< Data mark */
    Break               = 243,  /**< Break */
    Interrupt           = 244,  /**< Interrupt process */
    AbortOutput         = 245,  /**< Abort output */
    AreYouThere         = 246,  /**< Are you there? */
    EraseChar           = 247,  /**< Erase character */
    EraseLine           = 248,  /**< Erase line */
    GoAhead             = 249,  /**< Go ahead in half-duplex mode */
    SubStart            = 250,  /**< Option sub-negotiation */
    WILL                = 251,  /**< Will use option */
    WONT                = 252,  /**< Won't use option */
    DO                  = 253,  /**< Do use option */
    DONT                = 254,  /**< Don't use option */
    IAC                 = 255   /**< Interpret As Command */
};

/** Telnet options used in sub-negotiations */
enum Option
{
    Binary              = 0,    /**< Binary transmission */
    Echo                = 1,    /**< Echo */
    Reconnection        = 2,    /**< Reconnection */
    SuppressGoAhead     = 3,    /**< Suppress half-duplex go ahead signals */
    ApproxMsgSize       = 4,    /**< Approx message size negotiation */
    Status              = 5,    /**< Give status on prevailing options */
    TimingMark          = 6,    /**< Timing mark */
    RemoteTransmitEcho  = 7,    /**< Remote controlled transmit and echo */
    LineWidth           = 8,    /**< Line width */
    PageSize            = 9,    /**< Page size */
    CarriageReturn      = 10,   /**< Carriage return disposition */
    HorzTabStops        = 11,   /**< Horizontal tab stops */
    HorzTabStopDisp     = 12,   /**< Horizontal tab stop disposition */
    FormFeed            = 13,   /**< Form feed disposition */
    VertTabStops        = 14,   /**< Vertical tab stops */
    VertTabStopDisp     = 15,   /**< Vertical tab stop disposition */
    LineFeed            = 16,   /**< Line feed disposition */
    ExtendedASCII       = 17,   /**< Extended ASCII */
    Logout              = 18,   /**< Force logout */
    ByteMacro           = 19,   /**< Byte macro */
    DataEntryTerminal   = 20,   /**< Data entry terminal */
    SUPDUP              = 21,   /**< SUPDUP protocol */
    SUPDUPOutput        = 22,   /**< SUPDUP output */
    SendLocation        = 23,   /**< Send the user's location */
    TerminalType        = 24,   /**< Terminal type */
    EndOfRecordOption   = 25,   /**< End of record option */
    TACACSUserId        = 26,   /**< TACACS user identification */
    OutputMarking       = 27,   /**< Output marking */
    TerminalLocation    = 28,   /**< Terminal location number */
    Telnet3270Regime    = 29,   /**< Telnet 3270 regime */
    X3Pad               = 30,   /**< X.3 PAD */
    WindowSize          = 31,   /**< Window size */
    Speed               = 32,   /**< Terminal speed */
    RemoteFlowControl   = 33,   /**< Remote flow control */
    Linemode            = 34,   /**< Linemode option */
    XDisplay            = 35,   /**< X display location */
    EnvironmentOld      = 36,   /**< Environment variables (old version) */
    Authentication      = 37,   /**< Authentication */
    Encryption          = 38,   /**< Encryption */
    Environment         = 39,   /**< Environment variables (new version) */
    Extended            = 255   /**< Extended options list */
};

};

#endif
