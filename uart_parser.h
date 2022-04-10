/**
 * @file uart_parser.c
 * @brief 串口指令解析器
 * @details 指令格式 <命令字母> + <double数值>
 */

#ifndef HAL_UART_PARSER_UART_PARSER_H
#define HAL_UART_PARSER_UART_PARSER_H

#include "usart.h"

#ifdef __cplusplus
extern "C"{
#endif

    typedef void (* UartParserCommandCallback)(char *); // 这是一个函数指针

    void uart_parser_init(UART_HandleTypeDef *huart);
    void uart_parser_add_command(char command, UartParserCommandCallback callback_func_pointer, char *label);
    void uart_parser_scalar(float* value, char *cmd);
    void uart_parser_output(char *output, UART_HandleTypeDef *huart);

#ifdef __cplusplus
}
#endif

#endif //HAL_UART_PARSER_UART_PARSER_H
