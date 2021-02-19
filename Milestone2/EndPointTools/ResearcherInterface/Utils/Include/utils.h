#pragma once

#include "CoreTypes.h"
#include "StructuredBuffer.h"
#include <vector>
#include <string>

/********************************************************************************************/

typedef enum
{
    eQUIT = 0,
    eRUN,
    eCONNECT,
    eCHECK,
    eINSPECT,
    eGETTABLE,
    ePUSHDATA,
    ePULLDATA,
    eDELETEDATA,
    ePUSHFN,
}  RequestType; 

template <class T>
void __cdecl BufToVec
(
    _in StructuredBuffer& oBuffer,
    _in T& stlVec
)
{
    int nLen = oBuffer.GetInt16("size");
    for(int i=0; i<nLen; i++)
    {
        if constexpr(std::is_same<T, std::vector<std::string>>::value)
            stlVec.push_back(oBuffer.GetString(std::to_string(i).c_str()));
        else if constexpr(std::is_same<T, std::vector<std::vector<Byte>>>::value)
            stlVec.push_back(oBuffer.GetBuffer(std::to_string(i).c_str()));
    }    
}

template <class T>
void __cdecl VecToBuf
(
    _in T& stlArgIDs,
    StructuredBuffer& oArgs
)
{
    oArgs.PutInt16("size", stlArgIDs.size());
    for(size_t i=0;i<stlArgIDs.size();i++)
    {
        if constexpr(std::is_same<T, std::vector<std::string>>::value)
            oArgs.PutString(std::to_string(i).c_str(), stlArgIDs[i]);
        else if constexpr(std::is_same<T, std::vector<std::vector<Byte>>>::value)
            oArgs.PutBuffer(std::to_string(i).c_str(), stlArgIDs[i]);
    }
}
