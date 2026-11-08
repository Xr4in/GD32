# 基于 GD32F303RCT6 与 FreeRTOS 的 Modbus RTU 远程 I/O 终端

## 1. 项目定位

本项目面向 MCU + RTOS 方向的嵌入式软件岗位，目标是在较短时间内完成一个可演示、可测试、可在面试中展开讲解的完整嵌入式项目。

项目不以堆叠功能为目标，而是集中展示以下能力：

- MCU 外设驱动开发。
- FreeRTOS 多任务、队列、互斥量和看门狗。
- UART DMA、空闲中断和环形缓冲区。
- RS485 半双工通信。
- Modbus RTU 从站协议。
- LVGL 与 ILI9341/XPT2046 的移植和线程使用。
- 异常处理、压力测试和工程文档。

项目名称：

> 基于 GD32F303RCT6 与 FreeRTOS 的 Modbus RTU 远程 I/O 终端

项目本质：

```text
电脑、PLC 或 Modbus 主机
        │
        │ USB转RS485
        ▼
     RS485 A/B 总线
        │
        ▼
MAX3485/SP3485
        │
        ▼
GD32F303RCT6 + FreeRTOS + LVGL
```

GD32 作为 Modbus RTU 从站，对外提供模拟量、数字量和输出控制能力，同时在本地屏幕上显示运行状态。

## 2. 第一版功能范围

### 2.1 必须实现

- 两路 ADC 模拟量采集。
- 两路数字输入。
- 两路数字输出。
- Modbus RTU 从站通信。
- 一个 LVGL 本地监控页面。
- 触摸按钮控制两路输出。
- Modbus 主机远程控制两路输出。
- 输出状态在本地触摸和远程写入之间保持同步。
- 从站地址和报警阈值保存在内部 Flash。
- 通信错误计数和运行状态显示。
- FreeRTOS 任务心跳和独立看门狗。

### 2.2 暂时不做

- ESP8266、Wi-Fi 和 MQTT。
- W25Q64 和离线历史日志。
- Modbus 主机轮询。
- 多页面 UI、图表、动画和图片。
- 复杂文件系统和 Bootloader。
- 工业级浪涌、隔离和 EMC 设计。

W25Q64、Wi-Fi 和 MQTT 应作为完成第一版后的扩展，不进入当前关键路径。

## 3. 最终演示效果

1. 电脑通过 USB 转 RS485 连接 GD32。
2. 使用 Modbus Poll、QModMaster 或 Python 脚本读取 ADC 和输入状态。
3. 主机发送 Modbus 命令，改变从站地址或报警阈值。
4. 主机发送 `0x05`，控制两路输出。
5. 在触摸屏上按下输出按钮，本地状态立即改变。
6. 再次通过 Modbus 读取，确认远程和本地看到的输出状态一致。
7. 发送错误 CRC、短帧、非法地址和未知功能码，节点不越界且返回正确结果。
8. 断开并重新连接 RS485，从站自动恢复接收。
9. 长时间运行不出现复位、内存泄漏和任务栈溢出。

## 4. 硬件方案

### 4.1 硬件清单

| 器材 | 数量 | 说明 |
|---|---:|---|
| GD32F303RCT6 开发板 | 1 | 已有 |
| 2.4 寸 ILI9341 + XPT2046 屏 | 1 | 已有 |
| ST-Link/DAPLink | 1 | 已有 |
| USB 转 TTL | 1 | 调试串口 |
| USB 转 RS485 | 1 | 电脑侧 Modbus 主机 |
| MAX3485/SP3485 模块 | 1 | 3.3V，必须引出 DE/RE |
| 10k 电位器 | 2 | 模拟量输入 |
| 轻触按键 | 2 | 数字量输入 |
| LED 或光耦模块 | 2 | 输出演示 |
| 120Ω 电阻 | 2 | RS485 总线终端 |
| 杜邦线、端子和电源 | 若干 | 共地、供电和接线 |
| 8 通道逻辑分析仪 | 1 | 强烈推荐 |

### 4.2 推荐引脚分配

