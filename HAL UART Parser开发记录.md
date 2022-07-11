# HAL UART Parser 开发记录

> [HAL UART](https://blog.csdn.net/as480133937/article/details/99073783)
>
> [串口解析指令](https://blog.csdn.net/u012388993/article/details/79628218)

## HAL库

### UART结构体定义

```c++
UART_HandleTypeDef huart1;
```

这个结构体中存放了**UART所有用到的功能**

串口发送/接收函数

- HAL_UART_Transmit();串口发送数据，使用**超时管理机制** 
- HAL_UART_Receive();串口接收数据，使用**超时管理机制**
- HAL_UART_Transmit_IT();串口中断模式发送  
- HAL_UART_Receive_IT();串口中断模式接收
- HAL_UART_Transmit_DMA();串口DMA模式发送
- HAL_UART_Transmit_DMA();串口DMA模式接收

### 串口发送数据：

```c++
HAL_UART_Transmit(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size, uint32_t)
```

功能：**串口****发送指定长度的数据。如果超时没发送完成，则不再发送，返回超时标志（HAL_TIMEOUT）。**

参数：

- UART_HandleTypeDef *huart      UATR的别名    如 :   UART_HandleTypeDef huart1;   别名就是huart1 
- *pData      需要发送的数据 
- Size    发送的**字节数**
- Timeout   最大发送时间，发送数据超过该时间退出发送   

> sizeof()
>
> 返回一个对象或者类型所占的内存**字节数**
>
> 故上述函数中的Size只需要填入sizeof(pData)即可.

**实验:**

```c++
char data[] = "A\n"; // 发送的数据
char ok[]="ok\n"; // 发送一个OK
HAL_StatusTypeDef status = HAL_UART_Transmit(&huart2, data, sizeof(data), 0xFF); // 如果发送成功,会返回一个HAL_OK标志位,如果超时,会返回HAL_TIMEOUT标志位
if(status == HAL_OK){
	HAL_UART_Transmit(&huart2, ok, sizeof(ok), 0xFF);
}
HAL_Delay(100);
```

如果一切正常,串口输出应该是A与OK交替进行

### 串口中断接收数据

**CubeMX配置**

NVIC Setting -> USART global interrupt √

**函数定义**

```c++
HAL_UART_Receive_IT(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size)
```

功能：**串口中断接收，以中断方式接收指定长度数据。
大致过程是，设置数据存放位置，接收数据长度，然后使能串口接收中断。接收到数据时，会触发串口中断。
再然后，串口中断函数处理，直到接收到指定长度数据，而后关闭中断，进入中断接收回调函数，不再触发接收中断。(只触发一次中断)**

参数：

- UART_HandleTypeDef *huart   **UATR的别名**  如 :  UART_HandleTypeDef huart1;  别名就是huart1 
- *pData   **接收到的数据存放地址**
- Size  <u>**注意注意！！！Size指的是希望被读取的总字节数，所以如果设置为3，则只有读满三个字符，才会进入中断**</u>

***\*串口接收中断回调函数：\****

```c++
HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);  
```

功能：HAL库的中断进行完之后，并不会直接退出，而是会进入中断回调函数中，用户可以在其中设置代码

**串口中断接收完成之后，会进入该函数**，该函数为空函数，用户需自行修改

> HAL_UART_RxCpltCallback函数的原型带有__weak弱定义前缀，原型为
>
> ```c
> #define __weak   __attribute__((weak))
> ```
>
> [C语言__arrtibute__](https://blog.csdn.net/huasir_hit/article/details/77531942)
>
> *GNU C* 的一大特色就是`__attribute__`机制。`__attribute__` 可以设置函数属性（*Function Attribute* ）、变量属性（*Variable Attribute* ）和类型属性（*Type Attribute* ）
>
> `__attribute__` 语法格式为：`__attribute__ ((attribute-list))`
>
> `__attribute__((weak))`的作用就是把该函数弱定义，若其他地方出现相同函数，则此处不编译

**实验：**

串口中断debug的几个方法：

- 观察ttl转usb模块的tx灯有无闪烁
- 用逻辑分析仪检查是否正常输出信号
- 使用stm32的debug模式，在中断回调函数处设置断点，若发送后正常进入回调函数，说明硬件无问题

关于上位机软件：

- 如果使用的是串口猎人，输入内容里面要带回车换行，这样读取数据时才会检测到换行，相当于数据传输结束的一个标志位
- 发码区要选择字符串

一些问题：

- HAL_UART_Receive_IT函数的Size应该设置为1，<u>因为如果设置为n，则只有读满n个字符，才会进入中断，然而上位机每次传输的字符总数不是固定的，这使得stm32的行为无法预测</u>，所以我们要一个一个字符读取，然后检测传输结束标志位，比如回车

## 指令解析

### 函数指针（指向函数的指针）

> [C函数指针](http://c.biancheng.net/view/2023.html)
>
> 一个函数总是占用一段连续的内存区域，**函数名在表达式中有时也会被转换为该函数所在内存区域的首地址**，这和数组名非常类似。我们可以把函数的这个首地址（或称入口地址）赋予一个[指针](http://c.biancheng.net/c/80/)变量，使指针变量指向函数所在的内存区域，然后通过指针变量就可以找到并调用该函数。这种指针就是函数指针。
>
> 函数指针的定义形式为：
>
> ```c 
> returnType (*pointerName)(param list);
> ```
>
> `param list`参数列表中可以同时给出参数的类型和名称，也可以只给出参数的类型，省略参数的名称，这一点和函数原型非常类似。
>
> 注意`( )`的优先级高于`*`，第一个括号不能省略，如果写作`returnType *pointerName(param list);`就成了函数原型，它表明函数的返回值类型为`returnType *`。
>
> 调用函数指针指向的函数：
>
> ```c
> (*pointerName)(param);
> ```
>

### sizeof的问题：

sizeof(RxBuffer)的结果是256，但是sizeof(RxBuffer + 1)的结果是4

> https://blog.csdn.net/FX677588/article/details/77151014
>
> **如果sizeof的操作数是一个表达式的话，这个表达式时不会被计算的。**
> **sizeof当预处理看就行了,它后面括号里的东西,根本不求值,只根据C的一堆规则判断结果类型,然后返回结果类型的大小。**
