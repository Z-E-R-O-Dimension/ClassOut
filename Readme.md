# ClassOut

## NOTE/注意事项

**IMPORTANT:**  
This project is still in an early stage.  
All popup menu items in English are NOT completed or NOT tested carefully.  
Misuse could lead to unexpected results.

**重要：**  
本项目仍处于早期阶段，所有英文的下拉菜单项仍未完全实现或有待测试。误用可导致意外的后果。

## 前言

***网课千万节，专注每一节***  
***上课不认真，考试两行泪***  

## 概述

* 解决ClassIn专注学习模式的最大化置顶教室窗口  
  Windows 10或以上版本系统中有虚拟桌面。但对于仍使用Windows 7或者想要边上课边摸鱼的用户，专注学习模式无疑是一个难以解决的问题。

## 组成

### ClassOut\ClassOut.vcxproj

* `ClassOut`主程序，实现主要功能
* 依赖于`helperDll`，故`helperDll`应先于`ClassOut`生成

### helperDll\helperDll.vcxproj

* 用于实现API Hook的dll，`增强模式`中启动ClassIn时强制加载
* 作为资源载入`ClassOut.exe`中（由`ClassOut\ClassOut.rc`中相对路径`"..\\Release\\helperDll.dll"`指定），启动`ClassOut`时释放到临时文件夹中（通常是`%USERPROFILE%\AppData\Local\Temp\ClassOut\helperDll.dll`）
* 由于ClassIn是32位，故应生成32位的dll

## 构建

1. 需求
   * `Visual Studio 2022`（旧版本请尝试“重定解决方案目标"）
   * `detours:x86-windows`（安装方法见下）
     >
     > 1. 安装 [vcpkg](https://github.com/Microsoft/vcpkg)
     > 2. 执行 `vcpkg install detours:x86-windows`
     > 3. 启用集成 `vcpkg integrate install`
     >
2. 使用 `Visual Studio 2022` 生成解决方案

## 特点

* 使用`Win32 API`/`C++`黑魔法构建
* 资源占用低（至少目前是），bug无数，代码风格自由散漫
* ⚠本项目尚未完成，所以release里的仅仅是alpha版

## 用法

1. 启动`ClassOut.exe`，进入教室，按`Ctrl-F1`使`ClassOut`主窗口置顶显示
2. 然后选择菜单栏`ClassIn`->`刷新`，`ClassIn`->`隐藏`
3. 享受生活

## 特性/已知问题

* 置顶热键
  * 通过`Ctrl-F1`或菜单`ClassOut`->`置顶`以切换`ClassOut`主窗口置顶
  * `ClassOut`主窗口最小化时置顶无效。用`Alt-Tab`调出主窗口再置顶即可
  * 同时启动多个ClassOut实例时后启动的热键将失败
* 隐藏/置底
  * 最小化（`SW_MINIMIZE`）或隐藏（`SW_HIDE`）的窗口抓图将是空白，故通过移动教室窗口至屏幕外实现隐藏
  * 置底将取消教室窗口的置顶属性，从而其他窗口得以显示
* “转播” ***⚠未完全实现⚠***
  * 使用 `PrintWindow` 将教室窗口的内容保存到内存中
  * 使用 `StretchBlt` 将内存中的图像拉伸显示到`ClassOut`主窗口上
  * 使用 `SendMessageW`/`PostMessageW` 转发主窗口收到的鼠标键盘消息到教室窗口，可直接对ClassOut主窗口执行鼠标单击右击等操作
  * 转播的是`设备上下文`中未经系统缩放的画面
  * ⚠显示比例可调，但是 `StretchBlt` 缩放质量堪忧。可尝试通过 `Direct2D` 辅助缩放
  * ⚠输入法支持尚不完善，输入焦点在ClassOut主窗口进行英文输入时会有大小写、上档转换键等问题；输入焦点在教室窗口时无上述问题。同时输入法窗口不会对齐光标
  * ⚠聊天窗口移出教室外时聊天窗口会消失
  * 上述问题可通过取消隐藏教室窗口暂时解决
* 缩放/DPI感知 ***⚠未完全实现⚠***
  * 获取屏幕和教室窗口缩放因子并计算坐标等，用于转发鼠标消息
  * ⚠尚不清楚 `ClassIn` 如何决策是否启用DPI感知（设置里的`开启高分屏支持`并没有用）
  * ⚠对于启用DPI感知的ClassIn，坐标计算不准确，导致鼠标操作错位
* 增强模式 ***⚠几乎未实现⚠***
  * 通过`连接`菜单->`Start ClassIn With Dll`，`连接`菜单->`Connect`来启用增强模式
  * 通过API Hook，可实现上下台提醒、视频墙提醒、黑板焦点跟踪（可能）、屏幕共享伪装等高阶操作
  * 上下台提醒：使用`Detours`将`Qt5Gui.dll`中  
    `public: void __thiscall QPainter::drawText(class QPoint const &,class QString const &)`  
    等函数替换，可截获ClassIn调用Qt显示的所有字符串。然后用`Qt5Core.dll`中  
    `public: int __thiscall QString::toWCharArray(wchar_t *)const`  
    将`QString`转换为`wchar_t *`再进行比较。例如`你下台了，暂时无法与大家互动`
  * 有无限可能，不过**现在什么都做不了**

## 其他待办

* ClassOut.exe无需加载helperDll.dll
* 更好的图形界面

## 关于图标

![年轻又帅气而富有师德的cjw 作于课上](ClassOut/rcs/icon.png#pic_center "年轻又帅气而富有师德的cjw 作于课上")  

## 一些话

* 这个项目的想法始于2021年暑假网课期间。当时完成了初代版本（Release里的`ClassInUnfetter.exe`），少一些功能，但兼容性可能更好。ClassOut出现问题可以试试那个
* 愿每个受疫情影响的学子都能在真正的教室上课