| 功能 | GD32 引脚 | 说明 |
|---|---|---|
| LCD SPI0 SCK | PA5 | 与 XPT2046 共用 |
| LCD SPI0 MISO | PA6 | 与 XPT2046 共用 |
| LCD SPI0 MOSI | PA7 | 与 XPT2046 共用 |
| LCD CS | PB6 | 低有效 |
| LCD DC/RS | PB7 | 命令/数据选择 |
| LCD RESET | PA11 | 复位 |
| LCD 背光 | PA8 | 高电平点亮 |
| 触摸 CS | PB8 | 低有效 |
| RS485 DI | PA2 | USART1 TX |
| RS485 RO | PA3 | USART1 RX |
| RS485 DE/RE | PB0 | 高电平发送 |
| 调试 UART0 TX | PA9 | USB-TTL RX |
| 调试 UART0 RX | PA10 | USB-TTL TX |
| ADC0 | PC0 | 电位器 1 |
| ADC1 | PC1 | 电位器 2 |
| DI0 | PC2 | 按键 1 |
| DI1 | PC3 | 按键 2 |
| DO0 | PC4 | LED/光耦 1 |
| DO1 | PC5 | LED/光耦 2 |
| 板载状态 LED | PA0、PA1 | 保留现有 LED |
| 预留 ESP8266 TX | PB10 | 后续 USART2 |
| 预留 ESP8266 RX | PB11 | 后续 USART2 |

接线前必须核对开发板原理图，确认上述引脚实际引出且没有板载功能冲突。

### 4.3 RS485 接线

```text
GD32 PA2 TX  ─────── DI
GD32 PA3 RX  ─────── RO
GD32 PB0     ─────── DE
                 └── /RE

收发器 A ─────────── 总线 A
收发器 B ─────────── 总线 B
```

- A 接 A，B 接 B，不要交叉。
- DE 和 `/RE` 可以短接后由 PB0 控制。
- 总线距离较长时，在首尾两端各接一个 120Ω 电阻。
- 所有 RS485 设备和调试设备必须共地。
- 不能把 MCU 的 USART TX/RX 直接接到 A/B 差分线上。

### 4.4 屏幕接线

显示屏与触摸控制器共用 SPI0：

```text
LCD SCK  ─┬─ T_CLK
LCD MOSI ─┼─ T_DIN
LCD MISO ─┴─ T_DO
LCD CS   ─── LCD 片选
TP CS    ─── 触摸片选
```

同一时刻只能选中一个设备：

```text
刷新 LCD：LCD_CS = 0，TP_CS = 1
读取触摸：LCD_CS = 1，TP_CS = 0
```

第一版不使用 T_IRQ，通过周期读取 XPT2046 压力值判断触摸是否按下。

## 5. 软件架构

### 5.1 开发环境

- IDE：Keil MDK。
- MCU 库：GD32F30x 标准外设库。
- RTOS：FreeRTOS 10.3.1，ARM Cortex-M4F 移植。
- RTOS API：FreeRTOS 原生 API，不使用 CMSIS-RTOS V2 封装。
- GUI：LVGL 8.3.11。
- 屏幕：ILI9341，RGB565，横屏 320×240。
- 触摸：XPT2046。

### 5.2 任务划分

| 任务 | 优先级 | 栈大小 | 主要职责 |
|---|---:|---:|---|
| MonitorTask | 4 | 256 words | 检查任务心跳并喂看门狗 |
| AcqTask | 3 | 384 words | ADC、数字输入和输出状态维护 |
| ModbusTask | 3 | 512 words | 帧接收、解析、寄存器和应答 |
| UITask | 2 | 1024 words | LVGL、LCD、触摸和页面刷新 |

MonitorTask 不得无条件喂狗。只有采集、Modbus 和 UI 任务的心跳都在有效时间内，才允许刷新独立看门狗。

### 5.3 数据流

```text
ADC/GPIO
   │
   ▼
AcqTask
   │
   ├── 最新数据覆盖队列 ──────► UITask
   │
   └── 共享输出状态 ──互斥量──► ModbusTask

USART1 DMA/IDLE
   │
   ▼
ModbusTask
   │
   └── USART1 + DE 控制 ─────► Modbus 主机
```

