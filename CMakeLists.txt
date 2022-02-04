cmake_minimum_required(VERSION 3.1)

set(CMAKE_VERBOSE_MAKEFILE ON)
# note: cxx-17 requires cmake 3.8, cxx-20 requires cmake 3.12
project(basic_pub_sub CXX)

file(GLOB SRC_FILES
       "./main.cpp"
)

if(MUSL_BUILD)
    SET (CMAKE_C_COMPILER "/x86_64-linux-musl-cross/bin/x86_64-linux-musl-gcc")
    SET (CMAKE_CXX_COMPILER "/x86_64-linux-musl-cross/bin/x86_64-linux-musl-g++")
endif(MUSL_BUILD)

add_executable(${PROJECT_NAME} ${SRC_FILES})

if(STATIC_BUILD)
    SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
    SET(BUILD_SHARED_LIBS OFF)
    SET(CMAKE_EXE_LINKER_FLAGS "-static")
else()
    SET(BUILD_SHARED_LIBS ON)
endif(STATIC_BUILD)

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 14)

#set warnings
target_compile_options(${PROJECT_NAME} PRIVATE -Wno-long-long -pedantic)


if(MUSL_BUILD)
    #link_directories("/x86_64-linux-musl-cross/usr/local/lib")
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L/x86_64-linux-musl-cross/usr/local/lib")
    include_directories("/x86_64-linux-musl-cross/usr/local/include")
else()
    #link_directories("/usr/local/lib")
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L/usr/local/lib")
    include_directories("/usr/local/include")
endif(MUSL_BUILD)

set(SUPPORTING_LIBS ${SUPPORTING_LIBS}
    IotJobs-cpp
    aws-crt-cpp aws-c-s3
    aws-c-auth aws-c-event-stream 
    aws-c-cal aws-c-mqtt aws-c-io 
    aws-c-http aws-checksums aws-c-compression aws-c-common aws-c-iot)
	#aws-c-iot s2n)

if(STATIC_BUILD)
    if(MUSL_BUILD)
        #set(SUPPORTING_LIBS ${SUPPORTING_LIBS}  /x86_64-linux-musl-cross/usr/local/lib/libcrypto.a)
        set(SUPPORTING_LIBS ${SUPPORTING_LIBS}  /usr/local/ssl/lib/libcrypto.a)
    else()
        set(SUPPORTING_LIBS ${SUPPORTING_LIBS}  /usr/local/lib/libcrypto.a)
    endif(MUSL_BUILD)
else()
    set(SUPPORTING_LIBS ${SUPPORTING_LIBS} crypto)
endif(STATIC_BUILD)

set(STANDARD_LIBS ${STANDARD_LIBS} rt pthread dl)

target_link_libraries(${PROJECT_NAME} ${SUPPORTING_LIBS} ${STANDARD_LIBS})


