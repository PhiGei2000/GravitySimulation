cmake_minimum_required(VERSION 3.24.0)
project(GravitySimulation VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)

if(WIN32)
    include(FetchContent)

    FetchContent_Declare(glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG bf71a834948186f4097caa076cd2663c69a10e1e
    )

    FetchContent_Declare(glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
    )

    FetchContent_Declare(glew
        GIT_REPOSITORY https://github.com/Perlmint/glew-cmake.git
    )


    FetchContent_MakeAvailable(glm)
    FetchContent_MakeAvailable(glfw)
    FetchContent_MakeAvailable(glew)
endif()

include_directories(include)

include(sourcelist.cmake)

add_executable(GravitySimulation ${SOURCES})

target_link_libraries(GravitySimulation PRIVATE glm::glm glfw libglew_static)

