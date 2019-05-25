# https://qiita.com/take-iwiw/items/50a47dd3ff3e9163de0d

# cmake ../project -DCMAKE_BUILD_TYPE=Release
# cmake ../project -DCMAKE_BUILD_TYPE=Debug
# set(CMAKE_BUILD_TYPE release)
# set(CMAKE_BUILD_TYPE debug)

set(ARM_COMPILE_OPTION "-mcpu=native -mfpu=neon-vfpv4 -mfloat-abi=hard")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ARM_COMPILE_OPTION}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ARM_COMPILE_OPTION}")