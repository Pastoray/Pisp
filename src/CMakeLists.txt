set(SOURCE_FILES
    Compiler.cpp
    Parser.cpp
    Source.cpp
    Tokenizer.cpp
    Utils.cpp
    VM.cpp
)

add_executable(pisp ${SOURCE_FILES})

target_include_directories(pisp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
