# PetLocation 项目 Code Wiki

## 项目概述

**PetLocation** 是一个基于 STM32 微控制器的宠物定位器系统,采用主从架构设计。系统由定位器端(Location)和网关端(Gateway)两部分组成,通过 LoRa 无线通信和 2G/以太网实现远程定位数据上报。

### 核心功能
- GPS/北斗卫星定位(AT6558R芯片)
- 运动计步(DS3553芯片)
- 2G网络数据上传(QS100芯片)
- LoRa 远距离无线通信(E220x模块)
- 以太网数据上传(W5500芯片)
- 低功耗管理(STANDBY模式 + RTC唤醒)

---

## 项目架构

```
PetLocation/
├── gateway/          # 网关端(STM32F103ZE)
│   ├── App/         # 应用层(当前为空)
│   ├── Com/         # 通用组件层
│   ├── Core/        # 核心外设驱动
│   ├── Int/         # 集成外设驱动
│   └── MDK-ARM/     # Keil工程文件
│
└── location/        # 定位器端(STM32F103C8)
    ├── App/         # 应用层
    ├── Com/         # 通用组件层
    ├── Core/        # 核心外设驱动
    ├── Int/         # 集成外设驱动
    └── MDK-ARM/     # Keil工程文件
```

### 分层架构说明

#### 1. App层 (应用层)
- **职责**: 业务逻辑实现,数据整合与上报流程控制
- **文件**: `App_Main.c/h`
- **特点**: 协调各硬件模块,实现完整的功能流程

#### 2. Com层 (通用组件层)
- **职责**: 提供通用的工具函数和基础服务
- **主要模块**:
  - `Com_Debug`: 调试输出管理(支持开发/生产环境切换)
  - `Com_Delay`: 延时函数封装(ms/s级别)
  - `Com_Status`: 状态管理、数据结构定义、JSON序列化
  - `cJSON`: JSON解析库

#### 3. Core层 (核心外设驱动)
- **职责**: STM32 HAL库生成的基础外设驱动
- **外设包括**:
  - GPIO: 通用输入输出
  - USART: 串口通信(UART1/2/3)
  - SPI: SPI通信(SPI1/2)
  - I2C: I2C通信(I2C1)
  - RTC: 实时时钟(用于低功耗唤醒)

#### 4. Int层 (集成外设驱动)
- **职责**: 第三方芯片和模块的驱动封装
- **定位器端**:
  - `AT6558R`: GPS/北斗定位芯片
  - `QS100`: 2G通信模块
  - `DS3553`: 计步器芯片
  - `LoRa (E220x)`: LoRa无线通信模块
- **网关端**:
  - `LoRa (E220x)`: LoRa无线通信模块
  - `W5500`: 以太网控制器

---

## 定位器端 (Location) 详解

### 硬件配置
- **主控芯片**: STM32F103C8T6 (72MHz, 64KB Flash, 20KB RAM)
- **GPS模块**: AT6558R (通过UART2通信, 9600波特率)
- **2G模块**: QS100 (通过UART3通信, AT指令控制)
- **计步器**: DS3553 (通过I2C1通信)
- **LoRa模块**: E220x系列 (通过SPI1通信)
- **调试串口**: UART1 (115200波特率)

### 工作流程

```
1. 系统启动
   ↓
2. 唤醒外设 (App_Main_LeaveLowPower)
   - QS100唤醒
   - AT6558R上电
   ↓
3. 初始化外设 (App_Main_Init)
   - DS3553计步器初始化
   - AT6558R GPS初始化(设置波特率、更新频率、卫星系统)
   - QS100 2G模块初始化
   - LoRa模块初始化(设置同步字)
   ↓
4. 数据采集与上报 (App_Main_UploadData2Server)
   - 循环获取GPS数据,直到收到有效数据
   - 解析NMEA协议($GNRMC语句)
   - 提取时间、经纬度、速度信息
   - UTC时间转换为北京时间(+8小时)
   - 读取DS3553步数
   - 封装为JSON格式
   - 优先通过QS100(2G)上传到服务器
   - 如果QS100失败,通过LoRa发送
   ↓
5. 进入低功耗 (App_Main_EnterLowPower)
   - QS100进入快速关机模式
   - AT6558R断电
   - 设置RTC闹钟(10秒后唤醒)
   - STM32进入STANDBY模式
   ↓
6. RTC闹钟触发,系统复位,回到步骤2
```

