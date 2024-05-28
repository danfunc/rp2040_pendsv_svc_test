#include <cinttypes>
#include <hardware/exception.h>
#include <memory>
#include <pico/stdio.h>
#include <pico/stdlib.h>
#include <queue>
#include <stdio.h>
#include <stdlib.h>
using method = void (*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t);

enum struct exc_return : uint32_t {
  handler_MSP = 0xFFFFFFF1,
  thread_MSP = 0xFFFFFFF9,
  thread_PSP = 0xFFFFFFFD,
};
struct exception_stack {
  uint32_t r0, r1, r2, r3, r12;
  void (*lr)(), (*pc)();
  uintptr_t xPSR;
};
using exec_mode = exc_return;

void target(size_t count) {
  for (size_t i = 0; i < count; i++) {
    printf("success\n");
    sleep_ms(1000);
  }
}
void trap() {
  while (1) {
    printf("trapped\n");
    *(uintptr_t *)(PPB_BASE + M0PLUS_ICSR_OFFSET) |= 1 << 28;
    sleep_ms(1000);
  }
}
int main(int argc, char const *argv[]);
void context_switch() __attribute__((__noinline__));

struct context {
  uint32_t r4 = 0, r5 = 0, r6 = 0, r7 = 0, r8 = 0, r9 = 0, r10 = 0, r11 = 0;
  void *sp;
  void const *stack_start;
  context(method method, size_t arg0, size_t arg1, size_t arg2, size_t arg3,
          size_t stack_size = 1024)
      : stack_start{malloc(stack_size + sizeof(exception_stack))} {
    exception_stack return_stack;
    return_stack.pc = (void (*)())method;
    return_stack.r0 = arg0;
    return_stack.r1 = arg1;
    return_stack.r2 = arg2;
    return_stack.r3 = arg3;
    return_stack.lr = trap;
    return_stack.xPSR = (1 << 24);
    if (stack_start != nullptr) {
      sp = (void *)(((uintptr_t)stack_start + stack_size) & ~0xfL);
      *(exception_stack *)sp = return_stack;
    } else
      sp = nullptr;
  };
  context() : stack_start{nullptr} {};
  ~context() {
    if (stack_start != nullptr)
      free((void *)stack_start);
  };

  void __always_inline save_from_r4_to_r11() {
    uint32_t r4, r5, r6, r7, r8, r9, r10, r11, r12;
    asm volatile("mov %0, r4" : "=r"(r4));
    this->r4 = r4;
    asm volatile("mov %0, r5" : "=r"(r5));
    this->r5 = r5;
    asm volatile("mov %0, r6" : "=r"(r6));
    this->r6 = r6;
    asm volatile("mov %0, r7" : "=r"(r7));
    this->r7 = r7;
    asm volatile("mov %0, r8" : "=r"(r8));
    this->r8 = r8;
    asm volatile("mov %0, r9" : "=r"(r9));
    this->r9 = r9;
    asm volatile("mov %0, r10" : "=r"(r10));
    this->r10 = r10;
    asm volatile("mov %0, r11" : "=r"(r11));
    this->r11 = r11;
  };

  void __always_inline load_from_r4_to_r11() {
    uint32_t r4, r5, r6, r7, r8, r9, r10, r11, r12;
    this->r4 = r4;
    asm volatile("mov r4, %0" ::"r"(r4));
    this->r5 = r5;
    asm volatile("mov r5, %0" ::"r"(r5));
    this->r6 = r6;
    asm volatile("mov r6, %0" ::"r"(r6));
    this->r7 = r7;
    asm volatile("mov r7, %0" ::"r"(r7));
    this->r8 = r8;
    asm volatile("mov r8, %0 " ::"r"(r8));
    this->r9 = r9;
    asm volatile("mov r9, %0 " ::"r"(r9));
    this->r10 = r10;
    asm volatile("mov r10, %0" ::"r"(r10));
    this->r11 = r11;
    asm volatile("mov r11, %0" ::"r"(r11));
  };
};

