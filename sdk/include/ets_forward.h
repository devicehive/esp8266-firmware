#ifndef _ETS_FORWARD_H_
#define _ETS_FORWARD_H_

#include <c_types.h>
#include <os_type.h>

#include <eagle_soc.h>
#include <ets_sys.h>

void ets_install_putc1(void*);
void ets_isr_attach(int, void*, void*);
void ets_isr_mask(uint32_t);
void ets_isr_unmask(uint32_t);
void ets_intr_lock(void);
void ets_intr_unlock(void);

// memory
int ets_memcmp(const void*, const void*, size_t);
void *ets_memcpy(void*, const void*, size_t);
void *ets_memset(void*, int, size_t);
void *ets_memmove(void*, const void*, size_t);
void ets_bzero(void*, size_t);

// sprintf
int ets_sprintf(char *str, const char *format, ...)  __attribute__ ((format (printf, 2, 3)));
int os_snprintf(char *str, size_t size, const char *format, ...) __attribute__ ((format (printf, 3, 4)));
int os_printf_plus(const char *format, ...)  __attribute__ ((format (printf, 1, 2)));

// strings
int ets_strcmp(const char*, const char*);
int ets_strncmp(const char*, const char*, size_t);
char* ets_strcpy(char*, const char*);
char* ets_strncpy(char*, const char*, size_t);
size_t ets_strlen(const char*);
char *ets_strstr(const char*, const char*);
int ets_str2macaddr(void *, void *);
// int atoi(const char*);

// timer
void ets_timer_arm_new(os_timer_t*, int, int, int);
void ets_timer_disarm(os_timer_t*);
void ets_timer_setfn(os_timer_t*, ETSTimerFunc*, void*);

void ets_update_cpu_frequency(int freqmhz);
void uart_div_modify(int no, unsigned int freq);
uint8 wifi_get_opmode(void);
int rand(void);
void ets_delay_us(int ms);

// memory allocation
void *pvPortMalloc(size_t, const char *file, int line);
void *pvPortZalloc(size_t, const char *file, int line);
void vPortFree(void *ptr, const char *file, int line);

#endif // _ETS_FORWARD_H_
