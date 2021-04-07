/*********************************************************************************************
 *
 * @file CoreTypes.h
 * @author Luis Miguel Huapaya
 * @date 30 Sep 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#pragma once

#ifdef __cplusplus

#include <cstdint>
#include <cstddef>

extern "C"
{
	
#else
	
#include <stdint.h>
#include <stddef.h>

#endif

typedef uint8_t Byte;
typedef uint16_t Word;
typedef uint32_t Dword;
typedef uint64_t Qword;
typedef float float32_t;
typedef double float64_t;
typedef void * Handle;

#ifndef LODWORD
#define LODWORD(l)          ((uint32_t)((uint64_t)(l) & 0xFFFFFFFF))
#endif

#ifndef HIDWORD
#define HIDWORD(l)          ((uint32_t)((uint64_t)(l) >> 32))
#endif

#ifndef MAKE_WORD
#define MAKE_WORD(h, l)     ((((uint16_t) (0x00FF & (h))) << 8)|(0x00FF & ((uint16_t) (l))))
#endif

#ifndef MAKE_DWORD
#define MAKE_DWORD(h, l)    ((((uint32_t) (0x0000FFFF & (h))) << 16)|(0x0000FFFF & ((uint32_t) (l))))
#endif

#ifndef MAKE_QWORD
#define MAKE_QWORD(h, l)    ((((uint64_t) (0x00000000FFFFFFFF & (h))) << 32)|(0x00000000FFFFFFFF & ((uint64_t) (l))))
#endif

// Bit rotation intrinsics since Linux doesn't have those

#define BitsCount(Value) (sizeof(Value) * 8)
#define Shift(Value, Steps) (Steps % BitsCount(Value))

#define __rotl8(Value, Steps) (((Byte)Value << Shift((Byte)Value, Steps)) | ((Byte)Value >> (BitsCount((Byte)Value) - Shift((Byte)Value, Steps))))
#define __rotr8(Value, Steps) (((Byte)Value >> Shift((Byte)Value, Steps)) | ((Byte)Value << (BitsCount((Byte)Value) - Shift((Byte)Value, Steps))))

#define __rotl16(Value, Steps) (((Word)Value << Shift((Word)Value, Steps)) | ((Word)Value >> (BitsCount((Word)Value) - Shift((Word)Value, Steps))))
#define __rotr16(Value, Steps) (((Word)Value >> Shift((Word)Value, Steps)) | ((Word)Value << (BitsCount((Word)Value) - Shift((Word)Value, Steps))))

#define __rotl32(Value, Steps) (((Dword)Value << Shift((Dword)Value, Steps)) | ((Dword)Value >> (BitsCount((Dword)Value) - Shift((Dword)Value, Steps))))
#define __rotr32(Value, Steps) (((Dword)Value >> Shift((Dword)Value, Steps)) | ((Dword)Value << (BitsCount((Dword)Value) - Shift((Dword)Value, Steps))))

#define __rotl64(Value, Steps) (((Qword)Value << Shift((Qword)Value, Steps)) | ((Qword)Value >> (BitsCount((Qword)Value) - Shift((Qword)Value, Steps))))
#define __rotr64(Value, Steps) (((Qword)Value >> Shift((Qword)Value, Steps)) | ((Qword)Value << (BitsCount((Qword)Value) - Shift((Qword)Value, Steps))))

#define _in
#define _out
#define _inout

// Calling conventions. In Windows, this actually does something, but in Linux, no difference

#ifdef __linux__

#define __stdcall
#define __thiscall
#define __fastcall
#define __cdecl

#endif  // __linux__

#ifndef MAX_STRING_LENGTH
#define MAX_STRING_LENGTH 1073741824
#endif

#ifdef __cplusplus
}
#endif

