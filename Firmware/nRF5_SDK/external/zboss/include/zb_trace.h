/* ZBOSS Zigbee 3.0
 *
 * Copyright (c) 2012-2018 DSR Corporation, Denver CO, USA.
 * http://www.dsr-zboss.com
 * http://www.dsr-corporation.com
 * All rights reserved.
 *
 *
 * Use in source and binary forms, redistribution in binary form only, with
 * or without modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 2. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 3. This software, with or without modification, must only be used with a Nordic
 *    Semiconductor ASA integrated circuit.
 *
 * 4. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
PURPOSE: ZigBee trace. Application should include it.
*/
#ifndef ZB_LOGGER_H
#define ZB_LOGGER_H 1

/** @cond DOXYGEN_DEBUG_SECTION */
/**
 * @addtogroup ZB_TRACE Debug trace
 * @{
 */

/** @cond internals_doc */
extern zb_uint8_t g_trace_level, g_o_trace_level;
extern zb_uint16_t g_trace_mask;
extern zb_uint_t g_trace_inside_intr;
/** @endcond */ /* internals_doc */

/**
   Set trace level at runtime

   That macro can switch trace level on and off. Trace level must be included
   into build at compile time by setting ZB_TRACE_LEVEL compiler time define.

   @param l - new trace level.

   @par Example
   @snippet doxygen_snippets.dox set_trace_snippet_zc_combo_c
   @par
 */
#define ZB_SET_TRACE_LEVEL(l) g_trace_level = (l);

/**
   Set trace mask at runtime

   That macro can switch trace mask on and off. Trace mask must be included
   into build at compile time by setting ZB_TRACE_MASK compiler time define.

   @param m - new trace mask.

   @par Example
   @snippet doxygen_snippets.dox set_trace_snippet_zc_combo_c
   @par
 */
#define ZB_SET_TRACE_MASK(m) g_trace_mask = (m);

/**
   Switch off all trace at runtime
 */
#define ZB_SET_TRACE_OFF() g_o_trace_level = g_trace_level, g_trace_level = 0
/**
   Switch on trace at runtime

   That macro enables trace which was active before call to ZB_SET_TRACE_OFF().
 */
#define ZB_SET_TRACE_ON() g_trace_level = g_o_trace_level

/** @cond DSR_TRACE */
/**
 *  @addtogroup TRACE_SUBSYSTEMS
 *  @{
 */
#define TRACE_SUBSYSTEM_COMMON    0x0001  /**< Common subsystem. */
#define TRACE_SUBSYSTEM_MEM       0x0002  /**< MEM subsystem (buffers pool). */
#define TRACE_SUBSYSTEM_MAC       0x0004  /**< MAC subsystem. */
#define TRACE_SUBSYSTEM_NWK       0x0008  /**< NWK subsystem. */

#define TRACE_SUBSYSTEM_APS       0x0010  /**< APS subsystem. */
#define TRACE_SUBSYSTEM_CLOUD     0x0020  /**< Interface to the Cloud if used, else free */
#define TRACE_SUBSYSTEM_ZDO       0x0040  /**< ZDO subsystem. */
#define TRACE_SUBSYSTEM_SECUR     0x0080  /**< Security subsystem. */

#define TRACE_SUBSYSTEM_ZCL       0x0100  /**< ZCL subsystem. */
/** @cond DOXYGEN_ZLL_SECTION*/
#define TRACE_SUBSYSTEM_ZLL       0x0200  /**< ZLL subsystem. */
/** @endcond */ /* DOXYGEN_ZLL_SECTION */
/** @cond internals_doc */
#define TRACE_SUBSYSTEM_JSON      TRACE_SUBSYSTEM_ZLL  /**< JSON decoding is used in app,
                                           * else free */
#define TRACE_SUBSYSTEM_SSL       0x0400  /**< SSL subsystem */
/** @endcond */ /* internals_doc */
/** @endcond */ /* DSR_TRACE */
#define TRACE_SUBSYSTEM_APP       0x0800  /**< User Application */

/** @cond internals_doc */
#define TRACE_SUBSYSTEM_LWIP      0x1000  /* LWIP is used, else free */
#define TRACE_SUBSYSTEM_ALIEN     0x2000  /* Some special debug */
/** @endcond */ /* internals_doc */
#define TRACE_SUBSYSTEM_ZGP       0x4000  /**< ZGP subsystem */
/** @cond internals_doc */
#define TRACE_SUBSYSTEM_USB       0x8000
/** @endcond */ /* internals_doc */

#define TRACE_SUBSYSTEM_OTA TRACE_SUBSYSTEM_ZGP /**< OTA subsystem */
#define TRACE_SUBSYSTEM_ZSE       TRACE_SUBSYSTEM_CLOUD  /**< ZSE subsystem. Not conflicting
                                                          * with cloud, let's use it */
/** @cond DSR_TRACE */
/** @cond internals_doc */
/* SPI trace usage is limited (now only in
 * application\comcast\spi\css_sensor_spi.c). Not conflict with USB, so let's
 * use it. */
#define TRACE_SUBSYSTEM_SPI TRACE_SUBSYSTEM_USB
#define TRACE_SUBSYSTEM_HTTP TRACE_SUBSYSTEM_USB /* HTTP == LWIP usage (tcp),
                                                  * but not SSL */

#define TRACE_SUBSYSTEM_TRANSPORT TRACE_SUBSYSTEM_LWIP
#define TRACE_SUBSYSTEM_UART TRACE_SUBSYSTEM_LWIP

#define TRACE_SUBSYSTEM_SPECIAL1 TRACE_SUBSYSTEM_ALIEN
#define TRACE_SUBSYSTEM_MACLL   TRACE_SUBSYSTEM_ALIEN
#define TRACE_SUBSYSTEM_VENSTAR TRACE_SUBSYSTEM_ALIEN

#define TRACE_SUBSYSTEM_BATTERY TRACE_SUBSYSTEM_SSL
#define TRACE_SUBSYSTEM_MAC_API TRACE_SUBSYSTEM_SSL
/** @endcond */ /* internals_doc */
/* to be continued... */

/** @} */ /* TRACE_SUBSYSTEMS */
/** @endcond */ /* DSR_TRACE */
/** @} */ /* ZB_TRACE */

/** @cond DSR_TRACE */
/**
 * @addtogroup ZB_TRACE Debug trace
 * @{
 */
#if defined ZB_TRACE_LEVEL || defined DOXYGEN

/** @cond internals_doc */
#define TRACE_ENABLED_(mask,lev) ((lev) <= ZB_TRACE_LEVEL && ((mask) & ZB_TRACE_MASK))
/** @endcond */ /* internals_doc */

/**
 *  @brief Check that trace is enabled for provided level.
 *  To be used in constructions like:
 *  @code
 *  if (TRACE_ENABLED(TRACE_APS3))
 *  {
 *    call_some_complex_trace();
 *  }
 *  @endcode
 *  @param m - trace level macro.
 *  @return 1 if enabled, 0 if disabled.
 */
#define TRACE_ENABLED(m) TRACE_ENABLED_(m)

/** @cond internals_doc */
zb_uint32_t zb_trace_get_counter(void);
/** @endcond */ /* internals_doc */

#ifdef DOXYGEN
/**
   Trace file id used by win_com_dump to identify source file.

   Must be first define in the .c file, just before first include directive.
  @par Example
  @snippet doxygen_snippets.dox trace_file_id_light_sample_dimmable_light_bulb_c
  @par
*/
#define ZB_TRACE_FILE_ID 12345
#endif

/** @cond internals_doc */
#ifdef ZB_INTERRUPT_SAFE_CALLBACKS
/* If HW can detect that we are inside ISR, let's use it and do not trace from ISR. */
zb_bool_t zb_osif_is_inside_isr(void);
#define ZB_HW_IS_INSIDE_ISR() zb_osif_is_inside_isr()
#else
#define ZB_HW_IS_INSIDE_ISR() 0
#endif

#define TRACE_ENTER_INT() g_trace_inside_intr = 1
#define TRACE_LEAVE_INT() g_trace_inside_intr = 0
#define ZB_TRACE_INSIDE_INTR() (g_trace_inside_intr || ZB_HW_IS_INSIDE_ISR())

#ifndef ZB_TRACE_FROM_INTR
#define ZB_TRACE_INSIDE_INTR_BLOCK() ZB_TRACE_INSIDE_INTR()
#else
#define ZB_TRACE_INSIDE_INTR_BLOCK() 0
#endif
/** @endcond */

