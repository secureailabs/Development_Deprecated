/*********************************************************************************************
 *
 * @file RestFrameworkSharedFunctions.h
 * @author Shabana Akhtar Baig
 * @date 12 Nov 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#include "CoreTypes.h"

typedef uint64_t (* SubmitRequestFn)(const Byte *, unsigned int, unsigned int *);
typedef bool (* GetResponseFn)(uint64_t, Byte *, unsigned int);
typedef bool (* RegisterPluginFn)(const char *, const char *, Qword, SubmitRequestFn, GetResponseFn, const Byte *, unsigned int);
typedef bool (* InitializePluginFn)(RegisterPluginFn);
typedef void (* ShutdownPluginFn)(void);
