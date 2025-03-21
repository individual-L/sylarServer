- 继承自tcpServer，重写了处理连接请求的虚函数
## 容器
1. tcpServer：聚合一个socket模块，实现多个网卡监听和异步建立连接，连接建立后会异步处理连接请求，并提供处理连接请求的虚函数
2. [httpSession](./httpSession.md)：继承自socketIOStream，配合http解析对象，实现读取报文解析报文封装http请求和响应并返回
