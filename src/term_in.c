///
///  @file    term_in.c
///  @brief   System-independent functions to input from user's terminal.
///
///  @bug     No known bugs.
///
///  @copyright  2019-2020 Franklin P. Johnston
///
///  Permission is hereby granted, free of charge, to any person obtaining a copy
///  of this software and associated documentation files (the "Software"), to deal
///  in the Software without restriction, including without limitation the rights
///  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
///  copies of the Software, and to permit persons to whom the Software is
///  furnished to do so, subject to the following conditions:
///
///  The above copyright notice and this permission notice shall be included in
///  all copies or substantial portions of the Software.
///
///  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
///  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
///  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
///  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
///  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
///  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
///
////////////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "teco.h"
#include "ascii.h"
#include "eflags.h"
#include "errors.h"
#include "qreg.h"


#define FF_LINES    40                  ///< No. of lines to print for FF

#define VT_LINES    4                   ///< No. of lines to print for VT

int last_in = EOF;                      ///< Last character read


// Local functions

static void read_bs(void);

static void read_cr(void);

static void read_ctrl_c(int last);

static void read_ctrl_g(void);

static void read_ctrl_u(void);

static void read_ctrl_z(void);

static void read_ff(void);

static int read_first(void);

static void read_lf(void);

static void read_qname(int c);

static void read_vt(void);


///
///  @brief    Process backspace.
///
///  @returns  Nothing.
///
////////////////////////////////////////////////////////////////////////////////

static void read_bs(void)
{
    if (empty_buf())                    // In immediate mode?
    {
        print_echo(CRLF);
        print_prompt();

        if (f.et.scope)
        {
            //ZScrOp(SCR_EEL);          // TODO: write this function
        }

        // TODO: finish the following code

//        HowFar = Ln2Chr(-1L);
//        GapBeg += HowFar;
//        GapEnd += HowFar;
//        memmove(GapEnd + 1, GapBeg, (SIZE_T) - HowFar);

        if (f.ev)
        {
            //DoEvEs(EvFlag);
        }
        else
        {
            //TypBuf(GapEnd + 1, GapEnd + Ln2Chr((LONG)1) + 1);
        }
    }
    else
    {
        (void)delete_buf();
        print_echo(BS);
        print_echo(SPACE);
        print_echo(BS);
    }
}


///
///  @brief    Read next character from terminal.
///
///  @returns  Nothing.
///
////////////////////////////////////////////////////////////////////////////////

void read_cmd(void)
{
    int c;

    if (empty_buf())                    // If nothing in command string,
    {
        c = read_first();               // check for immediate-mode commands
        reset_buf();
    }
    else
    {
        c = getc_term((bool)WAIT);      // else read another character
    }

    for (;;)
    {
        // If the character is an accent grave and and the et.accent bit is set,
        // or it matches a non-NUL EE flag, then treat it as an ESCape.

        if ((f.et.accent && c == ACCENT) || f.ee == c)
        {
            echo_in(ACCENT);           // Echo as accent grave
            store_buf(c = ESC);         // But store as ESCape

            if (last_in == ESC)         // Consecutive delimiter?
            {
                print_echo(CRLF);

                last_in = EOF;

                return;                 // Yes, time to execute command
            }
        }
        else
        {
            switch (c)
            {
                case BS:
                    read_bs();

                    break;

                case FF:
                    read_ff();

                    break;

                case CR:
                    read_cr();

                    break;

                case CTRL_C:
                    read_ctrl_c(last_in);

                    break;

                case CTRL_G:
                    read_ctrl_g();

                    break;

                case CTRL_U:
                    read_ctrl_u();

                    break;

                case CTRL_Z:
                    read_ctrl_z();

                    break;

                case ESC:
                    echo_in('$');              // Use dollar sign to echo ESC
                    store_buf(c);

                    if (last_in == ESC)
                    {
                        last_in = EOF;

                        print_echo(CRLF);

                        return;                 // Done reading command
                    }

                    break;

                case LF:
                    read_lf();

                    break;

                case VT:
                    read_vt();

                    break;

                default:
                    if (!f.et.lower)
                    {
                        c = toupper(c);
                    }

                    echo_in(c);
                    store_buf(c);

                    break;
            }
        }

        last_in = c;

        c = getc_term((bool)WAIT);
    }
}


