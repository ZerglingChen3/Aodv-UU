# AODV-UU

## 整体情况

BUAA 无线网络系统课程设计 2021.09～2021.12

基于原生AODV-UU协议进行了扩展，提取周围环境信息，进行自适应的寻路策略。

测试采取NS2仿真器。

MAC层协议是mac-802/11。



## 内容

code：所有修改过的Aodv-UU协议源码（在源码中直接进行替换）

data：所有使用过的tcl脚本

data-maker：由C++自动根据配置文件生成对应的tcl脚本

md：相关协议阅读介绍和源码修改介绍

process：一个简单的python数据筛选器