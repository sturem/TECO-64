///
///  @file    s_cmd.c
///  @brief   Execute S command.
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "teco.h"
#include "errors.h"
#include "exec.h"

// TODO: fix magic number

char search_string[128] = { '\0' };     ///< last search string


///
///  @brief    Execute S command (local search).
///
///  @returns  Nothing.
///
////////////////////////////////////////////////////////////////////////////////

void exec_S(struct cmd *cmd)
{
    assert(cmd != NULL);

    if (cmd->n_set && cmd->n_arg == 0)  // 0Stext` isn't allowed
    {
        print_err(E_ISA);               // Illegal search argument
    }

    if (cmd->dcolon_set)                // ::Stext` => 1,1:Stext`
    {
        cmd->m_arg = cmd->n_arg = 1;
        cmd->m_set = cmd->n_set = true;
    }
    else if (!cmd->n_set)               // Stext` => 1Stext`
    {
        cmd->n_arg = 1;
        cmd->n_set = true;
    }

    if (cmd->text1.len != 0)
    {
        sprintf(search_string, "%.*s", (int)cmd->text1.len, cmd->text1.buf);
    }

    print_str("%s search for occurrence #%u of \"%s\"\r\n",
           cmd->n_arg < 0 ? "backward" : "forward", (uint)abs(cmd->n_arg),
           search_string);

    if (cmd->m_set)
    {
        print_str("    limit search to %d characters\r\n", abs(cmd->m_arg) - 1);
    }

    if (cmd->colon_set)
    {
        print_str("    return success or failure\r\n");
    }
}