///
///  @brief    Process carriage return.
///
///  @returns  Nothing.
///
////////////////////////////////////////////////////////////////////////////////

static void read_cr(void)
{
    print_echo(CR);

    store_buf(CR);
}


///
///  @brief    Process CTRL/C.
///
///  @returns  Nothing.
///
////////////////////////////////////////////////////////////////////////////////

static void read_ctrl_c(int last)
{
    echo_in(CTRL_C);
    store_buf(CTRL_C);
    print_echo(CRLF);

    if (last == CTRL_C)                 // Second CTRL/C?
    {
        exit(EXIT_SUCCESS);             // Yes: clean up, reset, and exit
    }

    reset_buf();
    print_prompt();
}


///
///  @brief    Process CTRL/G, looking for ^G^G, ^G{SPACE}, and ^G*.
///
///  @returns  Nothing.
///
////////////////////////////////////////////////////////////////////////////////

static void read_ctrl_g(void)
{
    echo_in(CTRL_G);
    store_buf(CTRL_G);

    int c = getc_term((bool)WAIT);      // Get next character

    echo_in(c);                        // Echo it

    if (c != CTRL_G && c != SPACE && c != '*')
    {
        store_buf(c);                     // Regular character, so just store it

        return;
    }

    // Here when we have a special CTRL/G command

    print_echo(CRLF);                       // Start new line
    (void)delete_buf();                 // Delete CTRL/G in buffer

    if (c == CTRL_G)                    // ^G^G
    {
        put_bell();
        reset_buf();
        print_prompt();
    }
    else if (c == SPACE)                // ^G<SPACE>
    {
        if (empty_buf())                // Printing from beginning of buffer?
        {
            print_prompt();             // Yes, so output prompt
        }

        echo_buf((int)start_buf());
    }
    else /* if (c == '*') */            // ^G*
    {
        if (empty_buf())                // Printing from beginning of buffer?
        {
            print_prompt();             // Yes, so output prompt
        }

        echo_buf(0);
    }
}


///
///  @brief    Process CTRL/U.
///
///  @returns  Nothing.
///
////////////////////////////////////////////////////////////////////////////////

static void read_ctrl_u(void)
{
    int c;

    while ((c = delete_buf()) != EOF)
    {
        if (c == LF)
        {
            store_buf(c);               // Add line terminator back

            break;
        }
    }

    if (f.et.scope)
    {
        print_echo(CR);

        // TODO: finish this
        // ZScrOp(SCR_EEL);             // erase line
    }
    else
    {
        echo_in(CTRL_U);
        print_echo(CRLF);
    }

    reset_buf();
    print_prompt();
}


///
///  @brief    Process input CTRL/Z.
///
///  @returns  Nothing.
///
////////////////////////////////////////////////////////////////////////////////

static void read_ctrl_z(void)
{
    echo_in(CTRL_Z);
    store_buf(CTRL_Z);

    int c = getc_term((bool)WAIT);

    echo_in(CTRL_Z);

    if (c == CTRL_Z)                    // Two CTRL/Z's?
    {
        exit(EXIT_SUCCESS);             // Clean up, reset, and exit
    }

    store_buf(c);                       // Normal character
}


///
///  @brief    Process input form feed.
///
///  @returns  Nothing.
///
////////////////////////////////////////////////////////////////////////////////

static void read_ff(void)
{
    print_echo(CR);

    for (int i = 0; i < FF_LINES; ++i)
    {
        print_echo(LF);
    }

    store_buf(FF);
}