#if defined ZB_TRACE_TO_FILE || defined ZB_TRACE_TO_SYSLOG || defined DOXYGEN
/** @cond internals_doc */
/**
   \par Trace to file means trace to disk file using printf() or its analog.
   Tricks to decrease code size by excluding format strings are not used.
 */
void zb_trace_init_file(zb_char_t *name);
void zb_trace_deinit_file(void);

/**

 Initialize trace subsystem

 @param name - trace file name component
*/
#define TRACE_INIT(name)   zb_trace_init_file(name)

/**
 Deinitialize trace subsystem
*/
#define TRACE_DEINIT zb_trace_deinit_file

#define _T0(...) __VA_ARGS__

#if defined ZB_BINARY_TRACE && !defined ZB_TRACE_TO_SYSLOG
/**
 *  @brief Print trace message.
 */
void zb_trace_msg_file(
    zb_uint_t mask,
    zb_uint_t level,
    zb_uint16_t file_id,
    zb_int_t line_number,
    zb_int_t args_size, ...);

#define _T1(s, l, fmts, args) if ((zb_int_t)ZB_TRACE_LEVEL>=(zb_int_t)l && ((s) & ZB_TRACE_MASK)) zb_trace_msg_file(s, l, _T0 args)
#else
/**
 *  @brief Print trace message.
 */
void zb_trace_msg_file(
    zb_uint_t mask,
    zb_uint_t level,
    zb_char_t *format,
    zb_char_t *file_name,
    zb_int_t line_number,
    zb_int_t args_size, ...);

#define _T1(s, l, fmts, args) if ((zb_int_t)ZB_TRACE_LEVEL>=(zb_int_t)l && ((s) & ZB_TRACE_MASK)) zb_trace_msg_file(s, l, fmts, _T0 args)
#endif

/** @endcond */ /* internals_doc */

/**
 *  @brief Put trace output.
 *  @param lm - trace subsystem and level marker.
 *  @param fmts - printf()-like format string.
 *  @param args - format string substitution parameters with a size marker.
 *
 *  @par Example
 *  @snippet doxygen_snippets.dox trace_msg_light_sample_dimmable_light_bulb_c
 *  @par
 */
#define TRACE_MSG(lm, fmts, args) \
  do { \
    _T1(lm, fmts, args); \
  } while (0)

#elif (defined ZB_TRACE_TO_PORT || defined ZB_TRACE_OVER_SIF)

/*
  8051 trace does not use format string in the code to save code space.

- will modify trace at device only, Linux will work as before
- trace implementation will hex dump all arguments as set of bytes
- external utility will parse dump, divide trace arguments dump into separate arguments and
convert hex-int, unsigned etc.
- utility will get argument strings from the source files (trace macros) and find it
by file:line
- Add one more parameter to the trace macro: sum of the trace argument sizes.
Define readable constants like
#define FMT_D_HD_X 5
- create script/program to modify existing trace calls
- combine dump parse utility functionality with win_com_dump, so it will produce human-readable trace

 */

#ifdef ZB_TRACE_OVER_SIF
#define TRACE_INIT(name) zb_osif_sif_init()
#else
#ifndef ZB_SERIAL_FOR_TRACE
#define TRACE_INIT(name)
#else
#define TRACE_INIT(name) zb_osif_serial_init()
#endif
#endif
/* No trace deinit */
#define TRACE_DEINIT()

#ifndef ZB_BINARY_TRACE
void zb_trace_msg_port(
    zb_char_t ZB_IAR_CODE *file_name,
    zb_int_t line_number,
    zb_uint8_t args_size, ...);

#else

#ifndef ZB_TRACE_FILE_ID
#ifndef ZB_DONT_NEED_TRACE_FILE_ID
#warning ZB_TRACE_FILE_ID undefined! Insert #define ZB_TRACE_FILE_ID NNN before 1-st #include
#endif
#define ZB_TRACE_FILE_ID 0
#else
ZB_ASSERT_COMPILE_DECL(ZB_TRACE_FILE_ID < ZB_UINT16_MAX);
#endif

void zb_trace_msg_port(
  zb_uint_t mask,
  zb_uint_t level,
  zb_uint16_t file_id,
  zb_uint16_t line_number,
  zb_uint_t args_size, ...);

#endif

#ifdef ZB_BINARY_TRACE
#define _T0(...) __VA_ARGS__
#define _T1(s, l, args) if ((zb_int_t)ZB_TRACE_LEVEL>=(zb_int_t)l && ((s) == (zb_uint_t)-1 || (s) & ZB_TRACE_MASK) && !ZB_TRACE_INSIDE_INTR_BLOCK()) zb_trace_msg_port(s, l, _T0 args)

#else
#define _T1(s, l, args) \
  if ((zb_int_t)ZB_TRACE_LEVEL>=(zb_int_t)l && ((s) == -1 || ((s) & ZB_TRACE_MASK)) && !ZB_TRACE_INSIDE_INTR_BLOCK()) zb_trace_msg_port args
#endif


#define TRACE_MSG(lm, fmt, args) \
  do { \
    _T1(lm, args); \
  } while (0)

#else

#error Must define either ZB_TRACE_TO_FILE or ZB_TRACE_TO_PORT || ZB_TRACE_OVER_SIF

#endif  /* trace type */


#else  /* if trace off */

#ifndef KEIL
#define TRACE_MSG(...)
#else
/* Keil does not support vararg macros */
#define TRACE_MSG(a,b,c)
#endif

#define TRACE_INIT(name)
#define TRACE_DEINIT(c)

#define TRACE_ENABLED(m) 0

#endif  /* trace on/off */


#if defined ZB_TRACE_TO_FILE || defined ZB_TRACE_TO_SYSLOG || defined DOXYGEN

/**
   Trace format for 64-bit address.

   @snippet doxygen_snippets.dox trace_64_tests_zdo_startup_dll_zdo_start_zr_c
*/
#define TRACE_FORMAT_64 "%hx.%hx.%hx.%hx.%hx.%hx.%hx.%hx"

/**
 Trace format for 128-bit number (key etc).

 @snippet doxygen_snippets.dox trace_128_snippet_cs_ick_tc_02_thr1_c
*/
#define TRACE_FORMAT_128 "%02hx:%02hx:%02hx:%02hx:%02hx:%02hx:%02hx:%02hx:%02hx:%02hx:%02hx:%02hx:%02hx:%02hx:%02hx:%02hx"

/**
   Trace format arguments for 64-bit address.

   @snippet doxygen_snippets.dox trace_64_tests_zdo_startup_dll_zdo_start_zr_c
*/
#define TRACE_ARG_64(a) (zb_uint8_t)((a)[7]),(zb_uint8_t)((a)[6]),(zb_uint8_t)((a)[5]),(zb_uint8_t)((a)[4]),(zb_uint8_t)((a)[3]),(zb_uint8_t)((a)[2]),(zb_uint8_t)((a)[1]),(zb_uint8_t)((a)[0])

/**
   Trace format arguments for 128-bit numver.

   @snippet doxygen_snippets.dox trace_128_snippet_cs_ick_tc_02_thr1_c
*/
#define TRACE_ARG_128(a) (zb_uint8_t)((a)[0]),(zb_uint8_t)((a)[1]),(zb_uint8_t)((a)[2]),(zb_uint8_t)((a)[3]),(zb_uint8_t)((a)[4]),(zb_uint8_t)((a)[5]),(zb_uint8_t)((a)[6]),(zb_uint8_t)((a)[7]),(zb_uint8_t)((a)[8]),(zb_uint8_t)((a)[9]),(zb_uint8_t)((a)[10]),(zb_uint8_t)((a)[11]),(zb_uint8_t)((a)[12]),(zb_uint8_t)((a)[13]),(zb_uint8_t)((a)[14]),(zb_uint8_t)((a)[15])


#else

/** @brief Trace format for 64-bit address - single argument for 8051. */
#define TRACE_FORMAT_64 "%A"
#define TRACE_FORMAT_128 "%A:%A"

/** @cond internals_doc */

typedef struct zb_addr64_struct_s
{
  zb_64bit_addr_t addr;
} ZB_PACKED_STRUCT zb_addr64_struct_t;

typedef struct zb_byte128_struct_s
{
  zb_uint8_t d[16];
} ZB_PACKED_STRUCT zb_byte128_struct_t;

/* Pass 8-bytes address as structure by value */
#define TRACE_ARG_64(a) *((zb_addr64_struct_t *)a)
/* Pass 16-bytes key as structure by value */
#define TRACE_ARG_128(a) *((zb_byte128_struct_t *)a)

#endif  /* ti file / !to file */

/** @endcond */ /* internals_doc */