### 关键数据结构

#### Upload_Data_T (上报数据结构)
```c
typedef struct {
    char timeDate[20];    // 北京时间: YYYY-MM-DD HH:MM:SS
    float lat;            // 纬度(度)
    char latDir[2];       // 纬度方向: N/S
    float lon;            // 经度(度)
    char lonDir[2];       // 经度方向: E/W
    float speed;          // 速度
    uint32_t step;        // 步数
    char JSONData[512];   // JSON字符串缓存
} Upload_Data_T;
```

#### JSON数据格式示例
```json
{
  "stepNum": 1234,
  "lat": 30.4682,
  "latDir": "N",
  "lon": 114.3883,
  "lonDir": "E",
  "speed": 3.44,
  "dateTime": "2025-05-06 23:49:19"
}
```

### 核心模块说明

#### 1. AT6558R GPS模块 (Int_AT6558R)
**文件**: `Int/AT6558R/Int_AT6558R.c/h`

**功能**:
- 初始化GPS芯片,配置工作参数
- 通过UART中断接收NMEA数据
- 提供GPS数据获取接口
- 控制GPS芯片电源(低功耗管理)

**关键函数**:
- `Int_AT6558R_Init()`: 初始化,设置波特率9600、更新频率250ms、北斗卫星系统
- `Int_AT6558R_GetGPS()`: 获取最新GPS数据
- `Int_AT6558R_SendCMD()`: 发送配置命令(自动计算校验和)
- `Int_AT6558R_EnterLowPower()`: 断电(拉低GPS_EN引脚)
- `Int_AT6558R_LeaveLowPower()`: 上电(拉高GPS_EN引脚)

**通信协议**:
- 使用$PCAS命令配置芯片
- 命令格式: `$PCASxx,yy*XX\r\n` (XX为异或校验和)
- 数据输出: NMEA-0183协议($GNRMC语句)

#### 2. QS100 2G模块 (Int_QS100)
**文件**: `Int/QS100/Int_QS100.c/h`

**功能**:
- 通过AT指令控制2G模块
- 网络附着状态检测
- TCP Socket创建与管理
- 数据发送与接收
- 低功耗管理

**关键函数**:
- `Int_QS100_Init()`: 初始化,发送AT+RB重启模块
- `Int_QS100_WakeUp()`: 唤醒模块(PB13引脚高脉冲100us-5s)
- `Int_QS100_IsNetWork()`: 检测网络附着(AT+CGATT?)
- `Int_QS100_CreateSocket()`: 创建TCP Socket(AT+NSOCR)
- `Int_QS100_ConnectServer()`: 连接服务器(AT+NSOCO)
- `Int_QS100_SendData2Server()`: 发送数据(AT+NSOSD)
- `Int_QS100_UploadData()`: 完整上报流程(检测网络→创建Socket→连接→发送)
- `Int_QS100_EnterLowPower()`: 快速关机(AT+FASTOFF=0)

**AT指令流程**:
```
AT+RB           -> 重启模块
AT+CGATT?       -> 查询网络附着状态
AT+NSOCR=STREAM,6,0,0  -> 创建TCP Socket
AT+NSOCO=0,IP,PORT     -> 连接服务器
AT+NSOSD=0,LEN,DATA    -> 发送数据
AT+FASTOFF=0    -> 快速关机
```

#### 3. DS3553 计步器 (Int_DS3553)
**文件**: `Int/DS3553/Int_DS3553.c/h`

**功能**:
- 通过I2C读取计步器寄存器
- 获取运动步数
- 配置计步器工作模式