### 5.4 队列与同步

- UI 最新数据使用长度为 1 的覆盖队列，只保留最新采样结果。
- Modbus 帧接收使用固定长度队列，避免在中断内解析。
- 输出状态使用互斥量保护，因为 UI 和 ModbusTask 都可能修改。
- Modbus 应答缓冲区只由 ModbusTask 使用。
- LVGL API 只允许在 UITask 中调用。

## 6. FreeRTOS 配置

建议初始配置：

```c
#define configTICK_RATE_HZ                1000
#define configMAX_PRIORITIES              8
#define configUSE_PREEMPTION              1
#define configUSE_MUTEXES                 1
#define configUSE_COUNTING_SEMAPHORES     1
#define configCHECK_FOR_STACK_OVERFLOW    2
#define configUSE_MALLOC_FAILED_HOOK      1
#define configUSE_TRACE_FACILITY          1
#define configUSE_TICKLESS_IDLE           0
#define configTOTAL_HEAP_SIZE             (14 * 1024)
```

所有调用 FreeRTOS API 的中断，其 NVIC 优先级数值必须不小于：

```c
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5
```

中断中只能使用 `xQueueSendFromISR()`、`xSemaphoreGiveFromISR()`、`vTaskNotifyGiveFromISR()` 等 ISR 安全接口。

## 7. UART DMA 与 Modbus 帧接收

### 7.1 接收流程

1. USART1 配置为 9600、8 数据位、无校验、1 停止位。
2. DMA 循环接收字节到固定缓冲区。
3. USART 空闲中断产生后，计算上一帧新增数据。
4. 将新增数据写入软件环形缓冲区。
5. 使用定时器判断 3.5 字符间隔。
6. 完整帧通过队列交给 ModbusTask。
7. 在任务中完成 CRC、地址、功能码和长度校验。

不要像待修正的示例代码一样直接在中断中解析协议或发送应答。

### 7.2 RS485 发送方向

```text
发送前：DE = 1
写入全部字节
等待 USART_FLAG_TC
发送完成后：DE = 0
```

必须等待发送完成标志 `TC`，不能只等待发送缓冲区空标志 `TBE`，否则可能截断最后一个字节。

## 8. Modbus RTU 从站设计

### 8.1 功能码

第一版只实现以下功能码：

| 功能码 | 功能 |
|---|---|
| `0x01` | 读取线圈 |
| `0x03` | 读取保持寄存器 |
| `0x05` | 写单个线圈 |
| `0x06` | 写单个保持寄存器 |

### 8.2 寄存器表

| 类型 | 地址 | 名称 | 说明 |
|---|---:|---|---|
| 保持寄存器 | `0x0000` | ADC0 | 12 位原始值 |
| 保持寄存器 | `0x0001` | ADC1 | 12 位原始值 |
| 保持寄存器 | `0x0002` | DI | 两路数字输入位图 |
| 保持寄存器 | `0x0003` | DO | 两路输出状态 |
| 保持寄存器 | `0x0004` | 有效帧计数 | 饱和计数 |
| 保持寄存器 | `0x0005` | CRC 错误计数 | 饱和计数 |
| 保持寄存器 | `0x0010` | 报警阈值 | 可读写 |
| 保持寄存器 | `0x0011` | 从站地址 | 可读写，重启后生效 |
| 线圈 | `0x0000` | DO0 | 输出 1 |
| 线圈 | `0x0001` | DO1 | 输出 2 |

### 8.3 帧处理规则

- 从站地址不匹配时，不应答。
- CRC 错误时，不应答。
- 帧长度不足或过长时，不应答。
- 非法功能码返回异常码 `0x01`。
- 非法寄存器或线圈地址返回异常码 `0x02`。
- 非法数据值返回异常码 `0x03`。
- 广播地址 `0` 的写命令执行，但不应答。
- 读命令必须使用合法从站地址。

### 8.4 参数保存

