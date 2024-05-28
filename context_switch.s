

.cpu cortex-m0plus
.thumb
.global pendsv_handler
.type   pendsv_handler, %function
pendsv_handler:
bl get_current_context
str r4, [r0,#0]
str r5, [r0,#4]
str r6, [r0,#8]
str r7, [r0,#12]
mov r1, r8
str r1, [r0,#16]
mov r1, r9
str r1, [r0,#20]
mov r1, r10
str r1, [r0,#24]
mov r1, r11
str r1, [r0,#28]
mrs r1, PSP
str r1, [r0,#32]
bl pop_next_context
ldr r4, [r0,#0]
ldr r5, [r0,#4]
ldr r6, [r0,#8]
ldr r7, [r0,#12]
ldr r1, [r0,#16]
mov r8, r1
ldr r1, [r0,#20]
mov r9, r1
ldr r1, [r0,#24]
mov r10, r9
ldr r1, [r0,#28]
mov r11, r1
ldr r1, [r0,#32]
msr PSP, r1
mov r0, #2
neg r0,r0
bx r0
bl save_from_r4_to_r11
bl load_from_r4_to_r11