**寄存器定义**:
```c
#define CHIP_ID      0x01   // 芯片ID(只读)
#define USER_SET     0xC3   // 用户设置(可读写)
#define STEP_CNT_L   0xC4   // 步数低字节(只读)
#define STEP_CNT_M   0xC5   // 步数中字节(只读)
#define STEP_CNT_H   0xC6   // 步数高字节(只读)
```

**USER_SET寄存器位定义**:
```c
PEDO_0     (1<<0)  // 计步使能位0
PEDO_1     (1<<1)  // 计步使能位1
CLEAR_EN   (1<<2)  // 清零使能
NOISE_DIS  (1<<3)  // 噪声抑制
PULSE_EN   (1<<4)  // 脉冲输出使能
RAISE_EN   (1<<5)  // 中断使能
sen_DIS    (1<<6)  // 灵敏度控制
PWR_MOD    (1<<7)  // 功耗模式
```

**关键函数**:
- `Int_DS3553_Init()`: 初始化,禁用脉冲输出(使用寄存器读取方式)
- `Int_DS3553_GetStepNumber()`: 读取24位步数计数器

**I2C通信**:
- 设备地址: 0x4F(读) / 0x4E(写)
- 通信前CS引脚拉低,至少3ms
- 通信后CS引脚拉高,至少10ms

#### 4. LoRa模块 (Int_Lora)
**文件**: `Int/lora/Int_Lora.c/h`

**功能**:
- LoRa无线通信初始化
- 数据发送与接收
- 同步字配置(设备身份识别)

**关键函数**:
- `Int_LoRa_Init()`: 初始化LoRa模块,设置同步字0x1234
- `Int_Lora_Send()`: 发送数据
- `Int_Lora_Receive()`: 轮询接收数据

**底层驱动**:
- 使用亿佰特(Ebyte)官方驱动库
- 通过`ebyte_conf.h`宏定义选择具体型号(EBYTE_E220_400M22S)
- 提供统一的函数指针接口(Ebyte_RF_t结构体)

#### 5. 通用组件

##### Com_Debug (调试输出)
**文件**: `Com/Com_Debug.c/h`

**功能**:
- 提供带文件名和行号的调试输出
- 支持开发/生产环境切换

**宏定义**:
```c
#define DEVELOPMENT  // 开发环境:输出调试信息
// #define PRODUCTION  // 生产环境:关闭调试输出

COM_DEBUG_LN("GPS data: %d", length);
// 输出: [App_Main.c:45] GPS data: 128\r\n
```

**实现原理**:
- 重定向`fputc()`到UART1
- 使用`__FILE__`和`__LINE__`宏获取源码位置
- 可变参数宏支持格式化输出

##### Com_Delay (延时函数)
**文件**: `Com/Com_Delay.c/h`

**功能**:
- 毫秒级延时
- 秒级延时

**关键函数**:
- `Com_Delay_Ms(uint16_t ms)`: 毫秒延时(基于HAL_Delay)
- `Com_Delay_Sec(uint16_t second)`: 秒延时

##### Com_Status (状态管理与数据处理)
**文件**: `Com/Com_Status.c/h`

**功能**:
- 定义状态枚举(COM_OK, COM_ERROR, COM_TIMEOUT)
- 定义上报数据结构(Upload_Data_T)
- UTC时间转北京时间
- 数据结构序列化为JSON

**关键函数**:
- `Com_Util_utc2beijing()`: UTC时间字符串转北京时间(+8小时)
- `Com_Util_UploadData2JsonString()`: 将Upload_Data_T转为JSON字符串

**时间转换流程**:
```
UTC字符串 -> tm结构体 -> time_t时间戳 -> +8小时 -> tm结构体 -> 北京时间字符串
```

---

## 网关端 (Gateway) 详解

### 硬件配置
- **主控芯片**: STM32F103ZET6 (72MHz, 512KB Flash, 64KB RAM)
- **LoRa模块**: E220x系列 (通过SPI1通信)
- **以太网模块**: W5500 (通过SPI2通信)
- **调试串口**: UART1 (115200波特率)

### 工作流程

