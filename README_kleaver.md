KLEE Generating Parsers from KQuery Files
=============================

First, install `KLEE` with uclibc. Detailed instructions are [here](https://klee.github.io/getting-started/). Built against LLVM 6.0, but the Docker image should also work. Pull this repo instead of klee repo.

Take the KQuery file from running `KLEE` on program (either the KQuery file is generated automatically because there was an error or you specify the --write-kqueries option). Append [] and [{symbolic argument names}]. Then run `kleaver` on KQuery file. 

To reproduce Coreutils errors, these links are helpful: [Testing Coreutils](https://klee.github.io/tutorials/testing-coreutils/) and [Experiments](https://klee.github.io/docs/coreutils-experiments/). 

Output file name is constraint{sym_arg_name}.c