/** @cond DSR_TRACE */
/**
 *  @addtogroup TRACE_GENERAL_MESSAGES
 *  @{
 */
/** @brief Error message. */
#define TRACE_ERROR (zb_uint_t)-1, 1
/** @brief Information message level 1. */
#define TRACE_INFO1 (zb_uint_t)-1, 2
/** @brief Information message level 2. */
#define TRACE_INFO2 (zb_uint_t)-1, 3
/** @brief Information message level 3. */
#define TRACE_INFO3 (zb_uint_t)-1, 4

/** @} */ /* TRACE_GENERAL_MESSAGES */

#ifdef ZB_TRACE_LEVEL
#ifndef ZB_TRACE_MASK
#define ZB_TRACE_MASK -1
#endif  /* if not defined trace_mask */
#endif  /* if defined trace level */

/**
 *  @addtogroup TRACE_DATA_FORMAT
 *  @{
 */

/*
  Trace format constants for serial trace
*/

/* Keil and sdcc put byte values to the stack as is, but IAR/8051 casts it to 16-bit
 * integers, so constant lengths differs */

/**
 *  @brief Calculates trace argument size.
 *  @param n_h - number of Hs/Cs.
 *  @param n_d - number of Ds.
 *  @param n_l - number of Ls.
 *  @param n_p - number of Ps.
 *  @param n_a - number of As.
 *  @hideinitializer
 */
#ifdef KEIL

/* Keil pass 1-byte to varargs as is, pointer is 3-bytes, short is 2-bytes */

#ifndef ZB_BINARY_TRACE
#define TRACE_ARG_SIZE(n_h, n_d, n_l, n_p, n_a) __FILE__,__LINE__, (n_h + n_d*2 + n_l*4 + n_p*3 + n_a*8)
#else
#define TRACE_ARG_SIZE(n_h, n_d, n_l, n_p, n_a) ZB_TRACE_FILE_ID,__LINE__, (n_h + n_d*2 + n_l*4 + n_p*3 + n_a*8)
#endif

#else  /* IAR & GCC - 32-bit */
/* IAR for Cortex passes 1-byte abd 2-bytes arguments as 4-bytes to vararg functions.
 * Pointers are 4-bytes. */

#if defined ZB_BINARY_TRACE && !defined ZB_TRACE_TO_SYSLOG
#define TRACE_ARG_SIZE(n_h, n_d, n_l, n_p, n_a) ZB_TRACE_FILE_ID,__LINE__, (n_h*4 + n_d*4 + n_l*4 + n_p*4 + n_a*8)
#else
#define TRACE_ARG_SIZE(n_h, n_d, n_l, n_p, n_a) __FILE__,__LINE__, (n_h*4 + n_d*4 + n_l*4 + n_p*4 + n_a*8)
#endif

#endif


