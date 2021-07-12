#include "function_node.h"
#include <Guid.h>
#include <fstream>
#include <sstream>

FunctionNode::FunctionNode
(
    int nInputNumber,
    int nOutputNumber,
    std::string& strPath
)
{
    Guid oFNGuid;
    m_nNumberOfInputArgs = nInputNumber;
    m_nNumberOfOutputArgs = nOutputNumber;
    m_strFNID = oFNGuid.ToString(eRaw);
    
    for(int i=0;i<m_nNumberOfInputArgs;i++)
    {
        Guid oInGuid;
        m_stlInputArgs.push_back(oInGuid.ToString(eRaw)); 
    }
    for(int i=0;i<m_nNumberOfOutputArgs;i++)
    {
        Guid oOutGuid;
        m_stlOutputArgs.push_back(oOutGuid.ToString(eRaw));
    }
    
    std::ifstream stlInputStream;
    std::stringstream stlStringBufferStream;
    
    stlInputStream.open(strPath);
    stlStringBufferStream<<stlInputStream.rdbuf();
    stlInputStream.close();
    
    m_strScript=stlStringBufferStream.str();
}

const std::string& FunctionNode::GetFNID(void)
{
    return m_strFNID;
}

const int& FunctionNode::GetInputNumber(void)
{
    return m_nNumberOfInputArgs;
}

const int& FunctionNode::GetOutputNumber(void)
{
    return m_nNumberOfOutputArgs;
}

const std::vector<std::string>& FunctionNode::GetInput(void)
{
    return m_stlInputArgs;
}

const std::vector<std::string>& FunctionNode::GetOutput(void)
{
    return m_stlOutputArgs;
}

const std::string& FunctionNode::GetScript(void)
{
    return m_strScript;
}
