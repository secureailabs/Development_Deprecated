#include "SafeObject.h"
#include <Guid.h>
#include <fstream>
#include <sstream>

SafeObject::SafeObject
(
    StructuredBuffer& oBuffer
)
{
    m_strTitle = oBuffer.GetString("Title");
    m_strSafeObjectID = oBuffer.GetString("Uuid");
    m_strScript = oBuffer.GetString("Payload");
    m_strDescription = oBuffer.GetString("Description");
    
    StructuredBuffer oInput = oBuffer.GetStructuredBuffer("InputParameters");
    m_nNumberOfInputArgs = oInput.GetNamesOfElements().size();
    for(int i = 0; i<m_nNumberOfInputArgs; i++)
    {
        StructuredBuffer oInputElement = oInput.GetStructuredBuffer(std::to_string(i).c_str());
        m_stlInputArgs.push_back(oInputElement.GetString("Uuid"));
        //m_stlInputDescription.push_back(oInputElement.GetString("Description"));
    }

    StructuredBuffer oOutput = oBuffer.GetStructuredBuffer("OutputParameters");
    m_nNumberOfOutputArgs = oOutput.GetNamesOfElements().size();
    for(int i = 0; i<m_nNumberOfOutputArgs; i++)
    {
        StructuredBuffer oOutputElement = oOutput.GetStructuredBuffer(std::to_string(i).c_str());
        m_stlOutputArgs.push_back(oOutputElement.GetString("Uuid"));
        //m_stlInputDescription.push_back(oInputElement.GetString("Description"));
    }
}

const std::string& SafeObject::GetSafeObjectID(void)
{
    return m_strSafeObjectID;
}

const std::string& SafeObject::GetTitle(void)
{
    return m_strTitle;
}

const std::string& SafeObject::GetDescription(void)
{
    return m_strDescription;
}

const int& SafeObject::GetInputNumber(void)
{
    return m_nNumberOfInputArgs;
}

const int& SafeObject::GetOutputNumber(void)
{
    return m_nNumberOfOutputArgs;
}

const std::vector<std::string>& SafeObject::GetInput(void)
{
    return m_stlInputArgs;
}

const std::vector<std::string>& SafeObject::GetInputDescription(void)
{
    return m_stlInputDescription;
}

const std::vector<std::string>& SafeObject::GetOutput(void)
{
    return m_stlOutputArgs;
}

const std::string& SafeObject::GetScript(void)
{
    return m_strScript;
}
