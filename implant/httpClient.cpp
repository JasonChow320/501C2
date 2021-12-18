#include "httpClient.h"

using namespace std;
//constructor
HTTP::HTTP(){
    LPCWSTR agent = L"agent1337";
    hSession = WinHttpOpen( agent, WINHTTP_ACCESS_TYPE_NO_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if(!hSession){
        wprintf(L"Failed to open session :(\n");
    }
}

HTTP::~HTTP(){
    // close all open handles
    if(hSession){
        WinHttpCloseHandle(hSession);
    }
    if(hConnect){
        WinHttpCloseHandle(hConnect);
    }
    if(hRequest){
        WinHttpCloseHandle(hRequest);
    }
}

//uses WinHttpConnect
DWORD HTTP::connectToServer(wstring fqdn, int port){
    if(hSession){
        hConnect = WinHttpConnect( hSession, fqdn.data(), port, 0);
        if(!hConnect){
            wprintf(L"Failed to connect to server\n");
            return GetLastError();
        }
    }else{
        wprintf(L"Failed to open session :(\n");
    }
    return 0;
}

int HTTP::checkConnection(){
    if(hConnect && hSession){
        return 1;
    }
    return 0;
}

string HTTP::makeHttpRequest(wstring method, wstring uri, bool useTLS, DWORD* res){
    LPWSTR buffer;
    string result;
    BOOL bResults = false;
    DWORD dwSize = 0, dwDownloaded = 0;
    LPSTR pszOutBuffer;
    
    //if connected to server, set flags and open request
    if(hConnect){
        DWORD tls = 0;
        if(useTLS){
            tls = WINHTTP_FLAG_SECURE;
        }
        hRequest = WinHttpOpenRequest( hConnect, method.data(), uri.data(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, tls);
    }else{
        wprintf(L"Have to connect to server first\n"); 
        *res = 1;
        return "nope:D";
    }

    if(hRequest){
        if(useTLS){
            //disable certificate verification 
            DWORD secure_protocols{ SECURITY_FLAG_IGNORE_UNKNOWN_CA | SECURITY_FLAG_IGNORE_CERT_DATE_INVALID | 
            SECURITY_FLAG_IGNORE_CERT_CN_INVALID | SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE };
            if(!WinHttpSetOption( hRequest, WINHTTP_OPTION_SECURITY_FLAGS, &secure_protocols, 4)){
                wcout << GetLastError() << endl;
            }
        }

        //send the request
        if(additionalHeader == NULL || header_size == 0){
            if(requestData == NULL || data_size == 0){
                bResults = WinHttpSendRequest( hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA,0,0,0);
            }else{
                bResults = WinHttpSendRequest( hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, (LPVOID)requestData,data_size,data_size,0);
            }
        }else{
            if(requestData == NULL || data_size == 0){
                bResults = WinHttpSendRequest( hRequest, additionalHeader, header_size, WINHTTP_NO_REQUEST_DATA,0,0,0);
            }else{
                bResults = WinHttpSendRequest( hRequest, additionalHeader, header_size, (LPVOID)requestData,data_size,data_size,0);
            }
        }

        //check request
        if (bResults){
            bResults = WinHttpReceiveResponse( hRequest, NULL);
        }else{
            wprintf(L"Send error?? 0x%x\n", GetLastError());
            *res = 1;
        }
    }else{
        wprintf(L"Couldn't Open request\n");
        *res = 1;
        return "Couldn't open request";
    }

    // Keep checking for data until there is nothing left.
    if (bResults){
        do {
            // Check for available data.
            dwSize = 0;
            if (!WinHttpQueryDataAvailable( hRequest, &dwSize)) {
                printf( "Error %u in WinHttpQueryDataAvailable.\n",
                        GetLastError());
                break;
            }
            // No more available data.
            if (!dwSize) break;

            // Allocate space for the buffer.
            // Not the most efficient to dynamically allocate memory every round, but converting to wchar_t is easier this way
            pszOutBuffer = new char[dwSize+1];
            buffer = new wchar_t[dwSize+1];
            if (!pszOutBuffer || !buffer){
                printf("Out of memory\n");
                *res = 1;
                break;
            }
            
            // Read the Data.
            ZeroMemory(pszOutBuffer, dwSize+1);
            if (!WinHttpReadData(hRequest, (LPVOID)pszOutBuffer, 
                                  dwSize, &dwDownloaded)){                                  
                printf( "Error %u in WinHttpReadData.\n", GetLastError());
            }else{
                // converting char to wchar_t
                mbstowcs(buffer, pszOutBuffer, dwSize+1);
                result += pszOutBuffer;
            }
        
            // Free the memory allocated to the buffer.
            delete [] pszOutBuffer;
            delete [] buffer;

            // This condition should never be reached since WinHttpQueryDataAvailable
            // reported that there are bits to read.
            if (!dwDownloaded)
                break;
                
        } while (dwSize > 0);
    }
    *res = 0;
    return result;
}
