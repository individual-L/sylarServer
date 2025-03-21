## 组件
1. ServBase：根据http请求去初始化一个http响应
2. ServFun：继承自ServBase，根据http请求调用回调函数去初始化一个http响应对象
3. httpDispatcher：根据请求路径调用不同的ServFunc对象去处理。