///
///  @brief    Read first character from command string. This is required if
///            special handling is needed, especially for the following immedi-
///            ate-mode commands, which are processed without destroying the
///            previous command buffer:
///
///            ?    Display previous command string up to erroneous command
///            /    Display verbose explanation of last error
///            *{q} Copy last command string to q-register {q}.
///
///            Once we return, the previous command string and error are con-
///            sidered lost. Our caller then overwrites the old command string
///            with a new one.
///
///            Note that although LF and BS are immediate-mode commands, we do
///            not deal with them here to avoid a situation such as a LF or BS
///            being typed following a CTRL/U used to delete a command string.
///            For this reason, these commands must be handled by our caller.
///
///  @returns  Nothing.
///
////////////////////////////////////////////////////////////////////////////////

static int read_first(void)
{
    // Loop until we seen something other than an immediate-mode command.

    for (;;)
    {
        if (f.ev)                       // Is edit verify flag set?
        {
            // DoEvEs(EvFlag);          // TODO: TBD!
        }

        print_prompt();

        int c = getc_term((bool)WAIT);

        switch (c)
        {
            case DEL:
            case CTRL_U:
                print_echo(CR);

                break;

            case '/':                   // Display verbose error message
                print_echo(c);

                if (last_error != E_NUL)
                {
                    print_echo(CRLF);
                    help_err(last_error);
                }

                break;

            case '?':                   // Display erroneous command string
                if (last_error != E_NUL)
                {
                    print_echo(c);
                    echo_buf(0);        // Echo command line
                    print_echo(c);
                }

                print_echo(CRLF);

                break;

            case '*':                   // Store last command in Q-register
                print_echo(c);
                c = getc_term((bool)WAIT);    // Get Q-register name

                if (f.e0.ctrl_c)
                {
                    f.e0.ctrl_c = false;

                    return CTRL_C;
                }

                read_qname(c);

                break;

            default:
                return c;
        }
    }
}


///
///  @brief    Process input line feed.
///
///  @returns  Nothing.
///
////////////////////////////////////////////////////////////////////////////////

static void read_lf(void)
{
    if (empty_buf())                    // Immediate mode?
    {
        print_echo(CRLF);
        print_prompt();

        if (f.et.scope)
        {
            //ZScrOp(SCR_EEL);
        }

#if     0                               // TODO: finish this
        HowFar = Ln2Chr((LONG)1);
        MEMMOVE(GapBeg, GapEnd + 1, (SIZE_T)HowFar);
        GapBeg += HowFar;
        GapEnd += HowFar;
#endif

        if (f.ev)
        {
            //DoEvEs(EvFlag);
        }
        else
        {
            //TypBuf(GapEnd + 1, GapEnd + Ln2Chr((LONG)1) + 1);
        }
    }
    else
    {
        print_echo(LF);
        store_buf(LF);
    }
}


///
///  @brief    Get Q-register name and store command string.
///
///  @returns  Nothing.
///
////////////////////////////////////////////////////////////////////////////////

static void read_qname(int c)
{
    if (c == CR)
    {
        return;
    }
    else if (c == BS || c == DEL || c == CTRL_U)
    {
        print_echo(CR);

        return;
    }

    int qname = c;
    bool qdot = (qname == '.');

    if (qdot)                           // Local Q-register?
    {
        echo_in(c);                    // Yes, echo the dot

        qname = getc_term((bool)WAIT);  // And get next character
    }

    echo_in(qname);                    // Echo Q-register name

    print_echo(CRLF);

    store_qtext(qname, qdot, copy_buf());
}


///
///  @brief    Process input vertical tab.
///
///  @returns  Nothing.
///
////////////////////////////////////////////////////////////////////////////////

static void read_vt(void)
{
    print_echo(CR);

    for (int i = 0; i < VT_LINES; ++i)
    {
        print_echo(LF);
    }

    store_buf(VT);
}
