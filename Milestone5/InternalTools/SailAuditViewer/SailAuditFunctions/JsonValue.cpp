/*********************************************************************************************
 *
 * @file JsonValue.cpp
 * @author Shabana Akhtar Baig
 * @date 08 Dec 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#include "Base64Encoder.h"
#include "JsonValue.h"
#include "DebugLibrary.h"
#include "Exceptions.h"

/********************************************************************************************
 *
 * @function FreeJsonArray
 * @brief Free a JsonArray
 *
 ********************************************************************************************/

void __stdcall FreeJsonArray(
    _in JsonArray & oJsonArray
    )
{
    __DebugFunction();

    JsonArray::iterator itrJsonArray;
    for (itrJsonArray = oJsonArray.begin(); itrJsonArray != oJsonArray.end(); ++itrJsonArray)
    {
        delete *itrJsonArray;
    }
}

/********************************************************************************************
 *
 * @function FreeJsonObject
 * @brief Free a JsonObject
 *
 ********************************************************************************************/
void __stdcall FreeJsonObject(
    _in JsonObject & oJsonObject
    )
{
    __DebugFunction();

    JsonObject::iterator itrJsonObject;
    for (itrJsonObject = oJsonObject.begin(); itrJsonObject != oJsonObject.end(); ++itrJsonObject)
    {
        delete itrJsonObject->second;
    }
}

/********************************************************************************************
 *
 * @class JsonValue
 * @function JsonValue
 * @brief Constructor for Null type
 *
 ********************************************************************************************/

JsonValue::JsonValue(void)
{
    __DebugFunction();

    oJsonType = eNullType;
}

/********************************************************************************************
 *
 * @class JsonValue
 * @function JsonValue
 * @brief Constructor
 * @param[in] fBooleanValue boolean value
 *
 ********************************************************************************************/

JsonValue::JsonValue(
    _in bool fBooleanValue
    )
{
    __DebugFunction();

    oJsonType = eBooleanType;
    m_fBooleanValue = fBooleanValue;
}

/********************************************************************************************
 *
 * @class JsonValue
 * @function JsonValue
 * @brief Constructor
 * @param[in] c_pszCharValue pointer to a constant string
 *
 ********************************************************************************************/

JsonValue::JsonValue(
    _in const char * c_pszCharValue
    )
{
    __DebugFunction();

    oJsonType = eStringType;
    m_pstrStringValue = new std::string(std::string(c_pszCharValue));
}

/********************************************************************************************
 *
 * @class JsonValue
 * @function JsonValue
 * @brief Constructor
 * @param[in] c_strStringValue string value
 *
 ********************************************************************************************/

JsonValue::JsonValue(
    _in const std::string & c_strStringValue
    )
{
    __DebugFunction();

    oJsonType = eStringType;
    m_pstrStringValue = new std::string(c_strStringValue);
}

/********************************************************************************************
 *
 * @class JsonValue
 * @function JsonValue
 * @brief Constructor
 * @param[in] fl64DoubleValue double value
 *
 ********************************************************************************************/

JsonValue::JsonValue(
    _in double fl64DoubleValue
    )
{
    __DebugFunction();

    oJsonType = eNumberType;
    m_fl64NumberValue = fl64DoubleValue;
}

/********************************************************************************************
 *
 * @class JsonValue
 * @function JsonValue
 * @brief Constructor
 * @param[in] nIntegerValue integer value
 *
 ********************************************************************************************/

JsonValue::JsonValue(
    _in int nIntegerValue
    )
{
    __DebugFunction();

    oJsonType = eNumberType;
    m_fl64NumberValue = nIntegerValue;
}

/********************************************************************************************
 *
 * @class JsonValue
 * @function JsonValue
 * @brief Constructor
 * @param[in] c_oJsonArray json array
 *
 ********************************************************************************************/

JsonValue::JsonValue(
    _in const JsonArray & c_oJsonArray
    )
{
    __DebugFunction();

    oJsonType = eJsonArrayType;
    m_poJsonArray = new JsonArray(c_oJsonArray);
}

/********************************************************************************************
 *
 * @class JsonValue
 * @function JsonValue
 * @brief Constructor
 * @param[in] c_oJsonObject json object
 *
 ********************************************************************************************/

JsonValue::JsonValue(
    _in const JsonObject & c_oJsonObject
    )
{
    __DebugFunction();

    oJsonType = eJsonObjectType;
    m_poJsonObject = new JsonObject(c_oJsonObject);
}

/********************************************************************************************
 *
 * @class JsonValue
 * @function JsonValue
 * @brief Copy Constructor
 * @param[in] c_oJsonValue the other JsonValue object
 *
 ********************************************************************************************/

