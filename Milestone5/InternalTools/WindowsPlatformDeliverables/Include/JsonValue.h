/*********************************************************************************************
 *
 * @file JsonValue.h
 * @author Shabana Akhtar Baig
 * @date 08 Dec 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#pragma once

#include "CoreTypes.h"
#include "Object.h"
#include "StructuredBuffer.h"
#include "Utils.h"

#include "math.h"
#include "string.h"

#include <iostream>
#include <map>
#include <string>
#include <vector>

/********************************************************************************************/
class JsonValue;

typedef std::vector<JsonValue *> JsonArray;
typedef std::map<std::string, JsonValue *> JsonObject;

typedef enum type
{
    eNullType = 0,
    eNumberType = 1,
    eBooleanType = 2,
    eStringType = 3,
    eJsonArrayType = 4,
    eJsonObjectType = 5
}
JsonType;

/********************************************************************************************/

class JsonValue : public Object
{
    public:

        // Constructors and Destructor
        JsonValue(void);
        JsonValue(
            _in bool fBooleanValue
            );
        JsonValue(
            _in const char * c_pszCharValue
            );
        JsonValue(
            _in const std::string & c_strStringValue
            );
        JsonValue(
            _in double fl64DoubleValue
            );
        JsonValue(
            _in int nIntegerValue
            );
        JsonValue(
            _in const JsonArray & c_oJsonArray
            );
        JsonValue(
            _in const JsonObject & c_oJsonObject
            );
        JsonValue(
            _in const JsonValue & c_oJsonValue
            );
        virtual ~JsonValue(void);

        // Trim whitespaces from the data and parse the json encoded data to a StructuredBuffer
        static std::vector<Byte> __stdcall ParseDataToStructuredBuffer(
            _in const char * c_pszData
            );

        // Parse a StructuredBuffer to a Json object
        static JsonValue * __stdcall ParseStructuredBufferToJson(
            _in const StructuredBuffer & c_oStructuredBufferObject
            );

        // Check the type of the JsonValue
        bool __thiscall IsNull(void) const throw();
        bool __thiscall IsString(void) const throw();
        bool __thiscall IsBoolean(void) const throw();
        bool __thiscall IsNumber(void) const throw();
        bool __thiscall IsArray(void) const throw();
        bool __thiscall IsObject(void) const throw();

        // Fetch the type of the JsonValue
        const JsonType & __thiscall GetValueType(void);

        // Fetch value of the JsonValue
        const std::string & __thiscall GetStringValue(void) const throw();
        bool __thiscall GetBooleanValue(void) const throw();
        double __thiscall GetNumberValue(void) const throw();
        const JsonArray & __thiscall GetJsonArrayValue(void) const throw();
        const JsonObject & __thiscall GetJsonObjectValue(void) const throw();

        // Fetch the keys in a JsonObject
        std::vector<std::string> __thiscall GetObjectKeys(void) const throw();

        std::string ToString(void) const;

    private:

        // Parse json encoded string to a Json Object
        static JsonValue * __stdcall ToJson(
            _in const char ** c_pszdata
            );

        // Parse Json to a StructuredBuffer
        static std::vector<Byte> __stdcall JsonToStructuredBuffer(
            _in const JsonValue * poJsonValue
            );

        // Parse StructuredBuffer to Json
        static JsonObject __stdcall StructuredBufferToJson(
            _in const StructuredBuffer & c_oStructuredBufferObject
            );

        // Skip over space, \t, \r or \n
        static bool __stdcall SkipWhitespaceAndCheckEndOfData(
            _in const char ** c_pszdata
            );

        // Parse the JSON object and extracts a number
        static JsonValue * __stdcall ParseNumber(
            _in const char ** c_pszdata
            );
        // Parse the JSON object and extracts an object
        static JsonValue * ParseObject(
            _in const char ** c_pszdata
            );
        // Parse the JSON object and extracts an array
        static JsonValue * __stdcall ParseArray(
            _in const char ** c_pszdata
            );
        // Parse the JSON object and extracts a string
        static std::string __stdcall ParseString(
            _in const char ** c_pszdata
            );
        // Parse the JSON object and extracts whole part of the number
        static double __stdcall GetWholePart(
            _in const char ** c_pszdata
            );
        // Parse the JSON object and extracts decimal part of the number
        static double __stdcall GetDecimalPart(
            _in const char ** c_pszdata
            );

        // Fetch JsonValues associated with c_szKey
        JsonValue * __thiscall GetObjectElementValue(
            _in const char * c_pszKey
        ) const;

        // Private data members
        // Type of the JsonValue
        JsonType oJsonType;
        union
        {
            bool m_fBooleanValue;
            double m_fl64NumberValue;
            std::string * m_pstrStringValue;
            JsonArray * m_poJsonArray;
            JsonObject * m_poJsonObject;
        };

};
