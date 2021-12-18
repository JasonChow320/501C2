#include <windows.h>
#include <string>
#include <iostream>
#include <winhttp.h>

class HTTP{
    private:
        HINTERNET hSession = NULL, hConnect = NULL, hRequest = NULL;
    public:
        HTTP();
        ~HTTP();
        int checkConnection();
        DWORD connectToServer(std::wstring fqdn, int port);
        std::string makeHttpRequest(std::wstring method, std::wstring uri, bool useTLS, DWORD* res);
        LPCWSTR additionalHeader = NULL;
        DWORD header_size = 0;  
        const char* requestData = NULL;
        DWORD data_size = 0;   
};

