/*
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PICO_I2S_H
#define PICO_I2S_H

#include <stdbool.h>

#include "hardware/dma.h"
#include "hardware/irq.h"
#include "hardware/pio.h"

#ifdef __cplusplus
extern "C"
{
#endif

#if 0

/** \file audio_i2s.h
 *  \defgroup pico_audio_i2s pico_audio_i2s
 *  I2S audio output using the PIO
 *
 * This library uses the \ref hardware_pio system to implement a I2S audio interface
 *
 * \todo Must be more we need to say here.
 * \todo certainly need an example
 *
 */

#ifndef PICO_AUDIO_I2S_DMA_IRQ
#ifdef PICO_AUDIO_DMA_IRQ
#define PICO_AUDIO_I2S_DMA_IRQ PICO_AUDIO_DMA_IRQ
#else
#define PICO_AUDIO_I2S_DMA_IRQ 0
#endif
#endif

#ifndef PICO_AUDIO_I2S_PIO
#ifdef PICO_AUDIO_PIO
#define PICO_AUDIO_I2S_PIO PICO_AUDIO_PIO
#else
#define PICO_AUDIO_I2S_PIO 0
#endif
#endif

#if !(PICO_AUDIO_I2S_DMA_IRQ == 0 || PICO_AUDIO_I2S_DMA_IRQ == 1)
#error PICO_AUDIO_I2S_DMA_IRQ must be 0 or 1
#endif

#if !(PICO_AUDIO_I2S_PIO == 0 || PICO_AUDIO_I2S_PIO == 1)
#error PICO_AUDIO_I2S_PIO ust be 0 or 1
#endif

#ifndef PICO_AUDIO_I2S_MAX_CHANNELS
#ifdef PICO_AUDIO_MAX_CHANNELS
#define PICO_AUDIO_I2S_MAX_CHANNELS PICO_AUDIO_MAX_CHANNELS
#else
#define PICO_AUDIO_I2S_MAX_CHANNELS 2u
#endif
#endif

#ifndef PICO_AUDIO_I2S_BUFFERS_PER_CHANNEL
#ifdef PICO_AUDIO_BUFFERS_PER_CHANNEL
#define PICO_AUDIO_I2S_BUFFERS_PER_CHANNEL PICO_AUDIO_BUFFERS_PER_CHANNEL
#else
#define PICO_AUDIO_I2S_BUFFERS_PER_CHANNEL 3u
#endif
#endif

#ifndef PICO_AUDIO_I2S_BUFFER_SAMPLE_LENGTH
#ifdef PICO_AUDIO_BUFFER_SAMPLE_LENGTH
#define PICO_AUDIO_I2S_BUFFER_SAMPLE_LENGTH PICO_AUDIO_BUFFER_SAMPLE_LENGTH
#else
#define PICO_AUDIO_I2S_BUFFER_SAMPLE_LENGTH 576u
#endif
#endif

#ifndef PICO_AUDIO_I2S_SILENCE_BUFFER_SAMPLE_LENGTH
#ifdef PICO_AUDIO_I2S_SILENCE_BUFFER_SAMPLE_LENGTH
#define PICO_AUDIO_I2S_SILENCE_BUFFER_SAMPLE_LENGTH PICO_AUDIO_SILENCE_BUFFER_SAMPLE_LENGTH
#else
#define PICO_AUDIO_I2S_SILENCE_BUFFER_SAMPLE_LENGTH 256u
#endif
#endif

// Allow use of pico_audio driver without actually doing anything much
#ifndef PICO_AUDIO_I2S_NOOP
#ifdef PICO_AUDIO_NOOP
#define PICO_AUDIO_I2S_NOOP PICO_AUDIO_NOOP
#else
#define PICO_AUDIO_I2S_NOOP 0
#endif
#endif

#ifndef PICO_AUDIO_I2S_MONO_INPUT
#define PICO_AUDIO_I2S_MONO_INPUT 0
#endif
#ifndef PICO_AUDIO_I2S_MONO_OUTPUT
#define PICO_AUDIO_I2S_MONO_OUTPUT 0
#endif

#ifndef PICO_AUDIO_I2S_DATA_PIN
// #warning PICO_AUDIO_I2S_DATA_PIN should be defined when using AUDIO_I2S
#define PICO_AUDIO_I2S_DATA_PIN 28
#endif

#ifndef PICO_AUDIO_I2S_CLOCK_PIN_BASE
// #warning PICO_AUDIO_I2S_CLOCK_PIN_BASE should be defined when using AUDIO_I2S
#define PICO_AUDIO_I2S_CLOCK_PIN_BASE 26
#endif

    /** \brief Base configuration structure used when setting up
     * \ingroup pico_audio_i2s
     */
    typedef struct
    {
        uint32_t freq;
        uint32_t bps;
        uint8_t data_pin;
        uint8_t clock_pin_base;

    } audio_i2s_config_t;

    typedef struct
    {
        uint32_t freq;
        uint8_t pio_sm;
        uint8_t dma_channel;
    } shared_state_t;

    typedef struct audio_format
    {
        uint32_t sample_freq;   ///< Sample frequency in Hz
        uint16_t format;        ///< Audio format \ref audio_formats
        uint16_t channel_count; ///< Number of channels
    } audio_format_t;

    /** \brief Set up system to output I2S audio
     * \ingroup pico_audio_i2s
     *
     * \param intended_audio_format \todo
     * \param config The configuration to apply.
     */
    void pico_i2s_setup(const audio_i2s_config_t *config);

    /** \brief Set up system to output I2S audio
     * \ingroup pico_audio_i2s
     *
     * \param enable true to enable I2S audio, false to disable.
     */
    void audio_i2s_set_enabled(bool enabled);

    void audio_start_dma_transfer(const int32_t *bytes, uint32_t count);

#endif
    //----------------------------------------------------------------------------------
    //----------------------------------------------------------------------------------
    //----------------------------------------------------------------------------------
    //----------------------------------------------------------------------------------
    //----------------------------------------------------------------------------------

#define AUDIO_BUFFER_FRAMES 48
#define STEREO_BUFFER_SIZE AUDIO_BUFFER_FRAMES * 2

    typedef struct
    {
        uint32_t freq;
        uint32_t bps;
        uint8_t data_pin;
        uint8_t clock_pin_base;

    } audio_i2s_config_t;

    typedef struct pio_i2s
    {
        PIO pio;
        uint8_t sm_mask;
        uint8_t sm_dout;
        uint dma_ch_out_ctrl;
        uint dma_ch_out_data;
        int32_t *out_ctrl_blocks[2];
        int32_t output_buffer[STEREO_BUFFER_SIZE * 2];
    } pio_i2s;

    void i2s_program_start_synched(PIO pio, void (*dma_handler)(void), pio_i2s *i2s, audio_i2s_config_t *config);

#ifdef __cplusplus
}
#endif

#endif //_AUDIO_I2S_H
