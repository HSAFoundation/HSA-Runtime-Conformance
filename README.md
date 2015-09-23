USING THE RUNTIME CONFORMANCE SUITE

The HSA Runtime Conformance Suite build environment utilizes the cmake 
for automatic makefile generation targeting specific platforms.
The required version of cmake is version 2.8. The build & execution
environment requires the check test framework, version 0.9.12 or later,
and the 1.0 Final HSA runtime.

In normal builds, CMake automatically determines the toolchain for host
builds based on system introspection and defaults. In cross-compiling
scenarios, a toolchain file may be specified with information about compiler
and utility paths. Please consult CMake documentation for more information.

Linux Environment Setup

The following steps will install the appropriate versions of
cmake, check and HSA in the default PATH and LD_LIBRARY_PATH directories.
Consult the appropriate documentation regarding installing the
executables and libraries in different locations.

    1) Install the appropriate level of cmake on your system. The latest version
       of cmake can be obtained from http://www.cmake.org/download/, where both
       binary and source distributions are available. On ubuntu the cmake package
       may come as a pre-installed package, but the following command will install
       the default version for the current system:

       'sudo apt-get install cmake'

     2) Install the appropriate version of the check framework on the
        build system. The check framework can be downloaded from
        http://sourceforge.net/projects/check/files/latest/download. On ubuntu the
        check test framework will install the default version on the
        current system:

       'sudo apt-get install check'

     3) If the HSA runtime isn't installed, get the desired version of the runtime
        from https://github.com/HSAFoundation/HSA-Runtime-AMD. Install the runtime
        from either the fedora rpm or the ubuntu deb package provided in the repository.
        This will install the runtime in the /opt/hsa directory.

Windows Environment Setup

The Runtime conformance suite can only be built on Windows if Cygwin development
environment is available. To install the required Cygwin resources execute the
link https://cygwin.com/setup-x86_64.exe. This will download the Cygwin installer
to the local system. Launch the Cygwin installer and install the entire Devel
category. Not all of the packages in Devel are required for Runtime conformance,
but it is safer and easier to install them all than to manually select the (several)
required components. 

BUILDING THE CONFORMANCE SUITE

Before building the suite the locations of the HSA libraries and header files
must be specified using the HSA_INCLUDE_DIR and the HSA_LIBRARY_DIR cmake
variables. This is only required the HSA headers and libraries aren't installed
in the standard system directories or in the default /opt/hsa location. For
example, if HSA is installed for a local user, the cmake variables should
be specified on the cmake command line like this:

    `cmake -D HSA_INCLUDE_DIR:STRING=/home/<user>/hsa/include 
           -D  HSA_LIBRARY_DIR=/home/<user>/hsa/lib CMakeLists.txt`

On a Windows system the cmake command should be executed in a Cygwin64 terminal shell.
Also on Windows, the HSA_INCLUDE_DIR and HSA_LIBRARY_DIR cmake variables must
be set; there are no valid default locations. The HSA_LIBRARY_DIR variable
should point to the directory that contains the hsa-runtime64.dll file, not the
associated hsa-runtime64.lib file.

To build the suite, create a build directory and run cmake on the CMakeLists.txt
file. After the make infrastructure is created, build the binaries with the 
make command. The following sequence of commands, if run from the top level
directory, would build the conformance suite:

     `mkdir build && cd build && cmake <-D ...> .. && make`

INSTALLING THE CONFORMANCE SUITE

The conformance suite can be installed in a directory by issuing the `make install`
command. The default installation directory is '/usr/local/hsa_conformance'. This
can be changed to another location using the `cmake -D CMAKE_INSTALL_PREFIX=<dir> ..`
option.

The user must have proper access to the install directory to both install
and execute the tests.

RUNNING THE TESTS USING CMAKE

Before running the tests the LD_LIBRARY_PATH environment variable must include
the PATH of the HSA runtime libraries.

The HSA conformance tests can use the ctest execution environment. After build the
conformance test all currently enabled tests can be run by building the 'test'
target:

     `make test`

RUNNING THE TESTS USING EXECUTION SCRIPTS

It is also possible to use the execute.sh script provided in the installation to
run the test suite. First, install the test suite:

     `make install`

This will transfer all of the tests, brig files and execution scripts to the install
directory (/usr/local/hsa_conformance is the default install directory). Change to
the install directory and run the following command to execute the conformance suite:

     `execute.sh test.lst`

FREQUENTLY ASKED QUESTIONS

	Q1: When debugging a test case with gdb I can't step into the test functions? How do
	I enable debugging?
	A1: By default the check test framework will fork a new process for each test and
	determines if it passes or fails by the code returned to the parent process. This allows
	the parent process to remain insulated from signals (SIGSEGV) that are sent to the
	child process. On most systems, GDB has no special support for debugging programs 
	which create additional processes using the fork function. When a program forks, GDB will 
	continue to debug the parent process and the child process will run unimpeded. This forking
	behavior can be turned of by setting the CK_FORK environment varialble to 'no', e.g.
	"export CK_FORK=no".

        Q2: Check appears to be generating a segfault when I run a test:
            ../../src/check_pack.c:312: ../../src/check_msg.c:75: No messaging setup Segmentation fault (core dumped)

        A2: The check assertion system isn't particularly robust when it comes to multiple threads, especially if
        CK_FORK=no is set. This segfault usually indicates that a multi-threaded test case is failing and 
        several of the threads are generating an ASSERT message concurrently.