#define FMT__0                                          TRACE_ARG_SIZE(0,0,0,0,0)
#define FMT__A                                          TRACE_ARG_SIZE(0,0,0,0,1)
#define FMT__A_A                                        TRACE_ARG_SIZE(0,0,0,0,2)
#define FMT__A_A_A_A                                    TRACE_ARG_SIZE(0,0,0,0,4)
#define FMT__A_D_A_P                                    TRACE_ARG_SIZE(0,1,0,1,2)
#define FMT__A_D_D                                      TRACE_ARG_SIZE(0,2,0,0,1)
#define FMT__A_D_D_P_H                                  TRACE_ARG_SIZE(1,2,0,1,1)
#define FMT__A_D_H                                      TRACE_ARG_SIZE(1,1,0,0,1)
#define FMT__A_D_P_H_H_H                                TRACE_ARG_SIZE(3,1,0,1,1)
#define FMT__A_H                                        TRACE_ARG_SIZE(1,0,0,0,1)
#define FMT__A_H_H                                      TRACE_ARG_SIZE(2,0,0,0,1)
#define FMT__A_H_H_H                                    TRACE_ARG_SIZE(3,0,0,0,1)
#define FMT__A_D_H_H_H                                  TRACE_ARG_SIZE(3,1,0,0,1)
#define FMT__A_P                                        TRACE_ARG_SIZE(0,0,0,1,1)
#define FMT__AA                                         TRACE_ARG_SIZE(0,0,0,0,2)
#define FMT__C                                          TRACE_ARG_SIZE(1,0,0,0,0)
#define FMT__C_C_H_H                                    TRACE_ARG_SIZE(4,0,0,0,0)
#define FMT__C_H                                        TRACE_ARG_SIZE(2,0,0,0,0)
#define FMT__D                                          TRACE_ARG_SIZE(0,1,0,0,0)
#define FMT__D_A                                        TRACE_ARG_SIZE(0,1,0,0,1)
#define FMT__D_A_A_H                                    TRACE_ARG_SIZE(1,1,0,0,2)
#define FMT__D_A_D_D_D_D_D_D_D_D                        TRACE_ARG_SIZE(0,9,0,0,1)
#define FMT__D_A_D_P_H_H_H                              TRACE_ARG_SIZE(3,2,0,1,1)
#define FMT__D_A_H_H                                    TRACE_ARG_SIZE(2,1,0,0,1)
#define FMT__D_A_P                                      TRACE_ARG_SIZE(0,1,0,1,1)
#define FMT__D_C                                        TRACE_ARG_SIZE(1,1,0,0,0)
#define FMT__D_D                                        TRACE_ARG_SIZE(0,2,0,0,0)
#define FMT__D_D_A                                      TRACE_ARG_SIZE(0,2,0,0,1)
#define FMT__D_D_A_A                                    TRACE_ARG_SIZE(0,2,0,0,2)
#define FMT__D_D_A_D                                    TRACE_ARG_SIZE(0,3,0,0,1)
#define FMT__D_D_A_D_D_D_D                              TRACE_ARG_SIZE(0,6,0,0,1)
#define FMT__D_D_D                                      TRACE_ARG_SIZE(0,3,0,0,0)
#define FMT__D_D_D_C                                    TRACE_ARG_SIZE(1,3,0,0,0)
#define FMT__D_D_D_D                                    TRACE_ARG_SIZE(0,4,0,0,0)
#define FMT__D_D_D_D_D                                  TRACE_ARG_SIZE(0,5,0,0,0)
#define FMT__D_D_D_D_D_D                                TRACE_ARG_SIZE(0,6,0,0,0)
#define FMT__D_D_D_D_D_D_D                              TRACE_ARG_SIZE(0,7,0,0,0)
#define FMT__D_D_D_D_D_D_D_D                            TRACE_ARG_SIZE(0,8,0,0,0)
#define FMT__D_D_D_D_D_D_D_D_D_D_D                      TRACE_ARG_SIZE(0,11,0,0,0)
#define FMT__D_D_D_D_D_D_D_D_D_D_D_D_D_D_D_D_D          TRACE_ARG_SIZE(0,17,0,0,0)
#define FMT__D_D_D_D_D_D_P                              TRACE_ARG_SIZE(0,6,0,1,0)
#define FMT__D_D_D_D_H                                  TRACE_ARG_SIZE(1,4,0,0,0)
#define FMT__D_D_D_H                                    TRACE_ARG_SIZE(1,3,0,0,0)
#define FMT__D_D_D_H_H                                  TRACE_ARG_SIZE(2,3,0,0,0)
#define FMT__D_D_D_L_L                                  TRACE_ARG_SIZE(0,3,2,0,0)
#define FMT__D_D_D_P                                    TRACE_ARG_SIZE(0,3,0,1,0)
#define FMT__D_D_H                                      TRACE_ARG_SIZE(1,2,0,0,0)
#define FMT__D_D_H_D_H                                  TRACE_ARG_SIZE(2,3,0,0,0)
#define FMT__D_D_H_H                                    TRACE_ARG_SIZE(2,2,0,0,0)
#define FMT__D_D_H_P_D                                  TRACE_ARG_SIZE(1,3,0,1,0)
#define FMT__D_D_L                                      TRACE_ARG_SIZE(0,2,1,0,0)
#define FMT__D_D_L_P                                    TRACE_ARG_SIZE(0,2,1,1,0)
#define FMT__D_D_P                                      TRACE_ARG_SIZE(0,2,0,1,0)
#define FMT__D_D_P_D                                    TRACE_ARG_SIZE(0,3,0,1,0)
#define FMT__D_D_P_D_D                                  TRACE_ARG_SIZE(0,4,0,1,0)
#define FMT__D_D_P_D_L                                  TRACE_ARG_SIZE(0,3,1,1,0)
#define FMT__D_D_P_H                                    TRACE_ARG_SIZE(1,2,0,1,0)
#define FMT__D_D_P_H_H_H                                TRACE_ARG_SIZE(3,2,0,1,0)
#define FMT__D_D_P_P_P                                  TRACE_ARG_SIZE(0,2,0,3,0)
#define FMT__D_H                                        TRACE_ARG_SIZE(1,1,0,0,0)
#define FMT__D_H_D                                      TRACE_ARG_SIZE(1,2,0,0,0)
#define FMT__D_H_D_D                                    TRACE_ARG_SIZE(1,3,0,0,0)
#define FMT__D_H_D_D_H_D                                TRACE_ARG_SIZE(2,4,0,0,0)
#define FMT__D_H_D_H                                    TRACE_ARG_SIZE(2,2,0,0,0)
#define FMT__D_H_D_H_H                                  TRACE_ARG_SIZE(3,2,0,0,0)
#define FMT__D_H_D_P                                    TRACE_ARG_SIZE(1,2,0,1,0)
#define FMT__D_H_D_P_D                                  TRACE_ARG_SIZE(1,3,0,1,0)
#define FMT__D_H_H                                      TRACE_ARG_SIZE(2,1,0,0,0)
#define FMT__D_H_H_D                                    TRACE_ARG_SIZE(2,2,0,0,0)
#define FMT__D_H_H_D_D_D_D                              TRACE_ARG_SIZE(2,5,0,0,0)
#define FMT__D_H_H_H                                    TRACE_ARG_SIZE(3,1,0,0,0)
#define FMT__D_H_H_H_H                                  TRACE_ARG_SIZE(4,1,0,0,0)
#define FMT__D_H_H_H_H_H                                TRACE_ARG_SIZE(5,1,0,0,0)
#define FMT__D_H_H_H_H_H_H_D_D_D_D                      TRACE_ARG_SIZE(6,5,0,0,0)
#define FMT__D_H_L                                      TRACE_ARG_SIZE(1,1,1,0,0)
#define FMT__D_H_P                                      TRACE_ARG_SIZE(1,1,0,1,0)
#define FMT__D_L                                        TRACE_ARG_SIZE(0,1,1,0,0)
#define FMT__D_L_L                                      TRACE_ARG_SIZE(0,1,2,0,0)
#define FMT__D_L_L_D                                    TRACE_ARG_SIZE(0,2,2,0,0)
#define FMT__D_L_L_H                                    TRACE_ARG_SIZE(1,1,2,0,0)
#define FMT__D_L_L_L                                    TRACE_ARG_SIZE(0,1,3,0,0)
#define FMT__D_P                                        TRACE_ARG_SIZE(0,1,0,1,0)
#define FMT__D_P_D                                      TRACE_ARG_SIZE(0,2,0,1,0)
#define FMT__D_P_D_D                                    TRACE_ARG_SIZE(0,3,0,1,0)
#define FMT__D_P_H_H_D_D                                TRACE_ARG_SIZE(2,3,0,1,0)
#define FMT__D_P_H_H_D_H_H                              TRACE_ARG_SIZE(4,2,0,1,0)
#define FMT__D_P_H_H_H_H                                TRACE_ARG_SIZE(4,1,0,1,0)
#define FMT__D_P_H_H_H_H_H                              TRACE_ARG_SIZE(5,1,0,1,0)
#define FMT__D_P_H_H_H_H_H_H                            TRACE_ARG_SIZE(6,1,0,1,0)
#define FMT__D_P_H_P                                    TRACE_ARG_SIZE(1,1,0,1,0)
#define FMT__D_P_P                                      TRACE_ARG_SIZE(0,1,0,2,0)
#define FMT__D_P_P_D_D_H_H                              TRACE_ARG_SIZE(2,3,0,2,0)
#define FMT__D_P_P_H                                    TRACE_ARG_SIZE(1,1,0,2,0)
#define FMT__H                                          TRACE_ARG_SIZE(1,0,0,0,0)
#define FMT__H_A                                        TRACE_ARG_SIZE(1,0,0,0,1)
#define FMT__H_A_A                                      TRACE_ARG_SIZE(1,0,0,0,2)
#define FMT__H_A_A_H_H                                  TRACE_ARG_SIZE(3,0,0,0,2)
#define FMT__H_A_H                                      TRACE_ARG_SIZE(2,0,0,0,1)
#define FMT__H_A_H_H_H                                  TRACE_ARG_SIZE(4,0,0,0,1)
#define FMT__H_A_H_H_H_H_H_H_H_H                        TRACE_ARG_SIZE(9,0,0,0,1)
#define FMT__H_C_D_C                                    TRACE_ARG_SIZE(3,1,0,0,0)
#define FMT__H_C_H_C                                    TRACE_ARG_SIZE(4,0,0,0,0)
#define FMT__H_D                                        TRACE_ARG_SIZE(1,1,0,0,0)
#define FMT__H_D_L                                      TRACE_ARG_SIZE(1,1,1,0,0)
#define FMT__H_D_A                                      TRACE_ARG_SIZE(1,1,0,0,1)
#define FMT__H_D_A_H_D                                  TRACE_ARG_SIZE(2,2,0,0,1)
#define FMT__H_D_A_H_H                                  TRACE_ARG_SIZE(3,1,0,0,1)
#define FMT__H_D_A_H_H_H_H                              TRACE_ARG_SIZE(5,1,0,0,1)
#define FMT__H_D_D                                      TRACE_ARG_SIZE(1,2,0,0,0)
#define FMT__H_D_D_D                                    TRACE_ARG_SIZE(1,3,0,0,0)
#define FMT__H_D_D_D_D                                  TRACE_ARG_SIZE(1,4,0,0,0)
#define FMT__H_D_D_D_D_H                                TRACE_ARG_SIZE(2,4,0,0,0)
#define FMT__H_D_D_D_H_H_D                              TRACE_ARG_SIZE(3,4,0,0,0)
#define FMT__H_D_D_H                                    TRACE_ARG_SIZE(2,2,0,0,0)
#define FMT__H_D_D_H_D                                  TRACE_ARG_SIZE(2,3,0,0,0)
#define FMT__H_D_D_H_D_H                                TRACE_ARG_SIZE(3,3,0,0,0)
#define FMT__H_D_D_H_H                                  TRACE_ARG_SIZE(3,2,0,0,0)
#define FMT__H_D_D_H_H_H                                TRACE_ARG_SIZE(4,2,0,0,0)
#define FMT__H_D_D_H_H_H_H                              TRACE_ARG_SIZE(5,2,0,0,0)
#define FMT__H_D_D_H_P                                  TRACE_ARG_SIZE(2,2,0,1,0)
#define FMT__H_D_D_P                                    TRACE_ARG_SIZE(1,2,0,1,0)
#define FMT__H_D_H                                      TRACE_ARG_SIZE(2,1,0,0,0)
#define FMT__H_D_H_D                                    TRACE_ARG_SIZE(2,2,0,0,0)
#define FMT__H_D_H_D_D                                  TRACE_ARG_SIZE(2,3,0,0,0)
#define FMT__H_D_H_D_H_H                                TRACE_ARG_SIZE(4,2,0,0,0)
#define FMT__H_D_H_H                                    TRACE_ARG_SIZE(3,1,0,0,0)
#define FMT__H_D_H_H_H                                  TRACE_ARG_SIZE(4,1,0,0,0)
#define FMT__H_D_H_H_H_H                                TRACE_ARG_SIZE(5,1,0,0,0)
#define FMT__H_D_H_H_H_H_H_H                            TRACE_ARG_SIZE(7,1,0,0,0)
#define FMT__H_D_P_H_H_H_H_H                            TRACE_ARG_SIZE(6,1,0,1,0)
#define FMT__H_D_P_H_H_H_H_H_H_H                        TRACE_ARG_SIZE(8,1,0,1,0)
#define FMT__H_D_P_P_P                                  TRACE_ARG_SIZE(1,1,0,3,0)
#define FMT__H_H                                        TRACE_ARG_SIZE(2,0,0,0,0)
#define FMT__H_H_A                                      TRACE_ARG_SIZE(2,0,0,0,1)
#define FMT__H_H_A_A                                    TRACE_ARG_SIZE(2,0,0,0,2)
#define FMT__H_H_D                                      TRACE_ARG_SIZE(2,1,0,0,0)
#define FMT__H_H_D_D                                    TRACE_ARG_SIZE(2,2,0,0,0)
#define FMT__H_H_D_D_D                                  TRACE_ARG_SIZE(2,3,0,0,0)
#define FMT__H_H_D_H                                    TRACE_ARG_SIZE(3,1,0,0,0)
#define FMT__H_H_D_H_H_H_H_H_H_H_H                      TRACE_ARG_SIZE(10,1,0,0,0)
#define FMT__D_H_H_H_H_H_H_H_H                          TRACE_ARG_SIZE(8,1,0,0,0)
#define FMT__H_H_D_H_P                                  TRACE_ARG_SIZE(3,1,0,1,0)
#define FMT__H_D_P                                      TRACE_ARG_SIZE(1,1,0,1,0)
#define FMT__H_H_D_P                                    TRACE_ARG_SIZE(2,1,0,1,0)
#define FMT__H_H_H                                      TRACE_ARG_SIZE(3,0,0,0,0)
#define FMT__H_H_H_D                                    TRACE_ARG_SIZE(3,1,0,0,0)
#define FMT__H_H_H_D_D                                  TRACE_ARG_SIZE(3,2,0,0,0)
#define FMT__H_H_H_D_D_H_A_H_A                          TRACE_ARG_SIZE(5,2,0,0,2)
#define FMT__H_H_H_D_D_D                                TRACE_ARG_SIZE(3,3,0,0,0)
#define FMT__H_H_H_D_H                                  TRACE_ARG_SIZE(4,1,0,0,0)
#define FMT__H_H_H_D_H_P                                TRACE_ARG_SIZE(4,1,0,1,0)
#define FMT__H_H_H_H                                    TRACE_ARG_SIZE(4,0,0,0,0)
#define FMT__H_H_H_H_D                                  TRACE_ARG_SIZE(4,1,0,0,0)
#define FMT__H_H_H_H_D_H                                TRACE_ARG_SIZE(5,1,0,0,0)
#define FMT__H_H_H_H_H                                  TRACE_ARG_SIZE(5,0,0,0,0)
#define FMT__H_H_H_H_H_H                                TRACE_ARG_SIZE(6,0,0,0,0)
#define FMT__H_H_H_H_H_H_H                              TRACE_ARG_SIZE(7,0,0,0,0)
#define FMT__H_H_H_H_H_H_H_H                            TRACE_ARG_SIZE(8,0,0,0,0)
#define FMT__H_H_H_H_H_H_H_H_H                          TRACE_ARG_SIZE(9,0,0,0,0)
#define FMT__H_H_H_H_H_H_H_H_P_P                        TRACE_ARG_SIZE(8,0,0,2,0)
#define FMT__H_H_H_H_H_H_P_P                            TRACE_ARG_SIZE(6,0,0,2,0)
#define FMT__H_H_H_H_H_H_P_P_P_P                        TRACE_ARG_SIZE(6,2,0,4,0)
#define FMT__H_H_H_H_P_P_P                              TRACE_ARG_SIZE(4,0,0,3,0)
#define FMT__H_H_H_P                                    TRACE_ARG_SIZE(3,0,0,1,0)
#define FMT__H_H_L                                      TRACE_ARG_SIZE(2,0,1,0,0)
#define FMT__H_H_L_H                                    TRACE_ARG_SIZE(3,0,1,0,0)
#define FMT__H_H_H_L                                    TRACE_ARG_SIZE(3,0,1,0,0)
#define FMT__H_H_L_H_P                                  TRACE_ARG_SIZE(3,0,1,4,0)
#define FMT__H_H_P                                      TRACE_ARG_SIZE(2,0,0,1,0)
#define FMT__H_H_P_D                                    TRACE_ARG_SIZE(2,1,0,1,0)
#define FMT__H_H_P_P_P                                  TRACE_ARG_SIZE(2,0,0,3,0)
#define FMT__H_L                                        TRACE_ARG_SIZE(1,0,1,0,0)
#define FMT__H_L_A                                      TRACE_ARG_SIZE(1,0,1,0,1)
#define FMT__H_L_D                                      TRACE_ARG_SIZE(1,1,1,0,0)
#define FMT__H_L_L                                      TRACE_ARG_SIZE(1,0,2,0,0)
#define FMT__H_L_H                                      TRACE_ARG_SIZE(2,0,1,0,0)
#define FMT__H_L_D_D                                    TRACE_ARG_SIZE(1,2,1,0,0)
#define FMT__H_L_D_P                                    TRACE_ARG_SIZE(1,1,1,1,0)
#define FMT__H_L_H_D                                    TRACE_ARG_SIZE(2,1,1,0,0)
#define FMT__H_L_H_D_D                                  TRACE_ARG_SIZE(2,2,1,0,0)
#define FMT__H_L_H_H_H_H_H_H_H_H_H_H_H_H                TRACE_ARG_SIZE(13,0,1,0,0)
#define FMT__H_D_H_H_H_H_H_H_H_H_H_H_H_H                TRACE_ARG_SIZE(13,1,0,0,0)
#define FMT__H_L_H_H_P                                  TRACE_ARG_SIZE(3,0,1,4,0)
#define FMT__H_L_L_H_A                                  TRACE_ARG_SIZE(2,0,2,0,1)
#define FMT__H_L_D_D_H                                  TRACE_ARG_SIZE(2,2,1,0,0)
#define FMT__H_P                                        TRACE_ARG_SIZE(1,0,0,1,0)
#define FMT__H_P_D                                      TRACE_ARG_SIZE(1,1,0,1,0)
#define FMT__H_P_D_D_H_P                                TRACE_ARG_SIZE(2,2,0,2,0)
#define FMT__H_P_H                                      TRACE_ARG_SIZE(2,0,0,1,0)
#define FMT__H_P_H_D                                    TRACE_ARG_SIZE(2,1,0,1,0)
#define FMT__H_P_H_D_H                                  TRACE_ARG_SIZE(3,1,0,1,0)
#define FMT__H_P_H_H                                    TRACE_ARG_SIZE(3,0,0,1,0)
#define FMT__H_P_H_H_H                                  TRACE_ARG_SIZE(4,0,0,1,0)
#define FMT__H_P_H_H_D                                  TRACE_ARG_SIZE(3,1,0,1,0)
#define FMT__H_P_H_H_H_H                                TRACE_ARG_SIZE(5,0,0,1,0)
#define FMT__H_P_H_P                                    TRACE_ARG_SIZE(2,0,0,2,0)
#define FMT__H_P_H_P_H                                  TRACE_ARG_SIZE(3,0,0,2,0)
#define FMT__H_P_H_P_H_H                                TRACE_ARG_SIZE(4,0,0,2,0)
#define FMT__H_P_H_P_H_H_P                              TRACE_ARG_SIZE(4,0,0,3,0)
#define FMT__H_P_L_H                                    TRACE_ARG_SIZE(2,0,1,1,0)
#define FMT__H_P_P_P                                    TRACE_ARG_SIZE(1,0,0,3,0)
#define FMT__H_P_P_P_D_P_P                              TRACE_ARG_SIZE(1,1,0,5,0)
#define FMT__H_P_P_P_P                                  TRACE_ARG_SIZE(1,0,0,4,0)
#define FMT__H_P_P_P_P_P                                TRACE_ARG_SIZE(1,0,0,5,0)
#define FMT__L                                          TRACE_ARG_SIZE(0,0,1,0,0)
#define FMT__L_A                                        TRACE_ARG_SIZE(0,0,1,0,1)
#define FMT__L_D                                        TRACE_ARG_SIZE(0,1,1,0,0)
#define FMT__L_D_D                                      TRACE_ARG_SIZE(0,2,1,0,0)
#define FMT__L_D_D_D                                    TRACE_ARG_SIZE(0,3,1,0,0)
#define FMT__L_H                                        TRACE_ARG_SIZE(1,0,1,0,0)
#define FMT__L_H_H                                      TRACE_ARG_SIZE(2,0,1,0,0)
#define FMT__L_H_H_H                                    TRACE_ARG_SIZE(3,0,1,0,0)
#define FMT__L_L                                        TRACE_ARG_SIZE(0,0,2,0,0)
#define FMT__L_L_L                                      TRACE_ARG_SIZE(0,0,3,0,0)
#define FMT__L_L_A_A                                    TRACE_ARG_SIZE(0,0,2,0,2)
#define FMT__L_D_A                                      TRACE_ARG_SIZE(0,1,1,0,1)
#define FMT__L_D_A_A                                    TRACE_ARG_SIZE(0,1,1,0,2)
#define FMT__L_L_A_A_H_L_H_H_H_H_H_H_H_H_H_H_H_H        TRACE_ARG_SIZE(13,0,3,0,2)
#define FMT__L_L_D_D                                    TRACE_ARG_SIZE(0,2,2,0,0)
#define FMT__L_L_H_L_H_H_H_H_H_H_H_H_H_H_H_H            TRACE_ARG_SIZE(13,0,3,0,0)
#define FMT__L_L_L_D                                    TRACE_ARG_SIZE(0,1,3,0,0)
#define FMT__L_L_L_L                                    TRACE_ARG_SIZE(0,0,4,0,0)
#define FMT__L_L_L_L_L                                  TRACE_ARG_SIZE(0,0,5,0,0)
#define FMT__L_L_L_L_L_L_L                              TRACE_ARG_SIZE(0,0,7,0,0)
#define FMT__L_P                                        TRACE_ARG_SIZE(0,0,1,1,0)
#define FMT__L_P_L                                      TRACE_ARG_SIZE(0,0,2,1,0)
#define FMT__P                                          TRACE_ARG_SIZE(0,0,0,1,0)
#define FMT__P_A                                        TRACE_ARG_SIZE(0,0,0,1,1)
#define FMT__P_D                                        TRACE_ARG_SIZE(0,1,0,1,0)
#define FMT__P_D_D                                      TRACE_ARG_SIZE(0,2,0,1,0)
#define FMT__P_D_D_D                                    TRACE_ARG_SIZE(0,3,0,1,0)
#define FMT__P_D_D_D_D                                  TRACE_ARG_SIZE(0,4,0,1,0)
#define FMT__P_D_D_H_D                                  TRACE_ARG_SIZE(1,3,0,1,0)
#define FMT__P_D_D_D_D_D                                TRACE_ARG_SIZE(0,5,0,1,0)
#define FMT__P_D_D_D_D_D_D                              TRACE_ARG_SIZE(0,6,0,1,0)
#define FMT__P_D_D_D_D_D_D_D                            TRACE_ARG_SIZE(0,7,0,1,0)
#define FMT__P_D_D_D_H_D                                TRACE_ARG_SIZE(1,4,0,1,0)
#define FMT__P_D_D_H                                    TRACE_ARG_SIZE(1,2,0,1,0)
#define FMT__P_D_D_L_L_P                                TRACE_ARG_SIZE(0,2,2,2,0)
#define FMT__P_D_H                                      TRACE_ARG_SIZE(1,1,0,1,0)
#define FMT__P_D_P                                      TRACE_ARG_SIZE(0,1,0,2,0)
#define FMT__P_D_L                                      TRACE_ARG_SIZE(0,1,1,1,0)
#define FMT__P_D_P_D                                    TRACE_ARG_SIZE(0,2,0,2,0)
#define FMT__P_H                                        TRACE_ARG_SIZE(1,0,0,1,0)
#define FMT__P_H_D                                      TRACE_ARG_SIZE(1,1,0,1,0)
#define FMT__P_H_D_D                                    TRACE_ARG_SIZE(1,2,0,1,0)
#define FMT__P_H_D_D_H                                  TRACE_ARG_SIZE(2,2,0,1,0)
#define FMT__P_H_D_P                                    TRACE_ARG_SIZE(1,1,0,2,0)
#define FMT__P_H_D_H                                    TRACE_ARG_SIZE(2,1,0,1,0)
#define FMT__P_H_D_L                                    TRACE_ARG_SIZE(1,1,1,1,0)
#define FMT__P_H_H                                      TRACE_ARG_SIZE(2,0,0,1,0)
#define FMT__P_H_H_D_D                                  TRACE_ARG_SIZE(2,2,0,1,0)
#define FMT__P_H_H_H                                    TRACE_ARG_SIZE(3,0,0,1,0)
#define FMT__P_H_H_H_A                                  TRACE_ARG_SIZE(3,0,0,1,1)
#define FMT__P_H_H_H_D                                  TRACE_ARG_SIZE(3,1,0,1,0)
#define FMT__P_H_H_H_H                                  TRACE_ARG_SIZE(4,0,0,1,0)
#define FMT__P_H_H_H_H_D                                TRACE_ARG_SIZE(4,1,0,1,0)
#define FMT__P_H_H_H_H_H_H                              TRACE_ARG_SIZE(6,0,0,1,0)
#define FMT__P_H_H_H_H_H_H_H                            TRACE_ARG_SIZE(7,0,0,1,0)
#define FMT__P_H_H_H_L                                  TRACE_ARG_SIZE(3,0,1,1,0)
#define FMT__P_H_H_L                                    TRACE_ARG_SIZE(2,0,1,1,0)
#define FMT__P_H_L                                      TRACE_ARG_SIZE(1,0,1,1,0)
#define FMT__P_H_L_H_L                                  TRACE_ARG_SIZE(2,0,2,1,0)
#define FMT__P_H_P                                      TRACE_ARG_SIZE(1,0,0,2,0)
#define FMT__P_H_P_D                                    TRACE_ARG_SIZE(1,1,0,2,0)
#define FMT__P_H_P_D_D                                  TRACE_ARG_SIZE(1,2,0,2,0)
#define FMT__P_H_P_D_D_D                                TRACE_ARG_SIZE(1,3,0,2,0)
#define FMT__P_H_P_H                                    TRACE_ARG_SIZE(2,0,0,2,0)
#define FMT__P_H_P_H_L                                  TRACE_ARG_SIZE(2,0,1,2,0)
#define FMT__P_H_P_P                                    TRACE_ARG_SIZE(1,0,0,3,0)
#define FMT__P_H_P_L                                    TRACE_ARG_SIZE(1,0,1,2,0)
#define FMT__P_H_P_P_H                                  TRACE_ARG_SIZE(2,0,0,3,0)
#define FMT__P_H_P_P_P                                  TRACE_ARG_SIZE(1,0,0,4,0)
#define FMT__P_L                                        TRACE_ARG_SIZE(0,0,1,1,0)
#define FMT__P_L_D                                      TRACE_ARG_SIZE(0,1,1,1,0)
#define FMT__P_L_H                                      TRACE_ARG_SIZE(1,0,1,1,0)
#define FMT__P_L_P                                      TRACE_ARG_SIZE(0,0,1,2,0)
#define FMT__P_L_L_D                                    TRACE_ARG_SIZE(0,1,2,1,0)
#define FMT__P_L_L_L                                    TRACE_ARG_SIZE(0,0,3,1,0)
#define FMT__P_L_P_L                                    TRACE_ARG_SIZE(0,0,2,2,0)
#define FMT__P_L_P_P_P                                  TRACE_ARG_SIZE(0,0,1,4,0)
#define FMT__P_P                                        TRACE_ARG_SIZE(0,0,0,2,0)
#define FMT__P_P_D                                      TRACE_ARG_SIZE(0,1,0,2,0)
#define FMT__P_P_D_D                                    TRACE_ARG_SIZE(0,2,0,2,0)
#define FMT__P_P_D_D_H                                  TRACE_ARG_SIZE(1,2,0,2,0)
#define FMT__P_P_D_D_P                                  TRACE_ARG_SIZE(0,2,0,3,0)
#define FMT__P_P_D_H_H                                  TRACE_ARG_SIZE(2,1,0,2,0)
#define FMT__P_P_D_H_P                                  TRACE_ARG_SIZE(1,1,0,3,0)
#define FMT__P_P_D_P                                    TRACE_ARG_SIZE(0,1,0,3,0)
#define FMT__P_P_D_P_D                                  TRACE_ARG_SIZE(0,2,0,3,0)
#define FMT__P_P_D_P_H                                  TRACE_ARG_SIZE(1,1,0,3,0)
#define FMT__P_P_H                                      TRACE_ARG_SIZE(1,0,0,2,0)
#define FMT__P_P_H_D                                    TRACE_ARG_SIZE(1,1,0,2,0)
#define FMT__P_P_H_H                                    TRACE_ARG_SIZE(2,0,0,2,0)
#define FMT__P_P_H_H_P                                  TRACE_ARG_SIZE(2,0,0,3,0)
#define FMT__P_P_H_H_H                                  TRACE_ARG_SIZE(3,0,0,2,0)
#define FMT__P_P_H_P                                    TRACE_ARG_SIZE(1,0,0,3,0)
#define FMT__P_P_H_P_D_P                                TRACE_ARG_SIZE(1,2,0,4,0)
#define FMT__P_P_H_P_H                                  TRACE_ARG_SIZE(2,0,0,3,0)
#define FMT__P_P_L_D                                    TRACE_ARG_SIZE(0,1,1,2,0)
#define FMT__P_P_P                                      TRACE_ARG_SIZE(0,0,0,3,0)
#define FMT__P_P_P_H                                    TRACE_ARG_SIZE(1,0,0,3,0)
#define FMT__P_P_P_D                                    TRACE_ARG_SIZE(0,1,0,3,0)
#define FMT__P_P_P_D_P                                  TRACE_ARG_SIZE(0,1,0,4,0)
#define FMT__P_P_P_H_H                                  TRACE_ARG_SIZE(2,0,0,3,0)
#define FMT__P_P_P_P                                    TRACE_ARG_SIZE(0,0,0,4,0)
#define FMT__D_D_L_D                                    TRACE_ARG_SIZE(0,3,1,0,0)
#define FMT__D_D_L_L                                    TRACE_ARG_SIZE(0,2,2,0,0)
#define FMT__D_D_D_D_L_L                                TRACE_ARG_SIZE(0,4,2,0,0)
#define FMT__L_D_D_D_D_L_L                              TRACE_ARG_SIZE(0,4,3,0,0)
#define FMT__L_L_L_D_D                                  TRACE_ARG_SIZE(0,2,3,0,0)
#define FMT__D_H_P_H                                    TRACE_ARG_SIZE(2,1,0,1,0)
#define FMT__H_D_H_H_D                                  TRACE_ARG_SIZE(3,2,0,0,0)
#define FMT__D_A_B_H_H                                  TRACE_ARG_SIZE(2,1,0,0,3)
#define FMT__A_A_P                                      TRACE_ARG_SIZE(0,0,0,1,2)
#define FMT__B                                          TRACE_ARG_SIZE(0,0,0,0,2)
#define FMT__B_H_B                                      TRACE_ARG_SIZE(1,0,0,0,4)
#define FMT__A_D                                        TRACE_ARG_SIZE(0,1,0,0,1)
#define FMT__L_D_P_H                                    TRACE_ARG_SIZE(1,1,1,1,0)
#define FMT__D_D_H_H_H                                  TRACE_ARG_SIZE(3,2,0,0,0)
#define FMT__P_B_D_D                                    TRACE_ARG_SIZE(0,2,0,1,2)
#define FMT__H_B_H_B                                    TRACE_ARG_SIZE(2,0,0,0,4)
#define FMT__B_A                                        TRACE_ARG_SIZE(0,0,0,0,3)
#define FMT__D_H_A                                      TRACE_ARG_SIZE(1,1,0,0,1)
#define FMT__D_A_H                                      TRACE_ARG_SIZE(1,1,0,0,1)
#define FMT__D_D_D_A_B                                  TRACE_ARG_SIZE(0,3,0,0,3)
#define FMT__D_B_A                                      TRACE_ARG_SIZE(0,1,0,0,3)
#define FMT__H_B_A                                      TRACE_ARG_SIZE(1,0,0,0,3)
#define FMT__D_D_H_H_D_D_D_D                            TRACE_ARG_SIZE(2,6,0,0,0)
#define FMT__P_D_H_H_H_A                                TRACE_ARG_SIZE(3,1,0,1,1)
#define FMT__H_A_H_H_H_H_H                              TRACE_ARG_SIZE(6,0,0,0,1)
#define FMT__H_D_A_H                                    TRACE_ARG_SIZE(2,1,0,0,1)
#define FMT__D_A_H_D                                    TRACE_ARG_SIZE(1,2,0,0,1)
#define FMT__P_H_H_H_H_H_H_H_H                          TRACE_ARG_SIZE(8,0,0,1,0)
#define FMT__D_D_H_L_H_H                                TRACE_ARG_SIZE(3,2,1,0,0)
#define FMT__D_H_L_H_H                                  TRACE_ARG_SIZE(3,1,1,0,0)
#define FMT__D_H_L_L_D_D_D                              TRACE_ARG_SIZE(1,4,2,0,0)
#define FMT__H_D_H_L_H_H                                TRACE_ARG_SIZE(4,1,1,0,0)
#define FMT__D_H_D_H_L_H_H                              TRACE_ARG_SIZE(4,2,1,0,0)
#define FMT__D_D_H_H_H_D_D_D_D                          TRACE_ARG_SIZE(3,6,0,0,0)
#define FMT__H_D_H_L                                    TRACE_ARG_SIZE(2,1,1,0,0)
#define FMT__H_H_L_L                                    TRACE_ARG_SIZE(2,0,2,0,0)
#define FMT__D_D_H_H_D_H_D_D_D_D                        TRACE_ARG_SIZE(3,7,0,0,0)
#define FMT__D_A_D_A                                    TRACE_ARG_SIZE(0,2,0,0,2)
#define FMT__D_D_A                                      TRACE_ARG_SIZE(0,2,0,0,1)
#define FMT__H_H_H_L_H_H_H                              TRACE_ARG_SIZE(6,0,1,0,0)
#define FMT__H_D_D_D_D_D                                TRACE_ARG_SIZE(1,5,0,0,0)
#define FMT__P_P_D_D_L                                  TRACE_ARG_SIZE(0,2,1,2,0)
#define FMT__P_L_L                                      TRACE_ARG_SIZE(0,0,2,1,0)
#define FMT__P_P_L_L                                    TRACE_ARG_SIZE(0,0,2,2,0)
#define FMT__D_D_H_D_D_H                                TRACE_ARG_SIZE(2,4,0,0,0)
#define FMT__NC(N)                                      TRACE_ARG_SIZE(N,0,0,0,0)
#define FMT__H_L_H_H                                    TRACE_ARG_SIZE(3,0,1,0,0)
#define FMT__A_B                                        TRACE_ARG_SIZE(0,0,0,1,1)
#define FMT__D_H_H_L_D_D_D                              TRACE_ARG_SIZE(2,4,1,0,0)
#define FMT__H_H_D_D_H                                  TRACE_ARG_SIZE(3,3,0,0,0)
#define FMT__L_D_H_H_H_H                                TRACE_ARG_SIZE(4,1,1,0,0)
#define FMT__P_A_A                                      TRACE_ARG_SIZE(0,0,0,1,2)
#define FMT__D_D_P_H_H_H_H                              TRACE_ARG_SIZE(4,2,1,0,0)
#define FMT__P_P_D_D_D                                  TRACE_ARG_SIZE(0,4,2,0,0)
#define FMT__D_D_H_H_H_H                                TRACE_ARG_SIZE(2,4,0,0,0)
#define FMT__P_D_P_D_P_P                                TRACE_ARG_SIZE(4,2,0,0,0)
#define FMT__D_A_D                                      TRACE_ARG_SIZE(0,2,0,0,1)
#define FMT__D_H_D_D_D_D                                TRACE_ARG_SIZE(1,5,0,0,0)
#define FMT__H_D_D_D_H_H                                TRACE_ARG_SIZE(3,3,0,0,0)
#define FMT__P_H_D_D_D_H_H                              TRACE_ARG_SIZE(3,3,1,0,0)
#define FMT__H_H_H_D_H_L_H_H                            TRACE_ARG_SIZE(6,1,1,0,0)
#define FMT__D_D_P_P                                    TRACE_ARG_SIZE(0,2,2,0,0)
#define FMT__H_P_P_H_H                                  TRACE_ARG_SIZE(3,0,0,2,0)
#define FMT__H_P_H_H_P                                  TRACE_ARG_SIZE(3,0,0,2,0)
#define FMT__H_P_P                                      TRACE_ARG_SIZE(1,0,0,2,0)
#define FMT__H_P_P_H                                    TRACE_ARG_SIZE(2,0,0,2,0)