- 内部 Flash 保存从站地址和报警阈值。
- 配置区包含魔术字、版本、长度、配置内容和 CRC。
- 从站地址修改后先保存，并提示“重启后生效”。
- 报警阈值修改后立即生效并保存。
- 掉电恢复时 CRC 无效则回到默认地址 `1` 和默认阈值。

## 9. LVGL 显示设计

### 9.1 单页布局

```text
┌────────────────────────────────┐
│ Modbus Remote I/O              │
├────────────────┬───────────────┤
│ ADC1: 0000     │ ADC2: 0000    │
│ DI1: ON        │ DI2: OFF      │
├────────────────┴───────────────┤
│ [Output 1: ON ] [Output 2: OFF]│
├────────────────────────────────┤
│ Address: 01   Frames: 0000     │
│ Errors: 0000  Uptime: 00:00:00 │
└────────────────────────────────┘
```

### 9.2 页面规则

- 只创建一个页面。
- 只使用 Label、Button、Bar、Obj 和 Default Theme。
- 中文只使用少量固定标签，通过 `lv_font_conv` 生成子集字体。
- 数值和英文使用 Montserrat 14。
- 数据没有变化时不刷新文本。
- 触摸按钮只修改输出请求，不直接操作 GPIO。

### 9.3 内存配置

```c
#define LV_COLOR_DEPTH      16
#define LV_COLOR_16_SWAP    1
#define LV_MEM_SIZE         (8 * 1024)
```

绘制缓冲使用单缓冲：

```c
static lv_color_t draw_buf[320 * 10];
```

该缓冲约占 `320 × 10 × 2 = 6400` 字节。

关闭以下功能以节省资源：

- 图片和图片解码器。
- Canvas、Chart、Calendar、Keyboard、Colorwheel。
- GIF、PNG、文件系统。
- Widget Demo 和 Benchmark。
- 不必要的主题、动画和字体。

### 9.4 LVGL 线程规则

- 所有 `lv_*()` 调用都在 UITask 中执行。
- FreeRTOS Tick Hook 调用 `lv_tick_inc(1)`。
- UITask 周期调用 `lv_timer_handler()`，建议间隔 5～10ms。
- 其他任务只发送队列或事件，不直接修改 LVGL 对象。

## 10. 建议工程结构

```text
APP/
  app_main.c
  app_tasks.c
  app_state.c
BSP/
  bsp_clock.c
  bsp_gpio.c
  bsp_lcd.c
  bsp_touch.c
  bsp_usart.c
  bsp_flash.c
  bsp_watchdog.c
DRIVER/
  ili9341.c
  xpt2046.c
PROTOCOL/
  modbus_crc.c
  modbus_rtu.c
  modbus_regmap.c
UI/
  ui_main.c
  ui_event.c
  font_zh_12.c
```

分层原则：

- `DRIVER` 不包含业务逻辑。
- `BSP` 只负责 GD32 外设。
- `PROTOCOL` 不直接调用 GPIO 或 USART。
- `APP` 负责任务和数据流。
- `UI` 只通过队列和状态接口获取数据。

## 11. 12 天实施计划

按每天 6～8 小时安排。每天只能投入 2～3 小时时，应扩展到三周。

| 天数 | 工作内容 | 验收标准 |
|---|---|---|
| 第 1 天 | FreeRTOS 移植、任务、队列 | 两个任务稳定运行 |
| 第 2 天 | 调试串口、栈水位、互斥量 | 串口输出任务和内存状态 |
| 第 3 天 | ILI9341 SPI 驱动 | 全屏填色和横屏正常 |
| 第 4 天 | LVGL 显示移植 | 文字、矩形和按钮显示正确 |
| 第 5 天 | XPT2046 触摸 | 点击按钮能够改变界面状态 |
| 第 6 天 | ADC、DI 和单页 UI | 数据能够实时更新 |
| 第 7 天 | USART1 和 RS485 方向控制 | PC 能收到原始应答 |
| 第 8 天 | DMA、空闲中断和环形缓冲区 | 完整接收可变长度帧 |
| 第 9 天 | CRC16、`0x03`、`0x06` | Modbus Poll 能读寄存器 |
| 第 10 天 | `0x01`、`0x05` 和异常响应 | 远程和触摸都能控制输出 |
| 第 11 天 | 参数保存、看门狗和错误恢复 | 掉电参数恢复，任务卡死可复位 |
| 第 12 天 | 异常帧、压力测试和文档 | 完成测试记录与演示视频 |