```
1. 系统启动
   ↓
2. 初始化外设
   - LoRa模块初始化(设置同步字0x1234)
   - W5500以太网初始化(配置MAC、IP、网关、子网掩码)
   ↓
3. 主循环
   - 轮询接收LoRa数据
   - 延时1秒
   - 重复步骤3
```

### 核心模块说明

#### 1. W5500以太网模块 (Int_w5500)
**文件**: `Int/eth/Int_w5500.c/h`

**功能**:
- W5500硬件初始化
- 网络参数配置(MAC、IP、网关、子网掩码)
- TCP Socket管理
- 数据发送

**网络配置**:
```c
wiz_NetInfo default_net_info = {
    .mac = {0x00, 0x08, 0xdc, 0x12, 0x22, 0x12},
    .ip = {192, 168, 55, 130},
    .gw = {192, 168, 55, 1},
    .sn = {255, 255, 255, 0}
};

uint8_t dest_ip[4] = {112, 125, 89, 8};  // 服务器IP
uint16_t dest_port = 34095;              // 服务器端口
```

**关键函数**:
- `Int_W5500_Init()`: 初始化W5500,配置网络参数
- `Int_W5500_GetSocketStatus()`: 查询Socket状态机
  - SOCK_CLOSED: 关闭并重新打开Socket
  - SOCK_INIT: 连接远程服务器
  - SOCK_ESTABLISHED: 检查连接标志位
- `Int_w5500_SendData()`: 发送数据(等待连接建立后发送)

**Socket状态机**:
```
SOCK_CLOSED -> socket() -> SOCK_INIT -> connect() -> SOCK_ESTABLISHED -> send()
```

**底层驱动**:
- 使用WIZnet官方ioLibrary驱动
- 提供寄存器级操作接口
- 支持8个独立Socket通道

#### 2. LoRa模块 (Int_Lora)
**文件**: `Int/lora/Int_Lora.c/h`

**功能**: 与定位器端相同,但工作模式为接收端

**关键函数**:
- `Int_LoRa_Init()`: 初始化,设置同步字0x1234
- `Int_Lora_Receive()`: 轮询接收数据(主循环中持续调用)

**注意**: 网关端目前只接收LoRa数据,未实现数据上传逻辑(可在App层扩展)

---

## 通信协议

### LoRa通信
- **同步字**: 0x1234 (用于设备身份识别)
- **工作模式**: 轮询接收
- **数据格式**: JSON字符串
- **通信距离**: 视距环境下可达数公里

### 2G通信 (QS100)
- **协议**: TCP/IP
- **服务器**: 112.125.89.8:33057
- **数据格式**: JSON字符串
- **AT指令**: 标准 Hayes AT 指令集扩展

### 以太网通信 (W5500)
- **协议**: TCP/IP
- **服务器**: 112.125.89.8:34095
- **网络配置**: 静态IP (192.168.55.130)
- **Socket类型**: TCP Stream

---

## 低功耗管理

### 定位器端低功耗策略

#### 进入低功耗流程
```c
1. QS100快速关机 (AT+FASTOFF=0)
2. AT6558R断电 (GPS_EN引脚拉低)
3. 设置RTC闹钟 (当前时间 + 10秒)
4. 清除待机标志位 (PWR_FLAG_SB, PWR_FLAG_WU)
5. 进入STANDBY模式 (HAL_PWR_EnterSTANDBYMode)
```

#### 唤醒流程
```c
1. RTC闹钟触发,系统复位
2. QS100唤醒 (PB13引脚高脉冲)
3. AT6558R上电 (GPS_EN引脚拉高)
4. 重新开始工作流程
```

#### 功耗优化点
- GPS模块: 非工作时完全断电
- 2G模块: 使用快速关机模式
- MCU: STANDBY模式(最低功耗,仅RTC运行)
- 唤醒源: RTC闹钟(精确定时唤醒)

### 网关端低功耗
- 无特殊低功耗设计(持续供电场景)
- 主循环持续运行

---

## 依赖关系

