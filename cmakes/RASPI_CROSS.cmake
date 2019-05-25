# https://qiita.com/take-iwiw/items/50a47dd3ff3e9163de0d

# ラズパイのCrossCompiler設定をしています。クロスコンパイラとしては、
# git clone https://github.com/raspberrypi/toolsで~/にダウンロードしてきたものを使います。
# まず、CROSS_COMPILEやCMAKE_C_COMPILERを設定することで、使用するコンパイラの設定をします。
# その後、CMAKE_FIND_ROOT_PATHを設定することでtoolchainの場所を指定します。
# また、ビルドする際に、ライブラリとインクルードファイルはCMAKE_FIND_ROOT_PATHで指定された
# ターゲット用のものだけを使用する用に設定しています。
# ラズパイ用のCross Compilerに関してはこちらの記事もご参照ください
# (https://qiita.com/take-iwiw/items/5b20558f8ab3f27ca4a4 )。


# cmake ../project -DCMAKE_BUILD_TYPE=Release
# cmake ../project -DCMAKE_BUILD_TYPE=Debug
# set(CMAKE_BUILD_TYPE release)
# set(CMAKE_BUILD_TYPE debug)

set(ARM_COMPILE_OPTION "-mcpu=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ARM_COMPILE_OPTION}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ARM_COMPILE_OPTION}")

# cross compiler settings
set(CMAKE_CROSSCOMPILING TRUE)
set(CROSS_COMPILE "arm-linux-gnueabihf-")
set(CMAKE_C_COMPILER $ENV{CMAKE_TOOLCHAIN_ROOT}/bin/${CROSS_COMPILE}gcc)
set(CMAKE_CXX_COMPILER $ENV{CMAKE_TOOLCHAIN_ROOT}/bin/${CROSS_COMPILE}g++)
set(CMAKE_LINKER $ENV{CMAKE_TOOLCHAIN_ROOT}/bin/${CROSS_COMPILE}gcc)

# root path settings
set(CMAKE_FIND_ROOT_PATH $ENV{CMAKE_TOOLCHAIN_ROOT})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)    # use host system root for program
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)     # use CMAKE_FIND_ROOT_PATH for library
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)     # use CMAKE_FIND_ROOT_PATH for include