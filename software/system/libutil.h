/* 
 * File:   libutil.h
 * Author: anthony
 *
 * Created on 18 septembre 2019, 17:19
 */

#ifndef LIBUTIL_H
#define	LIBUTIL_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#include <stdint.h>
#include <stdbool.h>

#ifndef BIT
#define BIT(x) (1UL << (x))
#endif

// -----------------------------------------------------------------------------
// 8-bit data
// -----------------------------------------------------------------------------
static inline bool u8_is_bit_set(uint8_t value, uint8_t bit)
{
    return ((value & BIT(bit)) == 0U) ? false : true;
}

static inline uint8_t u8_clr_bit(uint8_t value, uint8_t bit)
{
    return value &= ~BIT(bit);
}

static inline uint8_t u8_set_bit(uint8_t value, uint8_t bit)
{
    return value |= BIT(bit);
}

static inline uint8_t u8_toggle_bit(uint8_t value, uint8_t bit)
{
    return value ^= BIT(bit);
}

// -----------------------------------------------------------------------------
// 16-bit data
// -----------------------------------------------------------------------------
static inline uint16_t beu16_get(const uint8_t *buff)
{
    uint16_t val = ((uint16_t)buff[0]) << 8U;
    val += buff[1] & 0xFFU;
    return val;
}

static inline void beu16_put(uint8_t *buff, uint16_t data)
{
    buff[0] = (data >> 8U) & 0xFFU;
    buff[1] = data & 0xFFU;
}

static inline void leu16_put(uint8_t *buff, uint16_t data)
{
    buff[0] = data & 0xFFU;
    buff[1] = (data >> 8U) & 0xFFU;
}

static inline uint16_t leu16_get(const uint8_t *buff)
{
    uint16_t val = ((uint16_t)buff[1]) << 8U;
    val += buff[0] & 0xFFU;
    return val;
}

static inline uint16_t u16_clr_bit(uint16_t value, uint8_t bit)
{
    return value &= ~BIT(bit);
}

static inline uint16_t u16_set_bit(uint16_t value, uint8_t bit)
{
    return value |= BIT(bit);
}

static inline bool u16_is_bit_set(uint16_t value, uint8_t bit)
{
    return ((value & BIT(bit)) == 0U) ? false : true;
}

// -----------------------------------------------------------------------------
// 32-bit data
// -----------------------------------------------------------------------------
static inline uint32_t beu32_get(const uint8_t *a)
{
    uint32_t val = 0;
    val |= (((uint32_t) a[0]) << 24);
    val |= (((uint32_t) a[1]) << 16);
    val |= (((uint32_t) a[2]) << 8);
    val |= ((uint32_t) a[3]);
    
    return val;
}

static inline void beu32_put(uint8_t *buff, uint32_t data)
{
    buff[0] = (data >> 24U) & 0xFFU;
    buff[1] = (data >> 16U) & 0xFFU;
    buff[2] = (data >> 8U) & 0xFFU;
    buff[3] = data & 0xFFU;
}

static inline uint32_t leu32_get(const uint8_t *a)
{
    uint32_t val = 0;
    val |= (((uint32_t) a[3]) << 24);
    val |= (((uint32_t) a[2]) << 16);
    val |= (((uint32_t) a[1]) << 8);
    val |= ((uint32_t) a[0]);

    return val;
}

static inline void leu32_put(uint8_t *buff, uint32_t data)
{
    buff[3] = (data >> 24U) & 0xFFU;
    buff[2] = (data >> 16U) & 0xFFU;
    buff[1] = (data >> 8U) & 0xFFU;
    buff[0] = data & 0xFFU;
}

static inline uint32_t u32_set_bit(uint32_t value, uint8_t bit)
{
    return value |= BIT(bit);
}

static inline bool u32_is_bit_set(uint32_t value, uint8_t bit)
{
    return ((value & BIT(bit)) == 0U) ? false : true;
}



#ifdef	__cplusplus
}
#endif

#endif	/* LIBUTIL_H */