/** @} */ /* TRACE_DATA_FORMAT */

/**
 *  @addtogroup TRACE_FIRST_ARG
 *  @{
 */
#define TRACE_COMMON1 TRACE_SUBSYSTEM_COMMON, 1
#define TRACE_COMMON2 TRACE_SUBSYSTEM_COMMON, 2
#define TRACE_COMMON3 TRACE_SUBSYSTEM_COMMON, 3

/** @cond internals_doc */
/* osif subsystem is nearly not used. Place it to the same with common and free
 * 1 bit for buffers. */
#define TRACE_OSIF1 TRACE_SUBSYSTEM_COMMON, 1
#define TRACE_OSIF2 TRACE_SUBSYSTEM_COMMON, 2
#define TRACE_OSIF3 TRACE_SUBSYSTEM_COMMON, 3

#define TRACE_MAC1 TRACE_SUBSYSTEM_MAC, 1
#define TRACE_MAC2 TRACE_SUBSYSTEM_MAC, 2
#define TRACE_MAC3 TRACE_SUBSYSTEM_MAC, 3

#define TRACE_MACLL1 TRACE_SUBSYSTEM_MACLL, 1
#define TRACE_MACLL2 TRACE_SUBSYSTEM_MACLL, 2
#define TRACE_MACLL3 TRACE_SUBSYSTEM_MACLL, 3

