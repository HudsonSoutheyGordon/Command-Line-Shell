# Command-Line-Shell in C
---

Smallsh is a toy shell program built in C that implements a subset of features of the common shell, bash.
Commands can be inputted with the following syntax:
`command [arg1 arg2 ... argn] [< input_file] [> output_file] [&]`

Any command beginning with `#` is ignored. This is useful for putting comments in a shell script (see the test script file.)
Additionally, `$$` can be used to expand into the PID of the shell itself.

Input and Output redirection (`<` and `>` respectively) must come after an all arguments in the command.

Finally, commands can be executed in the background if the last char (after a space) in the command is `&`. The command will then run in the background and provide the process's PID.

Smallsh has three builtin commands: `exit`, `cd`, and `status`. Any other command that is supported by bash will work with Smallsh, and is implemented via the use of `fork()` and `exec()`.

Demo Video on YouTube
---
<a href="http://www.youtube.com/watch?feature=player_embedded&v=kxM7Z85rT40
" target="_blank"><img src="http://img.youtube.com/vi/kxM7Z85rT40/0.jpg" 
alt="Smallsh" width="800" height="450" border="10" /></a>
