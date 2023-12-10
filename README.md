
# MateBook E Pen

![GitHub release (latest by date)](https://img.shields.io/github/v/release/eiyooooo/MateBook-E-Pen)
![GitHub all releases](https://img.shields.io/github/downloads/eiyooooo/MateBook-E-Pen/total)

一个为解决MateBook E 系列(E 2022/E Go/E 2023)搭配M-Pencil使用时，侧边双击功能太少而开发的一个程序


## 特性

- 适配 OneNote for Windows 10 橡皮擦和笔的切换

- 适配 Drawboard PDF 橡皮擦和笔的切换

- 适配 Nebo 橡皮擦和笔的切换

- 自定义 橡皮擦和笔的切换 (非UWP应用)

- 截图模式

- 适配 华为一键摘录/全局批注 模式(保存图片后自动复制到剪切板)

- 复制/粘贴模式

- 撤销模式

## 使用方法
下载并打开[“MateBook-E-Pen.exe”](https://github.com/eiyooooo/MateBook-E-Pen/releases/latest)

(即开即用)

## 更新历史

#### v2.1.1

1.优化 可在没安装电脑管家的情况下运行

2.优化 捕获面板逻辑

#### v2.1.0

1.适配 Nebo 橡皮擦和笔的切换

2.修复 已知问题

#### v2.0.0

1.新增 配置文件保存常用配置

2.新增 自定义橡皮擦和笔的捕获/保存/切换

3.优化及修复

  1. 软件启动速度
  2. 偶发无法正确执行截图/复制/粘贴/撤销
  3. 批注模式第一次双击无法使用

#### v1.2.0

1.优化 无感初始化

2.修复 部分情况无法使用 的问题

#### v1.1.0

1.新增 OneNote for Windows 10 记忆上次使用的笔

2.优化 以位图形式复制批注保存的图片

3.优化 OneNote for Windows 10/双击检测/字符串处理 函数

4.修复 网络需登录时检查更新显示错误 的问题 

#### v1.0.0

1.新增 模式控制悬浮球

2.新增 粘贴后跳转到自定义模式 功能

3.移除 截图后自动切换粘贴模式

4.优化 初始化 逻辑

5.修复 OneNote for Windows 10打开时默认为无标题页时无法切换橡皮和笔切换的问题

#### v0.4.0

1.重构 初始化函数(实现即开即用)

2.新增 重新初始化 按钮

#### v0.3.1

1.优化 华为一键摘录/全局批注 保存图片后自动复制到剪切板的功能

2.修复 点击切换面板批注和复制按钮图标无变化的问题

#### v0.3.0

1.适配 华为一键摘录/全局批注 模式

2.优化 始终锁定Windows Ink双击按钮设置

#### v0.2.2

1.适配 OneNote for Windows 10全屏页面

2.优化 图标单击切换功能，双击打开切换面板

3.修复 无网络环境下检查更新时，程序闪退问题

4.优化 使用方法(无需进入”笔和Windows Ink“调整设置)

#### v0.2.1
1.新增 切换面板

2.优化 使用逻辑

  1. 点击图标后自动返回使用中的应用
  2. 复制模式使用后自动进入粘贴模式
  3. 粘贴模式使用后自动返回笔和橡皮模式
  4. 开启程序时检测是否多开
  5. 确定前往下载更新后自动关闭程序
  
3.优化 界面

4.图标适配深色模式

#### v0.2.0
1.新增 截图、复制/粘贴、撤销模式

2.新增 打开Windows笔设置快捷键

#### v0.1.0
1.适配 OneNote for Windows 10 橡皮擦和笔的切换

2.适配 Drawboard PDF 橡皮擦和笔的切换

## To do

- 无


## 项目由来
刚上大一的~~大学牲~~想体验无纸化学习

而且只有台式机没有笔记本无法~~异地~~离开宿舍办公

Android和iPadOS太封闭

遂购入一台MateBook E

使用一段时间后发现M-Pencil的侧键可以说是毫无功能可用

~~而C语言老师的Surface手写笔用着十分炫酷~~

正好入门了C语言，就想写个程序完善一下这笔的功能

（然而只用C来写是不可能的）


## License


[![GPLv3 License](https://img.shields.io/badge/License-GPL%20v3-yellow.svg)](https://opensource.org/licenses/)
