#include <vector>
#include <string>

class FunctionNode
{
    private:
        int m_nNumberOfInputArgs;
        int m_nNumberOfOutputArgs;
        int m_nNumberOfConfidentialInputArgs;
        int m_nNumberOfConfidentialOutputArgs;
        std::vector<std::string> m_stlInputArgs;
        std::vector<std::string> m_stlOutputArgs;
        std::vector<std::string> m_stlConfidentialInputArgs;
        std::vector<std::string> m_stlConfidentialOutputArgs;
        std::string m_strScript;
        std::string m_strFNID;

    public:
        FunctionNode
        (
            int nInputNumber,
            int nOutputNumber,
            int nConfidentialInputNumber,
            int nConfidentialOutputNumber,
            std::string strPath
        );
        FunctionNode
        (
            std::vector<std::string> stlInputArgs,
            std::vector<std::string> stlOutputArgs,
            std::vector<std::string> stlConfidentialInputArgs,
            std::vector<std::string> stlConfidentialOutputArgs,
            std::string strFNID
        );
        const std::string& GetFNID(void);
        const int& GetInputNumber(void);
        const int& GetOutputNumber(void);
        const int& GetConfidentialInputNumber(void);
        const int& GetConfidentialOutputNumber(void);
        const std::vector<std::string>& GetInput(void);
        const std::vector<std::string>& GetOutput(void);
        const std::vector<std::string>& GetConfidentialInput(void);
        const std::vector<std::string>& GetConfidentialOutput(void);
        const std::string& GetScript(void);
};