JsonValue::JsonValue(
    _in const JsonValue & c_oJsonValue
    )
{
    __DebugFunction();

    oJsonType = c_oJsonValue.oJsonType;

    switch(oJsonType)
    {
        case eBooleanType
        :
            m_fBooleanValue = c_oJsonValue.m_fBooleanValue;
            break;

        case eStringType
        :
            m_pstrStringValue = new std::string(*c_oJsonValue.m_pstrStringValue);
            break;

        case eNumberType
        :
            m_fl64NumberValue = c_oJsonValue.m_fl64NumberValue;
            break;

        case eJsonArrayType
        :
        {
            JsonArray oJsonArray = *c_oJsonValue.m_poJsonArray;
            JsonArray::iterator itrJsonArray;
            m_poJsonArray = new JsonArray();
            for (itrJsonArray = oJsonArray.begin(); itrJsonArray != oJsonArray.end(); ++itrJsonArray)
            {
                m_poJsonArray->push_back(new JsonValue(**itrJsonArray));
            }

            break;
        }

        case eJsonObjectType
        :
        {
            JsonObject oJsonObject = *c_oJsonValue.m_poJsonObject;
            JsonObject::iterator itrJsonObject;
            m_poJsonObject = new JsonObject();
            for (itrJsonObject = oJsonObject.begin(); itrJsonObject != oJsonObject.end(); ++itrJsonObject)
            {
                (*m_poJsonObject)[itrJsonObject->first] = new JsonValue(*(itrJsonObject->second));
            }

            break;
        }
    }
}

/********************************************************************************************
 *
 * @class JsonValue
 * @function ~JsonValue
 * @brief Destructor
 *
 ********************************************************************************************/

JsonValue::~JsonValue(void)
{
    __DebugFunction();

    if (eStringType == oJsonType)
    {
        delete m_pstrStringValue;
    }

    else if (eJsonArrayType == oJsonType)
    {
        for (JsonArray::iterator itrJsonArray = m_poJsonArray->begin(); itrJsonArray != m_poJsonArray->end(); ++itrJsonArray)
        {
            delete *itrJsonArray;
        }

        delete m_poJsonArray;
    }

    else if (eJsonObjectType == oJsonType)
    {
        for (JsonObject::iterator itrJsonObject = m_poJsonObject->begin(); itrJsonObject != m_poJsonObject->end(); ++itrJsonObject)
        {
            delete itrJsonObject->second;
        }

        delete m_poJsonObject;
    }
}

/********************************************************************************************
 *
 * @class JsonValue
 * @function ParseDataToStructuredBuffer
 * @brief Parses Json encoded data to a StructuredBuffer
 * @param[in] c_pszData pointer to the request data
 * return Data as a serialized buffer
 *
 ********************************************************************************************/

std::vector<Byte> __stdcall JsonValue::ParseDataToStructuredBuffer(
    _in const char * c_pszData
    )
{
    __DebugFunction();

    std::vector<Byte> stlJsonValue;

    if (true == SkipWhitespaceAndCheckEndOfData(&c_pszData))
    {
        // Parse data to json object
        JsonValue * poJsonValue = ToJson(&c_pszData);
        if (nullptr != poJsonValue)
        {
            if (false == SkipWhitespaceAndCheckEndOfData(&c_pszData))
            {
                // Parse json object to a StructuredBuffer and return the serialized buffer
                stlJsonValue = JsonToStructuredBuffer(poJsonValue);
                delete poJsonValue;
            }
        }
        else
        {
            _ThrowBaseException("Invalid request body", nullptr);
        }
    }

    return stlJsonValue;
}

/********************************************************************************************
 *
 * @class JsonValue
 * @function ParseStructuredBufferToJson
 * @brief Parses a StructuredBuffer to a Json object
 * @param[in] c_pszData reference to the structure buffer instance
 * return pointer to a JsonValue object
 *
 ********************************************************************************************/

JsonValue * __stdcall JsonValue::ParseStructuredBufferToJson(
    _in const StructuredBuffer & c_oStructuredBufferObject
    )
{
    __DebugFunction();

    JsonValue * oJsonValue = new JsonValue(StructuredBufferToJson(c_oStructuredBufferObject));

    return oJsonValue;

}

/********************************************************************************************
 *
 * @class JsonValue
 * @function IsNull
 * @brief Check if JsonValue if of type Null
 * @param[in] c_pszData pointer to the data pointer
 * @return true if type is Null
 * @return false otherwise
 *
 ********************************************************************************************/

bool __thiscall JsonValue::IsNull(void) const throw()
{
    __DebugFunction();

    bool fIsNull = (oJsonType == eNullType);

    return fIsNull;
}

/********************************************************************************************
 *
 * @class JsonValue
 * @function IsString
 * @brief Check if JsonValue if of type string
 * @param[in] c_pszData pointer to the data pointer
 * @return true if type is string
 * @return false otherwise
 *
 ********************************************************************************************/

