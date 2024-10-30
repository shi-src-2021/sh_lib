### 简介
sh_lib是一个mcu组件库，使用了面对对象编程范式，集成了多种常用组件，可以加快项目开发进度，快速实现以事件驱动为主的业务流程：
* 链表
* 哈希表
* 按键驱动
* 事件驱动框架
* 环形缓冲区
* 内存池
* 软件定时器
* 状态机

### 注意事项
使用中断的开关进行数据的包含，这就要求所有的回调函数应尽量简短，避免实时性的下降。

### 使用示例
[按键示例](https://github.com/shi-src-2021/sh_lib/blob/master/example/sh_button_example.c)

[状态机示例](https://github.com/shi-src-2021/sh_lib/blob/master/example/sh_sm_example.c)

[链表示例](https://github.com/shi-src-2021/sh_lib/blob/master/example/sh_list_example.c)
