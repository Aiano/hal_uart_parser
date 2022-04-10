/**
 * @file uart_parser.c
 * @brief 串口指令解析器
 * @details 指令格式 <命令字母> + <double数值>
 */

#include "uart_parser.h"
#include "string.h"
#include "stdlib.h"

#define RX_BUFFER_SIZE 256
// receive data byte by byte
// 逐Byte接收数据
uint8_t aRxBuffer = 0;
// and store aRxBuffer into RxBuffer
// 把aRxBuffer里的单Byte数据存到RxBuffer里
char RxBuffer[RX_BUFFER_SIZE];
int RxBufferCounts = 0;
// feedback message
// 返回给上位机的消息，表示收到命令
char FeedBackMessage[RX_BUFFER_SIZE + 20];

// callback function list, 20 is an arbitrary number.
// 这是一个数组，元素类型为函数指针，指向用户指定的回调函数
UartParserCommandCallback call_list[20];
// 命令字母
char call_commands[20];
// 命令标签
char *call_labels[20];
int call_count = 0;

/**
 * @brief 初始化串口解析器
 * @param huart
 */
void uart_parser_init(UART_HandleTypeDef *huart) {
    // clear rx buffer
    // 清空接收缓冲数组
    memset(RxBuffer, 0, sizeof(RxBuffer));
    // enable uart interrupt once
    // 第一次启动中断，之后触发后需要再次调用该函数
    HAL_UART_Receive_IT(huart, (uint8_t * ) & aRxBuffer, 1);
}


/**
 * @brief redefine Rx Transfer completed callbacks function.
 * @brief 重定义Rx Transfer completed callbacks function（这是HAL库的一个weak函数，在打开串口中断，并且接收串口数据完成时触发）
 * @param huart
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    // when enter this function, a byte of data has been stored in aRxBuffer.
    // 当进入这个函数，说明已经有一个数据存储到aRxBuffer里面了
    // now, push aRxBuffer into RxBuffer.
    // 现在要做的事，就是把aRxBuffer存入RxBuffer
    RxBuffer[RxBufferCounts++] = aRxBuffer;
    // if encounter LF, the transfer process has completed.
    // 如果遇到了LF，说明传输结束，这也意味着上位机传来的命令必须带有 \n 字符
    // LF = Linefeed 换行 ASCII 10 \n newline
    // CR = Carriage Return 回车 ASCII 13 \r return
    if (RxBuffer[RxBufferCounts - 1] == 10) {
        // output feedback message
        // 输出返回信息给上位机
        memset(FeedBackMessage, 0, sizeof(FeedBackMessage));
        strcat(FeedBackMessage, "Received:");
        strcat(FeedBackMessage, RxBuffer);
        HAL_UART_Transmit(huart, FeedBackMessage, sizeof(FeedBackMessage), 0xFF);

        // do something to the RxBuffer
        // 开始对接收到的数据进行处理
        for (int i = 0; i < call_count; i++) {
            // match subscribed command
            // 和第一个字符匹配已经注册的命令
            if (call_commands[i] == RxBuffer[0]) {
                // enter callback function
                // 若匹配成功，则调用注册的回调函数
                (*call_list[i])(RxBuffer + 1);
            }
        }

        // end of do something to the RxBuffer

        // clear RxBuffer and reset counts
        memset(RxBuffer, 0, sizeof(RxBuffer));
        RxBufferCounts = 0;
    }
    // enable uart interrupt again to receive rest data, or for the next reception
    // 串口中断触发一次后，必须重新启动中断
    HAL_UART_Receive_IT(huart, (uint8_t * ) & aRxBuffer, 1);
}

/**
 * @brief 注册命令
 * @param command
 * @param callback_func_pointer
 * @param label
 */
void uart_parser_add_command(char command, UartParserCommandCallback callback_func_pointer, char *label) {
    call_list[call_count] = callback_func_pointer;
    call_commands[call_count] = command;
    call_labels[call_count] = label;
    call_count++;
}

/**
 * @brief 把字符类型cmd转化为浮点值value
 * @param value
 * @param cmd
 */
void uart_parser_scalar(float *value, char *cmd) {
    if (cmd[0] != 10) {
        *value = atof(cmd);
    }
}

/**
 * @brief 输出一个字符串罢了
 * @param output
 * @param huart
 */
void uart_parser_output(char *output, UART_HandleTypeDef *huart) {
    HAL_UART_Transmit(huart, output, sizeof(output), 0xFF);
}