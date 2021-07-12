#pragma once

#include <vector>
#include <string>

class FunctionNode
{
    private:
        int m_nNumberOfInputArgs;
        int m_nNumberOfOutputArgs;
        std::vector<std::string> m_stlInputArgs;
        std::vector<std::string> m_stlOutputArgs;
        std::string m_strScript;
        std::string m_strFNID;

    public:
        FunctionNode
        (
            int nInputNumber,
            int nOutputNumber,
            std::string& strPath
        );

        const std::string& GetFNID(void);
        const int& GetInputNumber(void);
        const int& GetOutputNumber(void);
        const std::vector<std::string>& GetInput(void);
        const std::vector<std::string>& GetOutput(void);
        const std::string& GetScript(void);
};
