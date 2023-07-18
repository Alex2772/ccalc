/**
 * LCD/OLED fonts library
 *
 * Copyright (c) 2016 Ruslan V. Uss (https://github.com/UncleRus),
 *                    Zaltora (https://github.com/Zaltora)
 * MIT Licensed as described in the file LICENSE
 */
#ifndef _EXTRAS_FONTS_H_
#define _EXTRAS_FONTS_H_

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    FONT_FACE_GLCD5x7 = 0,

    FONT_FACE_ROBOTO_8PT = 1,
    FONT_FACE_ROBOTO_10PT = 2,

    FONT_FACE_BITOCRA_4X7 = 3,
    FONT_FACE_BITOCRA_6X11 = 4,
    FONT_FACE_BITOCRA_7X13 = 5,

    FONT_FACE_TERMINUS_6X12_ISO8859_1 = 6,
    FONT_FACE_TERMINUS_8X14_ISO8859_1 = 7,
    FONT_FACE_TERMINUS_BOLD_8X14_ISO8859_1 = 8,
    FONT_FACE_TERMINUS_10X18_ISO8859_1 = 9,
    FONT_FACE_TERMINUS_BOLD_10X18_ISO8859_1 = 10,
    FONT_FACE_TERMINUS_11X22_ISO8859_1 = 11,
    FONT_FACE_TERMINUS_BOLD_11X22_ISO8859_1 = 12,
    FONT_FACE_TERMINUS_12X24_ISO8859_1 = 13,
    FONT_FACE_TERMINUS_BOLD_12X24_ISO8859_1 = 14,
    FONT_FACE_TERMINUS_14X28_ISO8859_1 = 15,
    FONT_FACE_TERMINUS_BOLD_14X28_ISO8859_1 = 16,
    FONT_FACE_TERMINUS_16X32_ISO8859_1 = 17,
    FONT_FACE_TERMINUS_BOLD_16X32_ISO8859_1 = 18,

    FONT_FACE_TERMINUS_6X12_KOI8_R = 19,
    FONT_FACE_TERMINUS_8X14_KOI8_R = 20,
    FONT_FACE_TERMINUS_BOLD_8X14_KOI8_R = 21,
    FONT_FACE_TERMINUS_14X28_KOI8_R = 22,
    FONT_FACE_TERMINUS_BOLD_14X28_KOI8_R = 23,
    FONT_FACE_TERMINUS_16X32_KOI8_R = 24,
    FONT_FACE_TERMINUS_BOLD_16X32_KOI8_R = 25,
} font_face_t;

/**
 * Character descriptor
 */
typedef struct _font_char_desc
{
    uint8_t width;   ///< Character width in pixel
    uint16_t offset; ///< Offset of this character in bitmap
} font_char_desc_t;

/**
 * Font information
 */
typedef struct _font_info
{
    uint8_t height;                           ///< Character height in pixel, all characters have same height
    uint8_t c;                                ///< Simulation of "C" width in TrueType term, the space between adjacent characters
    char char_start;                          ///< First character
    char char_end;                            ///< Last character
    const font_char_desc_t *char_descriptors; ///< descriptor for each character
    const uint8_t *bitmap;                    ///< Character bitmap
} font_info_t;

/**
 * Built-in fonts
 */
extern const font_info_t *font_builtin_fonts[];
extern const size_t font_builtin_fonts_count;

/**
 * Find character decriptor in font
 * @param fnt Poniter to font information struct
 * @param c Character
 * @return Character descriptor or NULL if no character found
 */
inline const font_char_desc_t *font_get_char_desc(const font_info_t *fnt, char ch)
{
    uint8_t c = (uint8_t)ch;
    return c < fnt->char_start || c > fnt->char_end
        ? NULL
        : fnt->char_descriptors + c - fnt->char_start;
}

/**
 * Calculate width of string in pixels
 * @param fnt Poniter to font information struct
 * @param s String
 * @return String width
 */
uint16_t font_measure_string(const font_info_t *fnt, const char *s);

#ifdef __cplusplus
}
#endif

#endif /* _EXTRAS_FONTS_H_ */