bool __thiscall JsonValue::IsString(void) const throw()
{
    __DebugFunction();

    bool fIsString = (oJsonType == eStringType);

    return fIsString;
}

/********************************************************************************************
 *
 * @class JsonValue
 * @function IsBoolean
 * @brief Check if JsonValue if of type Boolean
 * @param[in] c_pszData pointer to the data pointer
 * @return true if type is Boolean
 * @return false otherwise
 *
 ********************************************************************************************/

bool __thiscall JsonValue::IsBoolean(void) const throw()
{
    __DebugFunction();

    bool fIsBoolean = (oJsonType == eBooleanType);

    return fIsBoolean;
}

/********************************************************************************************
 *
 * @class JsonValue
 * @function IsNumber
 * @brief Check if JsonValue if of type eNumberType
 * @param[in] c_pszData pointer to the data pointer
 * @return true if type is eNumberType
 * @return false otherwise
 *
 ********************************************************************************************/

bool __thiscall JsonValue::IsNumber(void) const throw()
{
    bool fIsNumber = (oJsonType == eNumberType);

    return fIsNumber;
}

/********************************************************************************************
 *
 * @class JsonValue
 * @function IsArray
 * @brief Check if JsonValue if of type JsonArray
 * @param[in] c_pszData pointer to the data pointer
 * @return true if type is JsonArray
 * @return false otherwise
 *
 ********************************************************************************************/

bool __thiscall JsonValue::IsArray(void) const throw()
{
    __DebugFunction();

    bool fIsArray = (oJsonType == eJsonArrayType);

    return fIsArray;
}

/********************************************************************************************
 *
 * @class JsonValue
 * @function IsObject
 * @brief Check if JsonValue if of type JsonObject
 * @param[in] c_pszData pointer to the data pointer
 * @return true if type is jsonObject
 * @return false otherwise
 *
 ********************************************************************************************/

bool __thiscall JsonValue::IsObject(void) const throw()
{
    __DebugFunction();

    bool fIsObject = (oJsonType == eJsonObjectType);

    return fIsObject;
}

/********************************************************************************************
 *
 * @class JsonValue
 * @function GetValueType
 * @brief Fetch Type of JsonValue
 * @param[in] c_pszData pointer to the data pointer
 * @return oJsonType
 *
 ********************************************************************************************/

const JsonType & __thiscall JsonValue::GetValueType(void)
{
    __DebugFunction();

    return oJsonType;
}

/********************************************************************************************
 *
 * @class JsonValue
 * @function GetStringValue
 * @brief Fetch string value
 * @param[in] c_pszData pointer to the data pointer
 * @return String value
 *
 ********************************************************************************************/

const std::string & __thiscall JsonValue::GetStringValue(void) const throw()
{
    __DebugFunction();

    return (* m_pstrStringValue);
}

/********************************************************************************************
 *
 * @class JsonValue
 * @function GetBooleanValue
 * @brief Fetch boolean value
 * @param[in] c_pszData pointer to the data pointer
 * @return Boolean value
 *
 ********************************************************************************************/

bool __thiscall JsonValue::GetBooleanValue(void) const throw()
{
    __DebugFunction();

    return m_fBooleanValue;
}

/********************************************************************************************
 *
 * @class JsonValue
 * @function GetNumberValue
 * @brief Fetch double value
 * @param[in] c_pszData pointer to the data pointer
 * @return Double value
 *
 ********************************************************************************************/

double __thiscall JsonValue::GetNumberValue(void) const throw()
{
    __DebugFunction();

    return m_fl64NumberValue;
}

/********************************************************************************************
 *
 * @class JsonValue
 * @function GetJsonArrayValue
 * @brief Fetch JsonArray
 * @param[in] c_pszData pointer to the data pointer
 * @return JsonArray
 *
 ********************************************************************************************/

const JsonArray & __thiscall JsonValue::GetJsonArrayValue(void) const throw()
{
    __DebugFunction();

    return (* m_poJsonArray);
}

/********************************************************************************************
 *
 * @class JsonValue
 * @function GetJsonObjectValue
 * @brief Fetch JsonObject
 * @param[in] c_pszData pointer to the data pointer
 * @return JsonObject
 *
 ********************************************************************************************/

const JsonObject & __thiscall JsonValue::GetJsonObjectValue(void) const throw()
{
    __DebugFunction();

    return (* m_poJsonObject);
}

/********************************************************************************************
 *
 * @class JsonValue
 * @function GetObjectKeys
 * @brief Get keys in JsonObject
 * @param[in] c_pszData pointer to the data pointer
 * @return Keys in JsonObject
 *
 ********************************************************************************************/

