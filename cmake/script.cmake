## Specify the SCRIPT_DIR.
set (SCRIPT_DIR "${CMAKE_SOURCE_DIR}/script")

## Install the run.sh script in the build directory for `make test` support
configure_file(${SCRIPT_DIR}/run.sh run.sh COPYONLY)
## Install the execute.sh script in the build directory for direct execution 
configure_file(${SCRIPT_DIR}/execute.sh execute.sh COPYONLY)

## Install the execute.sh script for traditional execute support
install(PROGRAMS ${SCRIPT_DIR}/execute.sh DESTINATION ${INSTALL_DIR})
