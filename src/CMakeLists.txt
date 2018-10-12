# 
# External dependencies
# 



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
    Config.hpp
    Playlist.hpp
    Server.hpp

    Protocols/Base.hpp
    Protocols/MPD.hpp
    Protocols/MPRIS.hpp
    Protocols/REST.hpp

    Protocols/Base/Event.hpp
    Protocols/MPD/Commands.hpp

    Util/Path.hpp
    Util/Tokeniser.hpp
)

set(sources
    Config.cpp
    Playlist.cpp
    Server.cpp

    Protocols/MPD.cpp
    Protocols/MPRIS.cpp
    Protocols/REST.cpp

    Util/Path.cpp

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
)


# 
# Libraries
# 

target_link_libraries(${META_PROJECT_NAME}
    PRIVATE
    ${DEFAULT_LIBRARIES}
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
