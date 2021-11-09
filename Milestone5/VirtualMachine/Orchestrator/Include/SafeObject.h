#pragma once

#include <vector>
#include <string>
#include "StructuredBuffer.h"

class SafeObject
{
    private:
        std::string m_strTitle; 
        int m_nNumberOfInputArgs;
        int m_nNumberOfOutputArgs;
        std::vector<std::string> m_stlInputArgs;
        std::vector<std::string> m_stlInputDescription;
        std::vector<std::string> m_stlOutputArgs;
        std::vector<std::string> m_stlOutputConfidential;
        std::string m_strScript;
        std::string m_strSafeObjectID;
        std::string m_strDescription;

    public:
        SafeObject
        (
            StructuredBuffer& oBuffer
        );

        const std::string& GetSafeObjectID(void);
        const std::string& GetTitle(void);
        const std::string& GetDescription(void);
        const int& GetInputNumber(void);
        const int& GetOutputNumber(void);
        const std::vector<std::string>& GetInput(void);
        const std::vector<std::string>& GetInputDescription(void);
        const std::vector<std::string>& GetOutputConfidential(void);
        const std::vector<std::string>& GetOutput(void);
        const std::string& GetScript(void);
};