#define TRACE_NWK1 TRACE_SUBSYSTEM_NWK, 1
#define TRACE_NWK2 TRACE_SUBSYSTEM_NWK, 2
#define TRACE_NWK3 TRACE_SUBSYSTEM_NWK, 3

#define TRACE_APS1 TRACE_SUBSYSTEM_APS, 1
#define TRACE_APS2 TRACE_SUBSYSTEM_APS, 2
#define TRACE_APS3 TRACE_SUBSYSTEM_APS, 3

#define TRACE_AF1 TRACE_SUBSYSTEM_AF, 1
#define TRACE_AF2 TRACE_SUBSYSTEM_AF, 2
#define TRACE_AF3 TRACE_SUBSYSTEM_AF, 3

#define TRACE_ZDO1 TRACE_SUBSYSTEM_ZDO, 1
#define TRACE_ZDO2 TRACE_SUBSYSTEM_ZDO, 2
#define TRACE_ZDO3 TRACE_SUBSYSTEM_ZDO, 3

#define TRACE_SECUR1 TRACE_SUBSYSTEM_SECUR, 1
#define TRACE_SECUR2 TRACE_SUBSYSTEM_SECUR, 2
#define TRACE_SECUR3 TRACE_SUBSYSTEM_SECUR, 3

#define TRACE_ZCL1 TRACE_SUBSYSTEM_ZCL, 1
#define TRACE_ZCL2 TRACE_SUBSYSTEM_ZCL, 2
#define TRACE_ZCL3 TRACE_SUBSYSTEM_ZCL, 3

