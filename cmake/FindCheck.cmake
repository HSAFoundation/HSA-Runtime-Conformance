if (CHECK_INCLUDE_DIR)
  ## The check information is already in the cache.
  set (CHECK_FIND_QUIETLY TRUE)
endif (CHECK_INCLUDE_DIR)

## If the CHECK_INSTALL_DIR cmake variable is set,
## add it to the list of system directories to search
## for the check library. This is done in the find_path
## and find_library calls below.

## Look for the check include file.
find_path (CHECK_INCLUDE_DIR NAMES check.h ${CHECK_INSTALL_DIR}/include)
## Look for the check library.
find_library (CHECK_LIBRARY NAMES check ${CHECK_INSTALL_DIR}/lib)

## Handle the QUIETLY and REQUIRED arguments and set CHECK_FOUND to TRUE if
## all listed variables are TRUE.
include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (Check "Please install 'check' and 'check-devel' packages" CHECK_LIBRARY CHECK_INCLUDE_DIR)

if (CHECK_FOUND)
  set (CHECK_LIBRARIES ${CHECK_LIBRARY})
else (CHECK_FOUND)
  set (CHECK_LIBRARIES)
endif(CHECK_FOUND)

mark_as_advanced (CHECK_INCLUDE_DIR)
mark_as_advanced (CHECK_LIBRARY)
