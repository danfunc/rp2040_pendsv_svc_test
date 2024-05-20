#include <cinttypes>
#include <exception>
#include <hardware/exception.h>
#include <memory>
#include <pico/stdio.h>
#include <pico/stdlib.h>
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

void target() {
  for (size_t i = 0; i < 5; i++) {
    printf("success\n");
    sleep_ms(1000);
  }
}
void trap() {
  while (1) {
    printf("trapped\n");
    sleep_ms(1000);
  }
}
int main(int argc, char const *argv[]);

struct context {
  uint32_t r4 = 0, r5 = 0, r6 = 0, r7 = 0, r8 = 0, r9 = 0, r10 = 0, r11 = 0;
  void const *stack_start;
  void *sp;
  exec_mode mode = exec_mode::thread_MSP;
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
      ;
  };
  context() : stack_start{nullptr} {};
  ~context() {
    if (stack_start != nullptr)
      free((void *)stack_start);
  };
};

class cpu_manager;

int main(int argc, const char *argv[]);

class rp2040 {
  friend cpu_manager;
  friend int main(int, const char *[]);
  static std::shared_ptr<context> current_contexts[2];
  static cpu_manager *cpu_managers[2];
  static void pendsv_handler();
  rp2040(cpu_manager &manager, size_t core_num);
};

class cpu_manager {
  friend rp2040;
  friend int main(int, const char **);
  std::shared_ptr<context> pop_next_context();
  rp2040 driver;
  cpu_manager(size_t core_num) : driver{*this, core_num} {};
};

cpu_manager *rp2040::cpu_managers[2] = {nullptr, nullptr};
std::shared_ptr<context> rp2040::current_contexts[2] = {
    {std::make_shared<context>()}, {std::make_shared<context>()}};

rp2040::rp2040(cpu_manager &manager, size_t core_num) {
  cpu_managers[core_num] = &manager;
}

std::shared_ptr<context> cpu_manager::pop_next_context() {
  return std::make_shared<context>((method)target, 0, 0, 0, 0);
}

void rp2040::pendsv_handler() {
  int core_num = get_core_num();
  if (current_contexts[core_num]) {
  }
  current_contexts[core_num] = cpu_managers[core_num]->pop_next_context();
  void *sp = current_contexts[core_num]->sp;
  asm volatile("msr PSP,%0" ::"r"(sp));
  exc_return return_code = exc_return::thread_PSP;
  asm volatile("bx %0" ::"r"(return_code));
}
int main(int argc, char const *argv[]) {
  stdio_init_all();
  cpu_manager manager{0};
  // asm volatile("msr CONTROL, %0" ::"r"(SP_SEL_VAL));
  exception_set_exclusive_handler(PENDSV_EXCEPTION, rp2040::pendsv_handler);
  while (1) {
    *(uintptr_t *)(PPB_BASE + M0PLUS_ICSR_OFFSET) |= 1 << 28;
    sleep_ms(1000);
    printf("pendsv_returnd\n");
  }

  return 0;
}