### 硬件依赖
```
定位器端:
├── STM32F103C8T6 (主控)
├── AT6558R (GPS/北斗)
├── QS100 (2G通信)
├── DS3553 (计步器)
└── E220x (LoRa)

网关端:
├── STM32F103ZET6 (主控)
├── E220x (LoRa)
└── W5500 (以太网)
```

### 软件依赖
```
定位器端:
├── STM32 HAL库 (外设驱动)
├── cJSON库 (JSON序列化)
├── Ebyte驱动库 (LoRa)
└── 自定义驱动 (AT6558R, QS100, DS3553)

网关端:
├── STM32 HAL库 (外设驱动)
├── WIZnet ioLibrary (W5500驱动)
└── Ebyte驱动库 (LoRa)
```

### 第三方库
1. **cJSON**: 轻量级JSON解析库
   - 文件: `Com/cJSON.c/h`
   - 用途: 数据结构序列化为JSON

2. **Ebyte LoRa驱动**: 亿佰特官方驱动库
   - 文件: `Int/lora/E220xMx/`
   - 用途: LoRa模块控制

3. **WIZnet ioLibrary**: W5500官方驱动库
   - 文件: `Int/eth/W5500/`
   - 用途: 以太网通信

---

## 开发环境

### 工具链
- **IDE**: Keil MDK-ARM (μVision 5)
- **编译器**: ARM Compiler 5/6
- **调试器**: ST-Link / J-Link
- **代码生成**: STM32CubeMX

### 工程文件
- 定位器端: `location/MDK-ARM/location.uvprojx`
- 网关端: `gateway/MDK-ARM/gateway.uvprojx`

### 编译配置
- **优化等级**: Level 0 (Debug) / Level 3 (Release)
- **C标准**: C99
- **头文件路径**: 
  - Core/Inc
  - Com/
  - Int/AT6558R/
  - Int/QS100/
  - Int/DS3553/
  - Int/lora/
  - Int/eth/

### 烧录与调试
1. 使用ST-Link连接目标板
2. 在Keil中点击"Download"烧录固件
3. 使用串口助手查看调试输出(UART1, 115200)

---

## 关键配置参数

### 定位器端
```c
// GPS配置
#define GPS_BAUDRATE        9600      // GPS波特率
#define GPS_UPDATE_RATE     250       // 更新频率(ms)
#define GPS_SATELLITE_SYS   2         // 北斗卫星系统

// 服务器配置
#define SERVER_IP           "112.125.89.8"
#define SERVER_PORT         33057

// LoRa配置
#define LORA_SYNC_WORD      0x1234    // 同步字

// 低功耗配置
#define STANDBY_DURATION    10        // 待机时长(秒)

// DS3553配置
#define DS3553_I2C_ADDR     0x4F      // I2C地址
```

### 网关端
```c
// 网络配置
#define W5500_MAC           {0x00, 0x08, 0xdc, 0x12, 0x22, 0x12}
#define W5500_IP            {192, 168, 55, 130}
#define W5500_GATEWAY       {192, 168, 55, 1}
#define W5500_SUBNET        {255, 255, 255, 0}

// 服务器配置
#define SERVER_IP           {112, 125, 89, 8}
#define SERVER_PORT         34095

// LoRa配置
#define LORA_SYNC_WORD      0x1234    // 同步字(必须与定位器端一致)
```

---

## 常见问题与解决方案

### 1. GPS无法获取有效数据
**原因**:
- 天线遮挡或室内环境
- GPS模块未正确初始化
- 卫星信号弱

**解决方案**:
- 在开阔环境测试
- 检查GPS_EN引脚电压(应为3.3V)
- 检查UART2是否有数据输出
- 延长等待时间(首次定位可能需要1-2分钟)

### 2. QS100无法连接服务器
**原因**:
- SIM卡未插或欠费
- 信号弱
- 服务器地址/端口错误

**解决方案**:
- 检查SIM卡状态
- 发送`AT+CGATT?`确认网络附着
- 检查服务器IP和端口配置
- 查看AT指令返回的ERROR信息

### 3. LoRa通信失败
**原因**:
- 同步字不匹配
- 距离过远
- 天线损坏