std::vector<std::string> __thiscall JsonValue::GetObjectKeys(void) const throw()
{
    __DebugFunction();

    std::vector<std::string> stlObjectKeys;

    if (eJsonObjectType == oJsonType)
    {
        for (JsonObject::iterator itrJsonObject = m_poJsonObject->begin(); itrJsonObject != m_poJsonObject->end(); ++itrJsonObject)
        {
            stlObjectKeys.push_back(itrJsonObject->first);
        }
    }

    return stlObjectKeys;
}

/********************************************************************************************
 *
 * @class JsonValue
 * @function ToString
 * @brief Return json encoded string
 * @return JsonValue encoded string
 *
 ********************************************************************************************/

std::string JsonValue::ToString(void) const
{
    __DebugFunction();

    std::string strOutputString;

    switch(oJsonType)
    {
        case eNullType
        :
            strOutputString = "null";
        case eBooleanType
        :
            strOutputString = m_fBooleanValue ? "true" : "false";
            break;

        case eStringType
        :
            strOutputString = ::EscapeJsonString((*m_pstrStringValue));
            break;

        case eNumberType
        :
        {
            if((true == isinf(m_fl64NumberValue)) || (true == isnan(m_fl64NumberValue)))
            {
                strOutputString = "null";
            }
            else
            {
                strOutputString = std::to_string(m_fl64NumberValue);
            }
            break;
        }
        case eJsonObjectType
        :
        {
            strOutputString = "{";
            JsonObject::iterator itrJsonObject;
            for (itrJsonObject = m_poJsonObject->begin(); itrJsonObject != m_poJsonObject->end();)
            {
                strOutputString += ::EscapeJsonString((itrJsonObject->first).c_str());
                strOutputString += ":";
                strOutputString += itrJsonObject->second->ToString();

                if (m_poJsonObject->end() != ++itrJsonObject)
                {
                    strOutputString += ",";
                }
            }
            strOutputString += "}";
            break;
        }
    }

    return strOutputString;
}

/********************************************************************************************
 *
 * @class JsonValue
 * @function ToJson
 * @brief Extracts a JsonValue
 * @param[in] c_pszData pointer to the request data
 * return pointer to a JsonValue object
 *
 ********************************************************************************************/

JsonValue * JsonValue::ToJson(
    _in const char ** c_pszData
    )
{
    __DebugFunction();

    JsonValue * poJsonValue = nullptr;

    // Check for string
    if ('"' == **c_pszData)
    {
        std::string strStringValue = ParseString(c_pszData);
        if (0 != strStringValue.length())
        {
            poJsonValue = new JsonValue(strStringValue);
        }
    }

    // Check if its a boolean and if it equals to true
    else if ((4 == strnlen(*c_pszData, 4)) && (0 == ::_strnicmp(*c_pszData, "true", 4)))
    {
        (*c_pszData) += 4;
        poJsonValue = new JsonValue(true);
    }

    // Check if its a boolean and if it equals to false
    else if ((5 == strnlen(*c_pszData, 5)) && (0 == _strnicmp(*c_pszData, "false", 5)))
    {
        (*c_pszData) += 5;
        poJsonValue = new JsonValue(false);
    }

    // Check if number
    else if (('-' == **c_pszData) || (('0' <= **c_pszData) && ('9' >= **c_pszData)))
    {
        poJsonValue = ParseNumber(c_pszData);
    }

    // Check if object
    else if ('{' == **c_pszData)
    {
        (*c_pszData)++;

        poJsonValue = ParseObject(c_pszData);
    }

    // Check if array
    else if ('[' == **c_pszData)
    {
        (*c_pszData)++;

        poJsonValue = ParseArray(c_pszData);
    }

    // Check if null
    else if ((4 == strnlen(*c_pszData, 4)) && (0 == _strnicmp(*c_pszData, "null", 4)))
    {
        (*c_pszData) += 4;
        poJsonValue = new JsonValue();
    }

    return poJsonValue;
}

