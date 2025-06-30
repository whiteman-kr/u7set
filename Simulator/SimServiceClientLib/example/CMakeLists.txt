project(SimServiceClientLib LANGUAGES CXX)

set(TARGET SimServiceClientExample)

add_executable(${TARGET}
	Actions.cpp
	Actions.hpp
	main.cpp
)

target_link_libraries(${TARGET} PUBLIC SimServiceClientLib)
target_include_directories(${TARGET} PRIVATE ${CMAKE_SOURCE_DIR})
