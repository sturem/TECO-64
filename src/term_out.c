///
///  @file    term_out.c
///  @brief   System-independent functions to output to user's terminal.
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
#include <string.h>

#include "teco.h"
#include "ascii.h"
#include "eflags.h"
#include "errors.h"
#include "exec.h"

uint ncommands = 0;                     ///< No. of commands seen so far


///
///  @brief    Echo character in a printable form, either as c, ^c, or [c].
///
///  @returns  Nothing.
///
////////////////////////////////////////////////////////////////////////////////

void echo_chr(int c)
{
    if (c == ESC)
    {
        if (f.et.accent)
        {
            c = '`';
        }
        else if (f.ee != NUL)
        {
            c = f.ee;
        }
    }

    if (isprint(c))
    {
        putc_term(c);
    }
    else if (!isascii(c))               // 8-bit character?
    {
        if (f.et.eightbit)              // Can terminal display it?
        {
            putc_term(c);               // Yes
        }
        else                            // No, make it printable
        {
            char chrbuf[5];             // [xx] + NUL

            uint nbytes = (uint)snprintf(chrbuf, sizeof(chrbuf), "[%02x]", c);

            assert(nbytes < sizeof(chrbuf));
        }
    }
    else                                // Must be a control character
    {
        switch (c)
        {
            case BS:                    // TODO: is this correct?
            case TAB:
            case LF:
            case CR:
                putc_term(c);

                break;

            case DEL:
                break;

            case ESC:
                putc_term('$');

                break;

            case FF:
                putc_term('\r');
                //lint -fallthrough

            case VT:
                putc_term('\n');
                putc_term('\n');
                putc_term('\n');
                putc_term('\n');

                break;

            case CTRL_G:
                putc_term(CTRL_G);

                //lint -fallthrough

            default:                    // Display as +^c
                putc_term('^');
                putc_term(c + 'A' - 1);

                break;
         }
    }
}


///
///  @brief    Process HELP command (TBD).
///
///  @returns  Returns true if we have a HELP command, else false.
///
////////////////////////////////////////////////////////////////////////////////

bool help_command(void)
{
    bool match = match_buf("HELP");

    if (!match)
    {
        return false;
    }

    f.ei.lf = true;                     // Discard next chr. if LF

    putc_term(CRLF);

    print_err(E_NYI);                   // TODO: temporary!
}


///
///  @brief    Print the command we just parsed.
///
///  @returns  Nothing.
///
////////////////////////////////////////////////////////////////////////////////

void print_cmd(struct cmd *cmd)
{
    assert(cmd != NULL);

    // Here when we've parsed the entire command - now type it out.

    printf("[%u,%u] %*s", ++ncommands, cmd->level, (int)cmd->level * 4, " ");
    (void)fflush(stdout);

    if (cmd->expr.len != 0)
    {
        const char *p = cmd->expr.buf;

        putc_term('{');

        while (p < cmd->expr.buf + cmd->expr.len)
        {
            echo_chr(*p++);
        }

        putc_term('}');
        putc_term(SPACE);
    }

    if (cmd->c1 == ESC)
    {
        echo_chr('$');
        putc_term(CRLF);

        return;
    }

    if (cmd->colon_set || cmd->dcolon_set)
    {
        putc_term('{');

        echo_chr(':');

        if (cmd->dcolon_set)
        {
            echo_chr(':');
        }        

        putc_term('}');
        putc_term(SPACE);
    }

    if (cmd->atsign_set)
    {
        putc_term('{');

        echo_chr('@');

        putc_term('}');
        putc_term(SPACE);
    }

    echo_chr(cmd->c1);

    if (cmd->c2 != NUL)
    {
        echo_chr(cmd->c2);

        if (cmd->c3 != NUL)
        {
            echo_chr(cmd->c3);
        }
    }

    if (cmd->q_req)                     // Do we have a Q-register name?
    {
        if (cmd->qlocal)                // Yes, is it local?
        {
            echo_chr('.');
        }

        if (isprint(cmd->qreg))
        {
            echo_chr(cmd->qreg);
        }
        else
        {
            echo_chr('?');
        }
    }

    putc_term(SPACE);

    if (cmd->text1.len != 0)
    {
        if (cmd->atsign_set)            // Conditionally echo delimiter before 1st arg.
        {
            echo_chr(cmd->delim);
            putc_term(SPACE);
        }

        putc_term('{');

        const char *p = cmd->text1.buf;

        while (p < cmd->text1.buf + cmd->text1.len)
        {
            if (isprint(*p) || toupper(cmd->c1) == 'I' || cmd->c1 == TAB)
            {
                echo_chr(*p);
            }
            ++p;
        }

        putc_term('}');
        putc_term(SPACE);
    }

    if (cmd->text2.len != 0 || cmd->delim != ESC)
    {
        echo_chr(cmd->delim);           // Echo delimiter between texts
        putc_term(SPACE);
    }

    if (cmd->text2.len != 0)
    {
        putc_term('{');

        const char *p = cmd->text2.buf;

        while (p < cmd->text2.buf + cmd->text2.len)
        {
            if (isprint(*p))
            {
                echo_chr(*p);
            }
            ++p;
        }

        putc_term('}');
        putc_term(SPACE);

        if (cmd->delim != ESC)
        {
            echo_chr(cmd->delim);       // Echo delimiter at end
        }
    }

    putc_term(CRLF);
}
