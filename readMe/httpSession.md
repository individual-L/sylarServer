# 容器
1. http请求响应：封装了http请求和响应
2. httpParser：根据读取到的消息进行http请求报文解析，并设置响应的回调函数初始化http请求
3. socketIOStream：支持读取固定大小的数据，不读到不返回。
4. [httpDispatcher](./httpDispatcher.md)：运用单例模式，保证全局只有一个调度器；根据不同的请求路径调用不同回调函数初始化http响应
