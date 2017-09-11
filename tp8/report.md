# SGM - TP8 (Louis Béziaud & Simon Bihel)

When `sortf` is launched a second time, the file still is in `g_open_table`. This has no impact to evaluate the `mmap`
as closing a file only deletes or decrement various objects. No writing is involved.

We cannot close the file properly at the death of process because we don't have access to the filename from `AddrSpace`.
We cannot close all entries of the `OpenFileTable` because it is shared between all processes in nachos.
