if (ELF_INCLUDE_DIR)
    ## The elf information is already in the cache.
    set (ELF_FIND_QUIETLY TRUE)
endif (ELF_INCLUDE_DIR)

## Look for the check include file.
find_path (ELF_INCLUDE_DIR NAMES libelf.h)
## Look for the check library.
find_library (ELF_LIBRARY NAMES elf)

## Handle the QUIETLY and REQUIRED arguments and set ELF_FOUND to TRUE if
## all listed variables are TRUE.
include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (ELF "Please install the 'elfutils' package" ELF_LIBRARY ELF_INCLUDE_DIR)

if (ELF_FOUND)
    set (ELF_LIBRARIES ${ELF_LIBRARY})
else (ELF_FOUND)
    set (ELF_LIBRARIES)
endif(ELF_FOUND)

mark_as_advanced (ELF_INCLUDE_DIR)
mark_as_advanced (ELF_LIBRARY)
