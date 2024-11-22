#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t start;
    uint32_t value;
} bsp_dwt_t;


void bsp_dwt_init();

/** Microseconds delay
 *
 * @param[in]  microseconds  The microseconds to wait
 */
void bsp_dwt_delay_us(uint32_t microseconds);

/** Get instructions per microsecond count
 *
 * @return     instructions per microsecond count
 */
uint32_t bsp_dwt_instructions_per_microsecond();

/** Get Timer
 *
 * @param[in]  timeout_us  The expire timeout in us
 *
 * @return     The bsp_dwt_t
 */
bsp_dwt_t bsp_dwt_timer_get(uint32_t timeout_us);

/** Check if timer expired
 *
 * @param[in]  dwt  The bsp_dwt_t
 *
 * @return     true if expired
 */
bool bsp_dwt_timer_is_expired(bsp_dwt_t dwt);

/** Wait for timer expire
 *
 * @param[in]  dwt  The bsp_dwt_t
 */
void bsp_dwt_timer_wait(bsp_dwt_t dwt);

#ifdef __cplusplus
}
#endif