**解决方案**:
- 确认两端同步字均为0x1234
- 在近距离测试
- 检查天线连接

### 4. 功耗过高
**原因**:
- GPS模块未断电
- QS100未进入关机模式
- MCU未进入STANDBY

**解决方案**:
- 检查GPS_EN引脚电平
- 确认发送了AT+FASTOFF=0指令
- 检查RTC闹钟是否正确设置

---

## 扩展开发指南

### 定位器端扩展

#### 1. 添加新传感器
```c
// 1. 在Int/目录创建新驱动目录
// 2. 实现Init、Read、EnterLowPower、LeaveLowPower函数
// 3. 在App_Main_Init()中调用初始化
// 4. 在App_Main_UploadData2Server()中读取数据
// 5. 在Upload_Data_T结构体中添加新字段
// 6. 在Com_Util_UploadData2JsonString()中添加JSON序列化
```

#### 2. 修改上报频率
```c
// 修改RTC闹钟时间
alarm.AlarmTime.Seconds = current_time.Seconds + 60; // 60秒上报一次
```

#### 3. 添加下行指令
```c
// 在Int_Lora_Receive()的回调函数中解析指令
// 根据指令执行相应操作(如修改配置、重启等)
```

### 网关端扩展

#### 1. 实现数据上传
```c
// 在App层创建新文件
// 1. 接收LoRa数据后缓存
// 2. 通过W5500上传到服务器
// 3. 处理服务器响应
```

#### 2. 支持多定位器
```c
// 1. 为每个定位器分配唯一ID
// 2. 在JSON数据中包含ID字段
// 3. 网关端维护定位器列表
```

#### 3. 添加DHCP支持
```c
// 使用W5500的DHCP功能
// 调用WIZnet提供的DHCP库函数
```

---

## 性能指标

### 定位精度
- **GPS**: 2.5米CEP (开阔环境)
- **更新频率**: 4次/秒(可配置)

### 通信距离
- **LoRa**: 视距环境3-5km
- **2G**: 运营商覆盖范围

### 功耗
- **工作模式**: ~100mA (GPS+2G+LoRa)
- **待机模式**: ~20uA (RTC运行)
- **电池续航**: 取决于上报频率和电池容量

### 数据格式
- **JSON大小**: ~200字节
- **上报频率**: 默认10秒/次(可配置)

---

## 版本历史

### v1.0.0 (当前版本)
- 实现基本定位功能
- 支持GPS/北斗定位
- 支持2G网络上传
- 支持LoRa备用通信
- 实现低功耗管理
- 网关端接收LoRa数据

---

## 参考资料

### 数据手册
- STM32F103C8T6: [STM32F103x8 Datasheet](https://www.st.com/resource/en/datasheet/stm32f103c8.pdf)
- AT6558R: [AT6558R GPS Module Manual](https://www.at-shop.com/at6558r)
- QS100: [QS100 2G Module AT Command Manual](https://www.quectel.com/product/gsm-qs100)
- DS3553: [DS3553 Pedometer Datasheet](https://www.maximintegrated.com/en/products/sensors/DS3553.html)
- W5500: [W5500 Ethernet Controller Datasheet](https://www.wiznet.io/product-item/w5500/)

### 开发工具
- STM32CubeMX: [STM32CubeMX](https://www.st.com/en/development-tools/stm32cubemx.html)
- Keil MDK-ARM: [Keil MDK](https://www.keil.com/mdk5)
- STM32 HAL库: [STM32 HAL Driver](https://www.st.com/en/embedded-software/stm32cube-f1-mcu-package.html)

### 第三方库
- cJSON: [cJSON GitHub](https://github.com/DaveGamble/cJSON)
- WIZnet ioLibrary: [WIZnet ioLibrary](https://github.com/Wiznet/W5500_EVB)
- Ebyte LoRa: [Ebyte Official Website](https://www.ebyte.com/)

---

## 联系与支持

如有问题或建议,请联系开发团队。

---

**文档生成日期**: 2026-07-27  
**文档版本**: v1.0.0  
**项目版本**: v1.0.0
