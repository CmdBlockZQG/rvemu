# RVEMU
RISC-V全系统模拟器，C++实现

- RISC-V 32 IMAC
- zicsr zifencei
- 支持S模式和U模式
- 只支持单Hart
- 中断只由CLINT和PLIC提供
- 不支持指令集功能选择，misa只读，写入被忽略
- 仅支持小端序，mstatus.MBE/SBE/UBE均硬编码为0
- U模式执行WFI将立即触发非法指令异常，mstatus.TW可写入但被忽略
- mconfigptr硬编码为0
- 性能监视器, xenvcfg, mseccfg, PMP未实现
- wip实现为nop

