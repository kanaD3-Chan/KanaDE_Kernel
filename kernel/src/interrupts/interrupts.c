#include "interrupts/interrupts.h"
#include "fb_console/fb_console.h"
#include <stdint.h>

// CPU 异常名称对照表
static const char *exception_names[32] = {"Division By Zero",
                                          "Debug",
                                          "Non Maskable Interrupt",
                                          "Breakpoint",
                                          "Into Detected Overflow",
                                          "Out of Bounds",
                                          "Invalid Opcode",
                                          "No Coprocessor",
                                          "Double Fault",
                                          "Coprocessor Segment Overrun",
                                          "Bad TSS",
                                          "Segment Not Present",
                                          "Stack Fault",
                                          "General Protection Fault",
                                          "Page Fault",
                                          "Unknown Interrupt",
                                          "Coprocessor Fault",
                                          "Alignment Check",
                                          "Machine Check",
                                          "SIMD Floating-Point Exception",
                                          "Virtualization Exception",
                                          "Control Protection Exception",
                                          "Reserved",
                                          "Reserved",
                                          "Reserved",
                                          "Reserved",
                                          "Reserved",
                                          "Reserved",
                                          "Hypervisor Injection Exception",
                                          "VMM Communication Exception",
                                          "Security Exception",
                                          "Reserved"};

// 统一的 Panic 处理函数
void panic_handler(struct interrupt_frame *frame, uint64_t int_no,
                   uint64_t err_code, const char *msg) {
  fb_print_str("\n==================================================\n");
  fb_print_str("KERNEL PANIC: ");
  fb_print_str(exception_names[int_no]);
  fb_print_str("\n==================================================\n");

  fb_print_str("Vector:    ");
  fb_print_hex(int_no);
  fb_print_str("\n");
  fb_print_str("Error Code:");
  fb_print_hex(err_code);
  fb_print_str("\n\n");

  fb_print_str("RIP:       ");
  fb_print_hex(frame->rip);
  fb_print_str("\n");
  fb_print_str("CS:        ");
  fb_print_hex(frame->cs);
  fb_print_str("\n");
  fb_print_str("RFLAGS:    ");
  fb_print_hex(frame->rflags);
  fb_print_str("\n");
  fb_print_str("RSP:       ");
  fb_print_hex(frame->rsp);
  fb_print_str("\n");
  fb_print_str("SS:        ");
  fb_print_hex(frame->ss);
  fb_print_str("\n\n");

  // 如果是 Page Fault (14)，CR2 寄存器会保存导致缺页的虚拟地址！这非常关键！
  if (int_no == 14) {
    uint64_t cr2;
    __asm__ volatile("mov %%cr2, %0" : "=r"(cr2));
    fb_print_str("CR2 (Fault Address): ");
    fb_print_hex(cr2);
    fb_print_str("\n");
  }

  fb_print_str("\nSystem Halted.\n");

  // 关中断并死循环停机
  while (1) {
    __asm__ volatile("cli; hlt");
  }
}

// 宏：用于生成【没有】错误码的异常 ISR
#define ISR_NO_ERR(vector, name)                                               \
  __attribute__((interrupt)) void isr_##vector(                                \
      struct interrupt_frame *frame) {                                         \
    panic_handler(frame, vector, 0, name);                                     \
  }

// 宏：用于生成【有】错误码的异常 ISR
#define ISR_ERR(vector, name)                                                  \
  __attribute__((interrupt)) void isr_##vector(struct interrupt_frame *frame,  \
                                               uint64_t error_code) {          \
    panic_handler(frame, vector, error_code, name);                            \
  }

// 按照 x86_64 规范生成 0~31 号异常处理函数
ISR_NO_ERR(0, "Divide by Zero")
ISR_NO_ERR(1, "Debug")
ISR_NO_ERR(2, "Non-Maskable Interrupt")
ISR_NO_ERR(3, "Breakpoint")
ISR_NO_ERR(4, "Overflow")
ISR_NO_ERR(5, "Bound Range Exceeded")
ISR_NO_ERR(6, "Invalid Opcode")
ISR_NO_ERR(7, "Device Not Available")
ISR_ERR(8, "Double Fault") // 有错误码
ISR_NO_ERR(9, "Coprocessor Segment Overrun")
ISR_ERR(10, "Invalid TSS")              // 有错误码
ISR_ERR(11, "Segment Not Present")      // 有错误码
ISR_ERR(12, "Stack-Segment Fault")      // 有错误码
ISR_ERR(13, "General Protection Fault") // 有错误码
ISR_ERR(14, "Page Fault")               // 有错误码
ISR_NO_ERR(15, "Reserved")
ISR_NO_ERR(16, "x87 Floating-Point Exception")
ISR_ERR(17, "Alignment Check") // 有错误码
ISR_NO_ERR(18, "Machine Check")
ISR_NO_ERR(19, "SIMD Floating-Point Exception")
ISR_NO_ERR(20, "Virtualization Exception")
ISR_ERR(21, "Control Protection Exception") // 有错误码
ISR_NO_ERR(22, "Reserved")
ISR_NO_ERR(23, "Reserved")
ISR_NO_ERR(24, "Reserved")
ISR_NO_ERR(25, "Reserved")
ISR_NO_ERR(26, "Reserved")
ISR_NO_ERR(27, "Reserved")
ISR_NO_ERR(28, "Hypervisor Injection Exception")
ISR_ERR(29, "VMM Communication Exception") // 有错误码
ISR_ERR(30, "Security Exception")          // 有错误码
ISR_NO_ERR(31, "Reserved")
