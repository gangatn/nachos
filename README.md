NachOS - Team F
=================

Progression
-----------
- assignments
  - [x] IOs
  - [x] Multithreading
  - [ ] Virtual Memory
  - [ ] File system
  - [ ] Network
- extras
  - Fork, Exec, and ForkExec syscalls
  - ...

Building the project
-----------------

Building the project will requires a MIPS compiler.

Once the MIPS compiler installed if needed, check out the README.Makefiles
to configure the build an have an overview of the architecture

then go to the code directory, from here you can execute the following command:
$ make

This will make both our flavor and original ones.
If you want to build only our flavor type:

$ make all
or
$ make build

If you want to build only original flavor type:
$ make origin

If you are working on our flavor, it is recommended going to the build directory.

From there, upon `make` you will only build our flavors, and it will be easier to
test them.

Documentation
-------------

Project documentation is available in the `docs` folder.
Eventually you can build documents the make command
while being in this folder

You will need the excellent perl script `latexmk`