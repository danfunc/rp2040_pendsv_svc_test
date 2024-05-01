#include <cinttypes>
#include <exception>
#include <hardware/exception.h>
#include <pico/stdio.h>
#include <pico/stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#define PICO_DEFAULT_LED_PI 25
const uint LED_PIN = PICO_DEFAULT_LED_PIN;
void svc_handler(void);
static bool is_led_on = false;
void svc_handler(void) {
  printf("svc_handle_success\n");
  if (is_led_on == false) {
    gpio_put(LED_PIN, 1);
    is_led_on = true;
    return;
  } else {
    gpio_put(LED_PIN, 0);
    is_led_on = false;
    return;
  }
  return;
}

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

inline void *get_sp() {
  void *rp;
  asm("mov %0 sp" : "=r"(rp));
  return rp;
}

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
  exception_stack return_stack;
  uint32_t r4 = 0, r5 = 0, r6 = 0, r7 = 0, r8 = 0, r9 = 0, r10 = 0, r11 = 0;
  void const *stack_start;
  void *sp;
  exec_mode mode = exec_mode::thread_MSP;
  context(method method, size_t arg0, size_t arg1, size_t arg2, size_t arg3,
          size_t stack_size = 1024)
      : stack_start{malloc(stack_size)} {
    return_stack.pc = (void (*)())method;
    return_stack.r0 = arg0;
    return_stack.r1 = arg1;
    return_stack.r2 = arg2;
    return_stack.r3 = arg3;
    return_stack.lr = trap;
    return_stack.xPSR = (1 << 24);
    if (stack_start != nullptr)
      sp = (void *)(((uintptr_t)stack_start + stack_size - 1) & ~0xfL);
    else
      ;
  };
  ~context() {
    if (stack_start != nullptr)
      free((void *)stack_start);
  };
};

void pendsv_handler() {

  exception_stack *stack;
  asm volatile("mrs %0, PSP" : "=r"(stack));
  stack->pc = target;
  stack->lr = trap;
  stack->xPSR = (1 << 24);
  exc_return return_code = exc_return::thread_PSP;
  asm volatile("bx %0" ::"r"(return_code));
}
int main(int argc, char const *argv[]) {
  stdio_init_all();
  // asm volatile("msr CONTROL, %0" ::"r"(SP_SEL_VAL));
  void *sp = (void *)(((uintptr_t)malloc(1024) + 1024) & ~0xfL);
  static context target_context{(method)target, 0, 0, 0, 0};
  asm volatile("msr PSP, %0" ::"r"(sp));
  exception_set_exclusive_handler(PENDSV_EXCEPTION, []() {
    void *sp = target_context.sp;
    asm volatile("msr PSP,%0" ::"r"(sp));
    exception_stack *stack;
    asm volatile("mrs %0, PSP" : "=r"(stack));
    *stack = target_context.return_stack;
    exc_return return_code = exc_return::thread_PSP;
    asm volatile("bx %0" ::"r"(return_code));
  });
  while (1) {
    *(uintptr_t *)(PPB_BASE + M0PLUS_ICSR_OFFSET) |= 1 << 28;
    sleep_ms(1000);
    printf("pendsv_returnd\n");
  }

  return 0;
}