extern "C" {
context *get_current_context() __noinline;
context *pop_next_context() __noinline;
context *load_from_r4_to_r11(context *t);
context *save_from_r4_to_r11(context *t);
void pendsv_handler(void);
}

context *save_from_r4_to_r11(context *t) {
  uint32_t r4, r5, r6, r7, r8, r9, r10, r11, r12;
  asm volatile("mov %0, r4" : "=r"(r4));
  t->r4 = r4;
  asm volatile("mov %0, r5" : "=r"(r5));
  t->r5 = r5;
  asm volatile("mov %0, r6" : "=r"(r6));
  t->r6 = r6;
  asm volatile("mov %0, r7" : "=r"(r7));
  t->r7 = r7;
  asm volatile("mov %0, r8" : "=r"(r8));
  t->r8 = r8;
  asm volatile("mov %0, r9" : "=r"(r9));
  t->r9 = r9;
  asm volatile("mov %0, r10" : "=r"(r10));
  t->r10 = r10;
  asm volatile("mov %0, r11" : "=r"(r11));
  t->r11 = r11;
  return t;
};

context *load_from_r4_to_r11(context *t) {
  uint32_t r4 = t->r4, r5 = t->r5, r6 = t->r6, r7 = t->r7, r8 = t->r8,
           r9 = t->r9, r10 = t->r10, r11 = t->r11;
  asm volatile("mov r4, %0" ::"r"(r4));
  asm volatile("mov r5, %0" ::"r"(r5));
  asm volatile("mov r6, %0" ::"r"(r6));
  asm volatile("mov r7, %0" ::"r"(r7));
  asm volatile("mov r8, %0 " ::"r"(r8));
  asm volatile("mov r9, %0 " ::"r"(r9));
  asm volatile("mov r10, %0" ::"r"(r10));
  asm volatile("mov r11, %0" ::"r"(r11));
  return t;
};

class cpu_manager;

int main(int argc, const char *argv[]);
void cpp_context_switch();
class rp2040 {
  friend cpu_manager;
  friend context *get_current_context();
  friend context *pop_next_context();
  friend void context_switch();
  friend void cpp_context_switch();
  friend int main(int, const char *[]);
  inline static context *get_current_context();
  inline static context *pop_next_context();
  static cpu_manager *cpu_managers[2];
  rp2040(cpu_manager &manager, size_t core_num);
};

struct task;

class cpu_manager {
  friend rp2040;
  friend int main(int, const char **);
  context *pop_next_context();
  context *get_current_context();
  std::shared_ptr<context> current_context, default_context;
  rp2040 driver;
  cpu_manager(size_t core_num) : driver{*this, core_num} {
    current_context = std::make_shared<context>();
    default_context = std::make_shared<context>((method)target, 0, 0, 0, 0);
  };
};

cpu_manager *rp2040::cpu_managers[2] = {nullptr, nullptr};

rp2040::rp2040(cpu_manager &manager, size_t core_num) {
  if (cpu_managers[core_num] == nullptr) {
    cpu_managers[core_num] = &manager;
  }
}

context *rp2040::get_current_context() {
  return cpu_managers[get_core_num()]->get_current_context();
}

context *rp2040::pop_next_context() {
  return cpu_managers[get_core_num()]->pop_next_context();
}

context *cpu_manager::get_current_context() { return current_context.get(); }

context *cpu_manager::pop_next_context() {
  current_context = std::make_shared<context>((method)target, 10, 0, 0, 0);
  return current_context.get();
}

context *get_current_context() { return rp2040::get_current_context(); };
context *pop_next_context() { return rp2040::pop_next_context(); };

int main(int argc, char const *argv[]) {
  exc_return a = exc_return::thread_PSP;
  stdio_init_all();
  cpu_manager manager{0};
  exception_set_exclusive_handler(PENDSV_EXCEPTION, pendsv_handler);
  while (1) {
    *(uintptr_t *)(PPB_BASE + M0PLUS_ICSR_OFFSET) |= 1 << 28;
    sleep_ms(1000);
    printf("pendsv_returnd\n");
  }
  return 0;
}
