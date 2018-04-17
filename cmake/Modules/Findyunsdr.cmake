INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(PC_YUNSDR gnuradio-yunsdr)

FIND_PATH(
	YUNSDR_INCLUDE_DIRS
    NAMES gnuradio/yunsdr/api.h
    HINTS $ENV{YUNSDR_DIR}/include
        ${PC_YUNSDR_INCLUDEDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/include
          /usr/local/include
          /usr/include
          ${GNURADIO_INSTALL_PREFIX}/include
)

FIND_LIBRARY(
    YUNSDR_LIBRARIES
    NAMES gnuradio-yunsdr
    HINTS $ENV{YUNSDR_DIR}/lib
        ${PC_YUNSDR_LIBDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/lib
          ${CMAKE_INSTALL_PREFIX}/lib64
          /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
          ${GNURADIO_INSTALL_PREFIX}/lib
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(YUNSDR DEFAULT_MSG YUNSDR_LIBRARIES YUNSDR_INCLUDE_DIRS)
MARK_AS_ADVANCED(YUNSDR_LIBRARIES YUNSDR_INCLUDE_DIRS)
