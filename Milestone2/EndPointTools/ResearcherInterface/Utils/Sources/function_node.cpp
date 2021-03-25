#include "function_node.h"
#include <Guid.h>
#include <fstream>
#include <sstream>

FunctionNode::FunctionNode
(
    int nInputNumber,
    int nOutputNumber,
    int nConfidentialInputNumber,
    int nConfidentialOutputNumber,
    std::string strPath
)
{
    Guid oFNGuid;
    m_nNumberOfInputArgs = nInputNumber;
    m_nNumberOfOutputArgs = nOutputNumber;
    m_nNumberOfConfidentialInputArgs = nConfidentialInputNumber;
    m_nNumberOfConfidentialOutputArgs = nConfidentialOutputNumber;
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
    for(int i=0;i<m_nNumberOfConfidentialInputArgs;i++)
    {
        Guid oConfidentialInputGuid;
        m_stlConfidentialInputArgs.push_back(oConfidentialInputGuid.ToString(eRaw));
    }
    for(int i=0;i<m_nNumberOfConfidentialOutputArgs;i++)
    {
        Guid oConfidentialOutputGuid;
        m_stlConfidentialOutputArgs.push_back(oConfidentialOutputGuid.ToString(eRaw));
    }
    
    std::ifstream stlInputStream;
    std::stringstream stlStringBufferStream;
    
    stlInputStream.open(strPath);
    stlStringBufferStream<<stlInputStream.rdbuf();
    stlInputStream.close();
    
    m_strScript=stlStringBufferStream.str();
}

FunctionNode::FunctionNode
(
    std::vector<std::string> stlInputArgs,
    std::vector<std::string> stlOutputArgs,
    std::vector<std::string> stlConfidentialInputArgs,
    std::vector<std::string> stlConfidentialOutputArgs,
    std::string strFNID
):
    m_nNumberOfInputArgs(stlInputArgs.size()),
    m_nNumberOfOutputArgs(stlOutputArgs.size()),
    m_nNumberOfConfidentialInputArgs(stlConfidentialInputArgs.size()),
    m_nNumberOfConfidentialOutputArgs(stlConfidentialOutputArgs.size()),
    m_stlInputArgs(std::move(stlInputArgs)),
    m_stlOutputArgs(std::move(stlOutputArgs)),
    m_stlConfidentialInputArgs(std::move(stlConfidentialInputArgs)),
    m_stlConfidentialOutputArgs(std::move(stlConfidentialOutputArgs)),
    m_strFNID(strFNID)
{

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

const int& FunctionNode::GetConfidentialInputNumber(void)
{
    return m_nNumberOfConfidentialInputArgs;
}

const int& FunctionNode::GetConfidentialOutputNumber(void)
{
    return m_nNumberOfConfidentialOutputArgs;
}

const std::vector<std::string>& FunctionNode::GetInput(void)
{
    return m_stlInputArgs;
}

const std::vector<std::string>& FunctionNode::GetOutput(void)
{
    return m_stlOutputArgs;
}

const std::vector<std::string>& FunctionNode::GetConfidentialInput(void)
{
    return m_stlConfidentialInputArgs;
}

const std::vector<std::string>& FunctionNode::GetConfidentialOutput(void)
{
    return m_stlConfidentialOutputArgs;
}

const std::string& FunctionNode::GetScript(void)
{
    return m_strScript;
}