#define TRACE_ZLL1 TRACE_SUBSYSTEM_ZLL, 1
#define TRACE_ZLL2 TRACE_SUBSYSTEM_ZLL, 2
#define TRACE_ZLL3 TRACE_SUBSYSTEM_ZLL, 3

#define TRACE_ZGP1 TRACE_SUBSYSTEM_ZGP, 1
#define TRACE_ZGP2 TRACE_SUBSYSTEM_ZGP, 2
#define TRACE_ZGP3 TRACE_SUBSYSTEM_ZGP, 3
#define TRACE_ZGP4 TRACE_SUBSYSTEM_ZGP, 4

#define TRACE_ZSE1 TRACE_SUBSYSTEM_ZSE, 1
#define TRACE_ZSE2 TRACE_SUBSYSTEM_ZSE, 2
#define TRACE_ZSE3 TRACE_SUBSYSTEM_ZSE, 3
#define TRACE_ZSE4 TRACE_SUBSYSTEM_ZSE, 4

#define TRACE_SPI1 TRACE_SUBSYSTEM_SPI, 1
#define TRACE_SPI2 TRACE_SUBSYSTEM_SPI, 2
#define TRACE_SPI3 TRACE_SUBSYSTEM_SPI, 3

#define TRACE_SSL1 TRACE_SUBSYSTEM_SSL, 1
#define TRACE_SSL2 TRACE_SUBSYSTEM_SSL, 2
#define TRACE_SSL3 TRACE_SUBSYSTEM_SSL, 3

