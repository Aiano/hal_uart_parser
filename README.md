# HAL UART PARSER

## 简介

基于ST官方的HAL库开发的UART串口指令解析器

## 移植

1. `uart_parser.h`复制到`Inc`文件夹

2. `uart_parser.c`复制到`Src`文件夹

3. `mian.c`中引用头文件

   ```c
   /* Private includes ----------------------------------------------------------*/
   /* USER CODE BEGIN Includes */
   #include "uart_parser.h"
   /* USER CODE END Includes */
   ```

4. main.c中声明一个回调函数

   ```c
   float test_f = 0;
   
   void do_something(char *cmd){
       uart_parser_scalar(&test_f, cmd);
   }
   ```

5. mian.c中初始化，并注册命令

   ```c
   /* USER CODE BEGIN 2 */
   uart_parser_init(&huart2);
   uart_parser_add_command('A', do_something, "do something");
   /* USER CODE END 2 */
   ```

即可正常运行