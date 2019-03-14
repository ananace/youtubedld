# 
# External dependencies
# 

find_package(gstreamermm REQUIRED)

# 
# Library name and options
# 

# Exit here if required dependencies are not met
message(STATUS "App ${META_PROJECT_NAME}")


# 
# Sources
# 

# Generate version file
#configure_file(Version.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/${META_PROJECT_NAME}-Version.cpp)

set(headers
    ActivePlaylist.hpp
    Config.hpp
    Playlist.hpp
    Server.hpp

    Protocols/Base.hpp
    Protocols/MPD.hpp
    Protocols/MPRIS.hpp
    Protocols/REST.hpp

    Protocols/Base/Event.hpp
    Protocols/MPD/Commands.hpp

    Util/EpollServer.hpp
    Util/GObjectSignalWrapper.hpp
    Util/Logging.hpp
    Util/Path.hpp
    Util/Tokeniser.hpp
    Util/WorkQueue.hpp
    Util/YoutubeDL.hpp
)

set(sources
    ActivePlaylist.cpp
    Config.cpp
    Playlist.cpp
    Server.cpp

    Protocols/MPD.cpp
    Protocols/MPRIS.cpp
    Protocols/REST.cpp

    Protocols/MPD/Acks.cpp
    Protocols/MPD/Commands.cpp

    Util/EpollServer.cpp
    Util/Logging.cpp
    Util/Path.cpp
    Util/WorkQueue.cpp
    Util/YoutubeDL.cpp

    main.cpp
)


# Group source files
set(header_group "Header Files")
set(source_group "Source Files")
source_group_by_path(${CMAKE_CURRENT_SOURCE_DIR} "\\\\.h$|\\\\.hpp$|\\\\.inl$" 
    ${header_group} ${headers})
source_group_by_path(${CMAKE_CURRENT_SOURCE_DIR}  "\\\\.cpp$|\\\\.c$|\\\\.h$|\\\\.hpp$|\\\\.inl$" 
    ${source_group} ${sources})


# 
# Create executable
# 

# Build executable
add_executable(${META_PROJECT_NAME}
    ${sources}
    ${headers}
)


# 
# Project options
# 

set_target_properties(${META_PROJECT_NAME}
    PROPERTIES
    ${DEFAULT_PROJECT_OPTIONS}
    FOLDER "${IDE_FOLDER}"
)


# 
# Include directories
# 

target_include_directories(${META_PROJECT_NAME}
    PRIVATE
    ${PROJECT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${GSTREAMERMM_INCLUDE_DIRS}
)


# 
# Libraries
# 

target_link_libraries(${META_PROJECT_NAME}
    PRIVATE
    ${DEFAULT_LIBRARIES}
    ${GSTREAMERMM_LIBRARIES}
)


# 
# Compile definitions
# 

target_compile_definitions(${META_PROJECT_NAME}
    PRIVATE
    ${DEFAULT_COMPILE_DEFINITIONS}
)


# 
# Compile options
# 

target_compile_options(${META_PROJECT_NAME}
    PRIVATE
    ${DEFAULT_COMPILE_OPTIONS}
)


# 
# Linker options
# 

target_link_libraries(${META_PROJECT_NAME}
    PRIVATE
    ${DEFAULT_LINKER_OPTIONS}
)


# 
# Install information
# 

install(
    TARGETS
    ${META_PROJECT_NAME}
    RUNTIME DESTINATION bin
)
