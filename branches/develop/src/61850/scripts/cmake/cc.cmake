# 自动查找头文件路径函数(没有去重) 定义函数,2个参数:存放结果result；指定路径curdir；
macro(FIND_INCLUDE_DIR result curdir)
  # 遍历获取{curdir}中*.hpp和*.h文件列表
  file(GLOB_RECURSE children "${curdir}/*.hpp" "${curdir}/*.h")
  # 打印*.hpp和*.h的文件列表
  # message(STATUS "children= ${children}")
  # 定义dirlist中间变量，并初始化
  set(dirlist "")

  foreach(child ${children})
    # 字符串替换,用/前的字符替换/*h
    string(REGEX REPLACE "(.*)/.*" "\\1" LIB_NAME ${child})
    # 判断是否为路径
    if(IS_DIRECTORY ${LIB_NAME})
      # 将合法的路径加入dirlist变量中
      list(APPEND dirlist ${LIB_NAME})
    endif()
  endforeach()
  # dirlist结果放入result变量中
  set(${result} ${dirlist})
endmacro()

# #查找include目录下的所有*.hpp,*.h头文件,并路径列表保存到 INCLUDE_DIR_LIST 变量中
# FIND_INCLUDE_DIR(INCLUDE_DIR_LIST ${PROJECT_SOURCE_DIR}/source) //调用函数，指定参数

# #将INCLUDE_DIR_LIST中路径列表加入工程 include_directories( //INCLUDE_DIR_LIST路径列表加入工程
# ${INCLUDE_DIR_LIST} )

# message(STATUS "INCLUDE_DIR_LIST = ${INCLUDE_DIR_LIST}")
# //打印INCLUDE_DIR_LIST中列表信息
