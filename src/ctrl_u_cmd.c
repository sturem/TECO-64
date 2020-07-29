///
///  @file    ctrl_u_cmd.c
///  @brief   Execute ^U (CTRL/U) command.
///
///  @bug     No known bugs.
///
///  @copyright  2019-2020 Franklin P. Johnston
///
///  Permission is hereby granted, free of charge, to any person obtaining a
///  copy of this software and associated documentation files (the "Software"),
///  to deal in the Software without restriction, including without limitation
///  the rights to use, copy, modify, merge, publish, distribute, sublicense,
///  and/or sell copies of the Software, and to permit persons to whom the
///  Software is furnished to do so, subject to the following conditions:
///
///  The above copyright notice and this permission notice shall be included in
///  all copies or substantial portions of the Software.
///
///  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
///  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
///  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
///  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIA-
///  BILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
///  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
///  THE SOFTWARE.
///
////////////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "teco.h"
#include "errors.h"
#include "exec.h"
#include "qreg.h"


///
///  @brief    Execute ^U (CTRL/U) command: store/append string/character in
///            Q-register.
///
///  @returns  Nothing.
///
////////////////////////////////////////////////////////////////////////////////

void exec_ctrl_U(struct cmd *cmd)
{
    assert(cmd != NULL);

    if (cmd->n_set)                     // n^Uq`?
    {
        if (cmd->text1.len != 0)        // Does it have a text string?
        {
            throw(E_MOD);               // Yes, that's an error
        }

        if (cmd->colon)                 // n:^Uq`?
        {
            append_qchr(cmd->qname, cmd->qlocal, cmd->n_arg);
        }
        else                            // n^Uq`
        {
            store_qchr(cmd->qname, cmd->qlocal, cmd->n_arg);
        }
    }
    else                                // No n argument
    {
        if (cmd->text1.len == 0)        // No text either?
        {
            throw(E_MOD);               // Yes, that's wrong
        }

        if (cmd->colon)                 // :^Utext`
        {
            const char *p = cmd->text1.buf;

            for (uint i = 0; i < cmd->text1.len; ++i)
            {
                append_qchr(cmd->qname, cmd->qlocal, *p++);
            }
        }
        else                            // ^Uqtext`
        {
            struct buffer text;

            text.len  = cmd->text1.len;
            text.pos  = 0;
            text.size = cmd->text1.len;
            text.buf  = alloc_mem(text.size);

            memcpy(text.buf, cmd->text1.buf, (ulong)text.size);

            store_qtext(cmd->qname, cmd->qlocal, &text);
        }
    }
}

