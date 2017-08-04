
# xval

xval是一个轻量级的C++动态类型对象库，主要用于与其他动态语言的通信

## Basic type

* NIL
* Integer 64bit
* Float 64bit
* String
* List 列表、动态数组
* Tuple 元祖，不可变的列表
* Dict 字典、关联数组，支持任意类型(除NIL外)的键值存储

## Features

* 内建msgpack打包/解包接口
* 短字符串内部化
* 引用计数的内存管理方式，对用户完全透明