#define TRACE_APP1 TRACE_SUBSYSTEM_APP, 1
#define TRACE_APP2 TRACE_SUBSYSTEM_APP, 2
#define TRACE_APP3 TRACE_SUBSYSTEM_APP, 3
#define TRACE_APP4 TRACE_SUBSYSTEM_APP, 4

#define TRACE_SPECIAL1 TRACE_SUBSYSTEM_SPECIAL1, 1
#define TRACE_SPECIAL2 TRACE_SUBSYSTEM_SPECIAL1, 2
#define TRACE_SPECIAL3 TRACE_SUBSYSTEM_SPECIAL1, 3

#define TRACE_USB1 TRACE_SUBSYSTEM_USB, 1
#define TRACE_USB2 TRACE_SUBSYSTEM_USB, 2
#define TRACE_USB3 TRACE_SUBSYSTEM_USB, 3

#define TRACE_MEMDBG1 TRACE_SUBSYSTEM_MEM, 1
#define TRACE_MEMDBG2 TRACE_SUBSYSTEM_MEM, 2
#define TRACE_MEMDBG3 TRACE_SUBSYSTEM_MEM, 3

#define TRACE_CLOUD1 TRACE_SUBSYSTEM_CLOUD, 1
#define TRACE_CLOUD2 TRACE_SUBSYSTEM_CLOUD, 2
#define TRACE_CLOUD3 TRACE_SUBSYSTEM_CLOUD, 3
#define TRACE_CLOUD4 TRACE_SUBSYSTEM_CLOUD, 4

#define TRACE_HTTP1 TRACE_SUBSYSTEM_HTTP, 1
#define TRACE_HTTP2 TRACE_SUBSYSTEM_HTTP, 2
#define TRACE_HTTP3 TRACE_SUBSYSTEM_HTTP, 3

#define TRACE_JSON1 TRACE_SUBSYSTEM_JSON, 1
#define TRACE_JSON2 TRACE_SUBSYSTEM_JSON, 2
#define TRACE_JSON3 TRACE_SUBSYSTEM_JSON, 3

#define TRACE_LWIP1 TRACE_SUBSYSTEM_LWIP, 1
#define TRACE_LWIP2 TRACE_SUBSYSTEM_LWIP, 2
#define TRACE_LWIP3 TRACE_SUBSYSTEM_LWIP, 3

#define TRACE_VENSTAR1 TRACE_SUBSYSTEM_VENSTAR, 1
#define TRACE_VENSTAR2 TRACE_SUBSYSTEM_VENSTAR, 2
#define TRACE_VENSTAR3 TRACE_SUBSYSTEM_VENSTAR, 3

#define TRACE_TRANSPORT1 TRACE_SUBSYSTEM_TRANSPORT, 1
#define TRACE_TRANSPORT2 TRACE_SUBSYSTEM_TRANSPORT, 2
#define TRACE_TRANSPORT3 TRACE_SUBSYSTEM_TRANSPORT, 3

#define TRACE_UART1 TRACE_SUBSYSTEM_UART, 1
#define TRACE_UART2 TRACE_SUBSYSTEM_UART, 2
#define TRACE_UART3 TRACE_SUBSYSTEM_UART, 3

#define TRACE_BATTERY1 TRACE_SUBSYSTEM_BATTERY, 1
#define TRACE_BATTERY2 TRACE_SUBSYSTEM_BATTERY, 2
#define TRACE_BATTERY3 TRACE_SUBSYSTEM_BATTERY, 3

#define TRACE_OTA1 TRACE_SUBSYSTEM_OTA, 1
#define TRACE_OTA2 TRACE_SUBSYSTEM_OTA, 2
#define TRACE_OTA3 TRACE_SUBSYSTEM_OTA, 3

#define TRACE_MAC_API1 TRACE_SUBSYSTEM_MAC_API, 1
#define TRACE_MAC_API2 TRACE_SUBSYSTEM_MAC_API, 2
#define TRACE_MAC_API3 TRACE_SUBSYSTEM_MAC_API, 3
/** @endcond */ /* internals_doc */

#ifndef ZB_SET_TRACE_LEVEL

/* empty definitions if not implemented */
#define ZB_SET_TRACE_LEVEL(l)
#define ZB_SET_TRACE_MASK(m)
#define ZB_SET_TRACE_OFF()
#define ZB_SET_TRACE_ON()
#define TRACE_ENTER_INT()
#define TRACE_LEAVE_INT()

#endif

/** @} */ /* TRACE_FIRST_ARG */
/** @} */ /* Debug trace */
/** @endcond */ /* DSR_TRACE */

/**
   @addtogroup DUMP_ON_OFF
   @{
*/
#if defined ZB_TRAFFIC_DUMP_ON || defined DOXYGEN
/** @cond internals_doc */
extern zb_uint8_t g_traf_dump;
/** @endcond */

/**
 * Switch Zigbee traffic dump OFF
 */
#define ZB_SET_TRAF_DUMP_OFF() g_traf_dump = 0
/**
 * Switch Zigbee traffic dump ON
 *
 * That function works only if traffic dump is enabled at compile time.
 */
#define ZB_SET_TRAF_DUMP_ON() g_traf_dump = 1
#else
#define ZB_SET_TRAF_DUMP_OFF()
#define ZB_SET_TRAF_DUMP_ON()
#endif
/** @} */ /* DUMP_ON_OFF */

/** @endcond */ /* DOXYGEN_DEBUG_SECTION */

#endif /* ZB_LOGGER_H */
