### What Is This?

Simple. It's my personal console filemanager. Like ranger, but made in c++ so faster.

### Documentation

> q

exit

> down

moves cursor down by 1

> up

moves cursor up by 1

> set {...}

sets cursor position

> get {cursor} {...}

gets command

> cd {directory}

change directory

> hidden

toggle show hidden files

> mkdir {...}

make directory

> open {directory/file}

open file or change directory

> rename {base} {target}

rename base to target

> rename {target}

rename selected to target

> rename

runs command "get -1 rename {extension}"

> brename

rename at begining of filename

> erename

rename at end of filename

> rm {...}

removes file

> rrm {...}

recursively removes file

> touch {...}

create files

> cp {base} {target}

copys base to target

> cp {target}

copys selected to target

> cp

copys to xclip clipboard

> rcp {base} {target}

recursively copy base to target

> rcp {target}

recursively copy selected to target

> rcp

recursively copy to clipboard

> paste

paste to current directory

> cpdir

copy current directory to xclip clipboard
