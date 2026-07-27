#ifndef __COM_DEBUG_H__
#define __COM_DEBUG_H__
#include "stdio.h"
#include "usart.h"
#include "string.h"
/**
 * @brief
 *  printf本质串口使用,串口通信的时候,本身占用时间！
 *  但是需要注意的是,咱们开发阶段(开发环境)测试代码经常使用printf.
 *  但是,项目完事(生产环境),源码代码就不应该出现测试代码printf!!!
 *   预处理指令:很多程序员称之为代码裁剪！
 *
 *   在C语言当中...,表示(宏)的是可变参数
 *   在C语言当中#__VA_ARGS__,表示(函数)的可变参数
 *   ##__VA_ARGS__,最前面的井号表示表前面逗号去掉！(可变参数没有)
 *
 */

// 宏:DEVELOPMENT(开发)   PRODUCTION(生产)
#define DEVELOPMENT
#ifdef DEVELOPMENT

#define FILENAME strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/') + 1) : __FILE__

#define COM_DEBUG(format, ...) printf("[%s:%d]" format, FILENAME, __LINE__, ##__VA_ARGS__)

#define COM_DEBUG_LN(format, ...) printf("[%s:%d]" format "\r\n", FILENAME, __LINE__, ##__VA_ARGS__)

#else
#define COM_DEBUG(format, ...) 
#define COM_DEBUG_LN(format, ...) 
#endif // DEBUG

#endif /* __COM_DEBUG_H__ */