/********************************************************************************************
 *
 * @function JsonToStructuredBuffer
 * @brief Adds JsonValue and its Child JsonValues to a StructuredBuffer
 * @param[in] poJsonValue pointer to the JsonValue object
 * return Serialized StructuredBuffer containing the JsonValue and its child JsonValues
 *
 ********************************************************************************************/

 std::vector<Byte> __stdcall JsonValue::JsonToStructuredBuffer(
     _in const JsonValue * poJsonValue
     )
{
    __DebugFunction();

    StructuredBuffer oJsonValueStructuredBuffer;

    std::vector<std::string> stlJsonKeys = poJsonValue->GetObjectKeys();
	std::vector<std::string>::iterator itrJsonKeys = stlJsonKeys.begin();
	while (itrJsonKeys != stlJsonKeys.end())
	{
		// Get the key's value.
		JsonValue * poJsonChildValue = poJsonValue->GetObjectElementValue((*itrJsonKeys).c_str());
		if (poJsonChildValue)
		{
			JsonType oType = poJsonChildValue->GetValueType();

            switch(oType)
            {
            	case eNullType
            	:
            		oJsonValueStructuredBuffer.PutNull((*itrJsonKeys).c_str());
            		break;
                case eNumberType
            	:
                    oJsonValueStructuredBuffer.PutFloat64((*itrJsonKeys).c_str(), poJsonChildValue->GetNumberValue());
            		break;
            	case eBooleanType
            	:
                    oJsonValueStructuredBuffer.PutBoolean((*itrJsonKeys).c_str(), poJsonChildValue->GetBooleanValue());
            		break;
            	case eStringType
            	:
                    oJsonValueStructuredBuffer.PutString((*itrJsonKeys).c_str(), poJsonChildValue->GetStringValue());
            		break;
            	case eJsonArrayType
            	:
                {
                    // TODO: add parsing for eJsonArrayType
                    JsonArray oJsonArray = poJsonChildValue->GetJsonArrayValue();
                    StructuredBuffer oArrayValue;
                    for (unsigned int unIndex = 0; unIndex < oJsonArray.size(); ++unIndex)
                    {
                        JsonType oArrayMemberType = oJsonArray[unIndex]->GetValueType();
                        std::string strElementName = *itrJsonKeys + std::to_string(unIndex);
                        switch(oArrayMemberType)
            			{
            				case eNullType
            				:
            					oArrayValue.PutNull(strElementName.c_str());
            					break;
                            case eNumberType
            				:
                                oArrayValue.PutFloat64(strElementName.c_str(), oJsonArray[unIndex]->GetNumberValue());
            					break;
            				case eBooleanType
            				:
                                oArrayValue.PutBoolean(strElementName.c_str(), oJsonArray[unIndex]->GetBooleanValue());
            					break;
            				case eStringType
            				:
                            {
                                oArrayValue.PutString(strElementName.c_str(), oJsonArray[unIndex]->GetStringValue());
            					break;
                            }
                            case eJsonObjectType
            				:
                            {
                                StructuredBuffer oObject(JsonToStructuredBuffer(oJsonArray[unIndex]));
                                oArrayValue.PutStructuredBuffer(strElementName.c_str(), oObject);
            					break;
                            }
                        }
                    }
            		oJsonValueStructuredBuffer.PutStructuredBuffer((*itrJsonKeys).c_str(), oArrayValue);
            		break;
                }
            	case eJsonObjectType
            	:
                    StructuredBuffer oObjectValue(JsonToStructuredBuffer(poJsonChildValue));
                    oJsonValueStructuredBuffer.PutStructuredBuffer((*itrJsonKeys).c_str(), oObjectValue);
            		break;
            }
		}
		itrJsonKeys++;
	}

    return oJsonValueStructuredBuffer.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @function StructuredBufferToJson
 * @brief Adds JsonValue and its Child JsonValues to a StructuredBuffer
 * @param[in] poJsonValue pointer to the JsonValue object
 * return Serialized StructuredBuffer containing the JsonValue and its child JsonValues
 *
 ********************************************************************************************/

JsonObject __stdcall JsonValue::StructuredBufferToJson(
    _in const StructuredBuffer & c_oStructuredBufferObject
    )
{
    __DebugFunction();

    JsonObject oJsonObject;

    JsonValue * poJsonValue = nullptr;
    // Get name and type of all elements in c_oStructuredBufferObject
    std::vector<std::string> stlStructuredBufferElements = c_oStructuredBufferObject.GetDescriptionOfElements();
	std::vector<std::string>::iterator itrStructuredBufferElements = stlStructuredBufferElements.begin();
	while (itrStructuredBufferElements != stlStructuredBufferElements.end())
	{
        // Get name and type of the element
        unsigned int unPositionOfName = (unsigned int) (*itrStructuredBufferElements).find("Name[") + 5;
        unsigned int unPositionOfEndOfName = (unsigned int) (*itrStructuredBufferElements).find("],Type[");
        unsigned int unPositionOfEndOfType = (unsigned int) (*itrStructuredBufferElements).find("],Size[");
        std::string strNameOfElement = (*itrStructuredBufferElements).substr(unPositionOfName, unPositionOfEndOfName - unPositionOfName);
        Dword dwType = (Dword) std::stol((*itrStructuredBufferElements).substr(unPositionOfEndOfName + 7, unPositionOfEndOfType - unPositionOfEndOfName + 7), nullptr);

        // Create a JsonValue based on the type of the element
        switch(dwType)
        {
            case NULL_VALUE_TYPE
            :
                poJsonValue = new JsonValue();
                break;
            case FLOAT32_VALUE_TYPE
            :
                poJsonValue = new JsonValue((double) c_oStructuredBufferObject.GetFloat32(strNameOfElement.c_str()));
                break;
            case FLOAT64_VALUE_TYPE
            :
                poJsonValue = new JsonValue((double) c_oStructuredBufferObject.GetFloat64(strNameOfElement.c_str()));
                break;
            case INT8_VALUE_TYPE
            :
                poJsonValue = new JsonValue((double) c_oStructuredBufferObject.GetInt8(strNameOfElement.c_str()));
                break;
            case INT16_VALUE_TYPE
            :
                poJsonValue = new JsonValue((double) c_oStructuredBufferObject.GetInt16(strNameOfElement.c_str()));
                break;
            case INT32_VALUE_TYPE
            :
                poJsonValue = new JsonValue((double) c_oStructuredBufferObject.GetInt32(strNameOfElement.c_str()));
                break;
            case INT64_VALUE_TYPE
            :
                poJsonValue = new JsonValue((double) c_oStructuredBufferObject.GetInt64(strNameOfElement.c_str()));
                break;
            case UINT8_VALUE_TYPE
            :
                poJsonValue = new JsonValue((double) c_oStructuredBufferObject.GetUnsignedInt8(strNameOfElement.c_str()));
                break;
            case UINT16_VALUE_TYPE
            :
                poJsonValue = new JsonValue((double) c_oStructuredBufferObject.GetUnsignedInt16(strNameOfElement.c_str()));
                break;
            case UINT32_VALUE_TYPE
            :
                poJsonValue = new JsonValue((double) c_oStructuredBufferObject.GetUnsignedInt32(strNameOfElement.c_str()));
                break;
            case UINT64_VALUE_TYPE
            :
                poJsonValue = new JsonValue((double) c_oStructuredBufferObject.GetUnsignedInt64(strNameOfElement.c_str()));
                break;
            case BYTE_VALUE_TYPE
            :
                poJsonValue = new JsonValue((double) c_oStructuredBufferObject.GetByte(strNameOfElement.c_str()));
                break;
            case WORD_VALUE_TYPE
            :
                poJsonValue = new JsonValue((double) c_oStructuredBufferObject.GetWord(strNameOfElement.c_str()));
                break;
            case DWORD_VALUE_TYPE
            :
                poJsonValue = new JsonValue((double) c_oStructuredBufferObject.GetDword(strNameOfElement.c_str()));
                break;
            case QWORD_VALUE_TYPE
            :
                poJsonValue = new JsonValue((double) c_oStructuredBufferObject.GetQword(strNameOfElement.c_str()));
                break;
            case BOOLEAN_VALUE_TYPE
            :
                poJsonValue = new JsonValue(c_oStructuredBufferObject.GetBoolean(strNameOfElement.c_str()));
                break;
            case ANSI_CHARACTER_STRING_VALUE_TYPE
            :
                poJsonValue = new JsonValue(c_oStructuredBufferObject.GetString(strNameOfElement.c_str()));
                break;
            case GUID_VALUE_TYPE
            :
                poJsonValue = new JsonValue(c_oStructuredBufferObject.GetGuid(strNameOfElement.c_str()).ToString(eHyphensAndCurlyBraces));
                break;
            case BUFFER_VALUE_TYPE
            :
            {
                std::vector<Byte> stlValue = c_oStructuredBufferObject.GetBuffer(strNameOfElement.c_str());
                poJsonValue = new JsonValue(::Base64Encode(&stlValue[0], static_cast<unsigned int>(stlValue.size())));
                break;
            }
            case INDEXED_BUFFER_VALUE_TYPE
            :
                poJsonValue = new JsonValue(StructuredBufferToJson(c_oStructuredBufferObject.GetStructuredBuffer(strNameOfElement.c_str())));
                break;

        }
        // Add the JsonValue to the JsonObject
        oJsonObject[strNameOfElement] = poJsonValue;
		itrStructuredBufferElements++;
	}

    return oJsonObject;
}

/********************************************************************************************
 *
 * @class JsonValue
 * @function SkipWhitespaceAndCheckEndOfData
 * @brief Skip over space, \t, \r or \n
 * @param[in] c_pszData pointer to the data pointer
 *
 ********************************************************************************************/

bool __stdcall JsonValue::SkipWhitespaceAndCheckEndOfData(
    _in const char ** c_pszData
    )
{
    __DebugFunction();

    bool fIsEnd = false;

    // Skip over whitespace characters in the data
    while ((0 != **c_pszData) && ((' ' == **c_pszData) || ('\t' == **c_pszData) || ('\r' == **c_pszData) || ('\n' == **c_pszData)))
    {
        (*c_pszData)++;
    }

    // Check if end of data
    fIsEnd = (0 != **(c_pszData));

    return fIsEnd;
}

/********************************************************************************************
 *
 * @class JsonValue
 * @function ParseNumber
 * @brief Parse JsonValue of type Number
 * @param[in] c_pszData pointer to the data pointer
 * @return JsonValue of type Number
 *
 ********************************************************************************************/

JsonValue * __stdcall JsonValue::ParseNumber(
    _in const char ** c_pszData
    )
{
    __DebugFunction();

    JsonValue * poJsonValue = nullptr;
    double fl64Number = 0.0;

    bool fIsNegative = false, fSuccess = true;
    if ('-' == **c_pszData)
    {
        fIsNegative = true;
        (*c_pszData)++;
    }

    // // Ignore the first 0s
    // while ('0' == **c_pszData)
    // {
    //     (*c_pszData)++;
    // }

    // Parse whole part of the integer
    if (('0' <= **c_pszData) && ('9' >= **c_pszData))
    {
        fl64Number = GetWholePart(c_pszData);
    }
    else
    {
        fSuccess = false;
    }

    // Parse decimal part
    if (('.' == **c_pszData) && (true == fSuccess))
    {
        (*c_pszData)++;
        if (('0' > **c_pszData) && ('9' < **c_pszData))
        {
            fSuccess = false;
        }
        else
        {
            double fl64DecimalPart = GetDecimalPart(c_pszData);
            fl64Number += fl64DecimalPart;
        }
    }

    // Parse exponent part
    if (('E' == **c_pszData) || ('e' == **c_pszData) && (true == fSuccess))
    {
        (*c_pszData)++;

        bool fIsNegativePower = false;
        if ('-' == **c_pszData){
            fIsNegative = true;
            (*c_pszData)++;
        }
        else if('+' == **c_pszData)
        {
            (*c_pszData)++;
        }

        // Check for valid digits
        if (('0' > **c_pszData) && ('9' < **c_pszData))
        {
            fSuccess = false;
        }
        else
        {
            // Get the exponent part
            double fl64ExponentPart = GetWholePart(c_pszData);
            // In JSON E and e represents a base 10 exponent
            for (double fl64Index = 0.0; fl64Index < fl64ExponentPart; fl64Index++)
            {
                if (true == fIsNegativePower)
                {
                    fl64Number /= 10.0;
                }
                else
                {
                    fl64Number *= 10.0;
                }
            }
        }
    }

    if ((true == fIsNegative) && (true == fSuccess))
    {
        fl64Number *= -1;
    }

    if (true == fSuccess)
    {
        poJsonValue = new JsonValue(fl64Number);
    }

    return poJsonValue;
}

/********************************************************************************************
 *
 * @class JsonValue
 * @function ParseObject
 * @brief Parse JsonValue of type JsonObject
 * @param[in] c_pszData pointer to the data pointer
 * @return JsonValue of type JsonObject
 *
 ********************************************************************************************/

JsonValue * JsonValue::ParseObject(
    _in const char ** c_pszData
    )
{
    __DebugFunction();

    JsonValue * poJsonValue = nullptr;
    JsonObject oJsonObject;
    bool fStop = false;

    while ((0 != **c_pszData) && (false == fStop))
    {
        if (false == SkipWhitespaceAndCheckEndOfData(c_pszData))
        {
            FreeJsonObject(oJsonObject);
            fStop = true;
        }

        else
        {
            if ((0 == oJsonObject.size()) && ('}' == **c_pszData))
            {
                (*c_pszData)++;
                poJsonValue = new JsonValue(oJsonObject);
                fStop = true;
            }
            else
            {
                std::string strName = ParseString(c_pszData);
                if (0 == strName.length())
                {
                    FreeJsonObject(oJsonObject);
                    fStop = true;
                }
                else
                {
                    if (false == SkipWhitespaceAndCheckEndOfData(c_pszData))
                    {
                        FreeJsonObject(oJsonObject);
                        fStop = true;
                    }
                    else
                    {
                        if (':' != **c_pszData)
                        {
                            FreeJsonObject(oJsonObject);
                            fStop = true;
                        }
                        else
                        {
                            (*c_pszData)++;
                            if (false == SkipWhitespaceAndCheckEndOfData(c_pszData))
                            {
                                FreeJsonObject(oJsonObject);
                                fStop = true;
                            }
                            else
                            {
                                JsonValue * poOblectElementValue = ToJson(c_pszData);
                                if (nullptr == poOblectElementValue)
                                {
                                    FreeJsonObject(oJsonObject);
                                    fStop = true;
                                }
                                else
                                {
                                    oJsonObject[strName] = poOblectElementValue;
                                    if (false == SkipWhitespaceAndCheckEndOfData(c_pszData))
                                    {
                                        FreeJsonObject(oJsonObject);
                                        fStop = true;
                                    }
                                    else
                                    {
                                        if ('}' == **c_pszData)
                                        {
                                            (*c_pszData)++;
                                            poJsonValue = new JsonValue(oJsonObject);
                                            fStop = true;
                                        }
                                        else if (',' != **c_pszData)
                                        {
                                            FreeJsonObject(oJsonObject);
                                            fStop = true;
                                        }
                                        else
                                        {
                                            (*c_pszData)++;
                                        }

                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return poJsonValue;
}

/********************************************************************************************
 *
 * @class JsonValue
 * @function ParseArray
 * @brief Parse JsonValue of type JsonArray
 * @param[in] c_pszData pointer to the data pointer
 * @return JsonValue of type JsonArray
 *
 ********************************************************************************************/

JsonValue * __stdcall JsonValue::ParseArray(
    _in const char ** c_pszData
    )
{
    __DebugFunction();

    JsonValue * poJsonValue = nullptr;
    JsonArray oJsonArray;
    bool fStop = false;

    while ((0 != **c_pszData) && (false == fStop))
    {
        if (false == SkipWhitespaceAndCheckEndOfData(c_pszData))
        {
            FreeJsonArray(oJsonArray);
            fStop = true;
        }
        else
        {
            if ((0 == oJsonArray.size()) && (']' == **c_pszData))
            {
                (*c_pszData)++;
                poJsonValue = new JsonValue(oJsonArray);
                fStop = true;
            }
            else
            {
                JsonValue * oJsonValue = ToJson(c_pszData);
                if (nullptr == oJsonValue)
                {
                    FreeJsonArray(oJsonArray);
                    fStop = true;
                }
                else
                {
                    oJsonArray.push_back(oJsonValue);
                    if (false == SkipWhitespaceAndCheckEndOfData(c_pszData))
                    {
                        FreeJsonArray(oJsonArray);
                        fStop = true;
                    }
                    else
                    {
                        if (']' == **c_pszData)
                        {
                            (*c_pszData)++;
                            poJsonValue = new JsonValue(oJsonArray);
                            fStop = true;
                        }
                        else
                        {
                            if (',' != **c_pszData)
                            {
                                FreeJsonArray(oJsonArray);
                                fStop = true;
                            }
                            else
                            {
                                (*c_pszData)++;
                            }
                        }
                    }
                }
            }
        }
    }

    return poJsonValue;
}

/********************************************************************************************
 *
 * @class JsonValue
 * @function ParseString
 * @brief Parse JsonValue of type string
 * @param[in] c_pszData pointer to the data pointer
 * @return JsonValue of type string
 *
 ********************************************************************************************/

std::string __stdcall JsonValue::ParseString(
    _in const char ** c_pszData
    )
{
    __DebugFunction();

    std::string strStringValue = "";
    bool fSuccess = false;

    if ('"' == **c_pszData)
    {
        (*c_pszData)++;

        while((0 != **c_pszData) && (true != fSuccess))
        {
            if ('"' != **c_pszData)
            {
                strStringValue += **c_pszData;
            }
            else
            {
                fSuccess = true;
            }

            (*c_pszData)++;
        }
    }

    return strStringValue;
}

/********************************************************************************************
 *
 * @class JsonValue
 * @function GetWholePart
 * @brief Fetch whole part of the number
 * @param[in] c_pszData pointer to the data pointer
 * @return Whole part
 *
 ********************************************************************************************/

double __stdcall JsonValue::GetWholePart(
    _in const char ** c_pszData
    )
{
    __DebugFunction();

    double fl64Number = 0;

    while ((0 != **c_pszData) && ('0' <= **c_pszData) && ('9' >= **c_pszData))
    {
        fl64Number = fl64Number * 10 + (**c_pszData - '0');
        (*c_pszData)++;
    }

    return fl64Number;
}

/********************************************************************************************
 *
 * @class JsonValue
 * @function GetDecimalPart
 * @brief Fetch decimal part of the number
 * @param[in] c_pszData pointer to the data pointer
 * @return Decimal part
 *
 ********************************************************************************************/

double __stdcall JsonValue::GetDecimalPart(
    _in const char ** c_pszData
    )
{
    __DebugFunction();

    double fl64Decimal = 0.0;
    double fl64Factor = 0.1;

    while ((0 != **c_pszData) && ('0' <= **c_pszData) && ('9' >= **c_pszData))
    {
        fl64Decimal = fl64Decimal + ((**c_pszData - '0') * fl64Factor);
        fl64Factor *= 0.1;
        (*c_pszData)++;
    }

    return fl64Decimal;
}

/********************************************************************************************
 *
 * @class JsonValue
 * @function GetObjectElementValue
 * @brief Fetches JsonValue associated with c_pszKey
 * @param[in] c_pszKey Key value
 * @return JsonValue associated with c_pszKey
 *
 ********************************************************************************************/

JsonValue * __thiscall JsonValue::GetObjectElementValue(
    _in const char * c_pszKey
    ) const
{
    __DebugFunction();

	JsonObject::const_iterator it = m_poJsonObject->find(c_pszKey);
	if (it != m_poJsonObject->end())
	{
		return it->second;
	}
	else
	{
		return NULL;
	}
}
