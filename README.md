# ush
A basic command-line interpreter with features without which there can be no shell.\
The shell:\
  • have the default prompt must look like u$h>, followed by a space character\
  • deal only with one line user input.\
    In other cases, the appropriate descriptive error message must be displayed\
  • implement builtin commands without flags: export, unset, exit, fg\
  • implement the following builtin commands with flags:\
    – env with -i , -P , -u\
    – cd with -s , -P and - argument\
    – pwd with -L , -P\
    – which with -a , -s\
    – echo with -n , -e , -E\
  • find builtins or flags in manuals to other shells if zsh hasn't got them\
  • call the builtin command instead of the binary program if there is a name match among them\
  • correctly manage errors\
  • manage user environment correctly\
  • run programs located in the directories listed in the PATH variable\
  • manage signals CTRL+D , CTRL+C and CTRL+Z\
  • implement the command separator ;\
  • manage these expansions correctly:\
    – the tilde expansion ~ with the following tilde-prefixes:\
      ~, ~/dir_name, ~username/dir_name , ~+/dir_name, ~-/dir_name\
    – the basic form of parameter expansion ${parameter}\
    – the two-level-nested command substitution $(command)