## 12. 测试计划

### 12.1 功能测试

- 读取 ADC 和数字输入。
- 通过触摸控制两路输出。
- 通过 Modbus 控制两路输出。
- 读取输出状态，验证与本地显示一致。
- 修改阈值并重新上电验证保存结果。

### 12.2 协议异常测试

- 错误 CRC。
- 从站地址不匹配。
- 只有帧头没有完整数据。
- 超长帧。
- 未知功能码。
- 非法寄存器地址。
- 两条帧粘在一起。
- 一条帧被拆成多次发送。
- 广播写命令。

### 12.3 稳定性测试

- 连续发送至少 10000 个合法请求。
- 验证错误帧不会导致数组越界和复位。
- 断开 RS485 一段时间后重新连接，从站应自动恢复。
- 人为挂起 ModbusTask 或 AcqTask，看门狗应在设定时间内复位。
- 连续运行至少 24 小时。
- 记录各任务的栈高水位，保留至少 25% 余量。
- 记录 FreeRTOS 堆剩余量，不应持续下降。

## 13. 简历模板

```text
基于 GD32F303RCT6 与 FreeRTOS 的 Modbus RTU 远程 I/O 终端
技术栈：C、GD32F303RCT6、FreeRTOS、USART、DMA、ADC、
       RS485、Modbus RTU、CRC16、LVGL、ILI9341、XPT2046

- 设计采集、Modbus通信、UI和监控四任务架构，通过覆盖队列传递
  最新采样数据，使用互斥量保护远端与本地共同访问的输出状态。
- 基于 USART DMA、空闲中断和环形缓冲区实现 Modbus RTU 帧接收，
  使用定时器判断 3.5 字符帧间隔，处理半帧、粘包和异常数据。
- 实现 Modbus RTU 从站寄存器和线圈映射，支持 0x01/0x03/0x05/0x06、
  CRC16 校验、非法地址和非法功能码异常响应。
- 基于 ILI9341 与 XPT2046 移植 LVGL 单页监控界面，实现 ADC、
  输入状态、输出状态和通信错误实时显示，触摸与 Modbus 控制保持同步。
- 实现任务心跳和独立看门狗，连续通信测试 [X] 小时无死机，
  实测最大响应时间 [X] ms。
```

所有性能数字必须来自真实测试，不填写未经测量的数据。

## 14. 面试准备重点

必须能够解释：

1. 为什么这个项目需要 FreeRTOS，而不是裸机主循环。
2. 任务优先级如何确定，是否存在高优先级任务长期占用 CPU。
3. 中断如何通知任务，为什么不在中断中解析完整 Modbus 帧。
4. 队列、信号量、互斥量和任务通知分别解决什么问题。
5. 为什么 UI 和 Modbus 共同访问输出状态时需要互斥量。
6. DMA 循环模式和空闲中断如何配合。
7. 软件环形缓冲区的读写指针如何避免覆盖。
8. 为什么 RS485 切换方向前必须等待 TC。
9. Modbus CRC 不正确、地址不匹配和功能码非法时分别如何处理。
10. 看门狗如何避免被错误的空闲任务或高频循环无条件喂狗。
11. 如何通过栈高水位和堆剩余量判断 FreeRTOS 资源是否足够。
12. LVGL 为什么只能在一个任务中调用。

## 15. 后续扩展顺序

完成并稳定第一版后，按以下顺序扩展：

1. 增加 W25Q64 配置备份和运行日志。
2. 增加 W25Q64 待上传数据缓存。
3. 使用 PB10/PB11 增加 ESP8266。
4. 增加 Wi-Fi 配网和断线重连。
5. 增加 MQTT 上报和缓存补传。
6. 增加 OTA 或 Bootloader。

每一步完成后都必须重新测试原功能，避免后续扩展破坏已经稳定的 Modbus、RTOS 和 LVGL 主链路。
