///
///  @file    eg_cmd.c
///  @brief   Execute EG command.
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
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "teco.h"
#include "editbuf.h"
#include "errors.h"
#include "estack.h"
#include "exec.h"
#include "file.h"


char eg_command[PATH_MAX];              ///< EG command to be executed on exit.


///
///  @brief    Execute EG command: execute system command.
///
///  @returns  Nothing.
///
////////////////////////////////////////////////////////////////////////////////

void exec_EG(struct cmd *cmd)
{
    assert(cmd != NULL);

    if (cmd->text1.len > sizeof(eg_command) - 1)
    {
        print_err(E_EGC);               // EG command is too long
    }
    
    if (cmd->colon_set || cmd->dcolon_set)
    {
        char command[PATH_MAX];         // Local command

        sprintf(command, "%.*s", (int)cmd->text1.len, cmd->text1.buf);

        int status = find_eg(command, cmd->dcolon_set);

        push_expr(status, EXPR_VALUE);

        return;
    }

    sprintf(eg_command, "%.*s", (int)cmd->text1.len, cmd->text1.buf);

    // The following ensures that we don't exit if we have nowhere to output
    // the data in the buffer to.

    struct ofile *ofile = &ofiles[ostream];

    if (ofile->fp == NULL && t.Z != 0)
    {
        print_err(E_NFO);               // No file for output
    }

    exec_EC(cmd);

    // EG`, not :EG`, so get ready to exit

    exit(EXIT_SUCCESS);
}


///
///  @brief    Initialize check that will be called to see if we need to execute
///            an EG command before we exit. We do it this way to ensure that
///            we've handled all possible cleanup.
///
///  @returns  Nothing.
///
////////////////////////////////////////////////////////////////////////////////

void init_EG(void)
{
    register_exit(exit_EG);
}
