# Tasks

In no specific order, these are the tasks that are outstanding

* Simple type checking on objects internally. Is that object we're getting out of `msg_eval_arg_at()` actually a block?
* CMake precompiled header integration. We depend on Clang and to have clang generate a precompiled header, we use the following: `clang -cc1 test.h -emit-pch -o test.h.pch`. Candidates for inclusion include:
* * branch_prediction.h
