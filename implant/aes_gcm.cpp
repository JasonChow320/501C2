#include "aes_gcm.h"

AESGCM:: ~AESGCM(){
    Cleanup();
}

// Freebie: initialize AES class
AESGCM::AESGCM( BYTE key[AES_256_KEY_SIZE]){
    hAlg = 0;
    hKey = NULL;

    // create a handle to an AES-GCM provider
    // https://docs.microsoft.com/en-us/windows/win32/api/bcrypt/nf-bcrypt-bcryptopenalgorithmprovider
    nStatus = ::BCryptOpenAlgorithmProvider(
        &hAlg, 
        BCRYPT_AES_ALGORITHM, 
        NULL, 
        0);
    if (! NT_SUCCESS(nStatus))
    {
        wprintf(L"**** Error 0x%x returned by BCryptOpenAlgorithmProvider\n", nStatus);
        Cleanup();
        return;
    }
    if (!hAlg){
        wprintf(L"Invalid handle!\n");
    }

    //hAlg should be a valid handle for BCRYPT_ALG_HANDLE now ...

    nStatus = ::BCryptSetProperty(
        hAlg, 
        BCRYPT_CHAINING_MODE, 
        (BYTE*)BCRYPT_CHAIN_MODE_GCM, 
        sizeof(BCRYPT_CHAIN_MODE_GCM), 
        0);
    if (!NT_SUCCESS(nStatus)){
         wprintf(L"**** Error 0x%x returned by BCryptGetProperty ><\n", nStatus);
         Cleanup();
         return;
    }
    //        bcryptResult = BCryptGenerateSymmetricKey(algHandle, &keyHandle, 0, 0, (PUCHAR)&key[0], key.size(), 0);
    // https://docs.microsoft.com/en-us/windows/win32/api/bcrypt/nf-bcrypt-bcryptgeneratesymmetrickey
    nStatus = ::BCryptGenerateSymmetricKey(
        hAlg, 
        &hKey, 
        NULL, 
        0, 
        key, 
        AES_256_KEY_SIZE, 
        0);
    if (!NT_SUCCESS(nStatus)){
        wprintf(L"**** Error 0x%x returned by BCryptGenerateSymmetricKey\n", nStatus);
        Cleanup();
        return;
    }

    // hKey should be a valid handle for BCRYPT_KEY_HANDLE now ...
    // from aes_gcm.h => BCRYPT_AUTH_TAG_LENGTHS_STRUCT authTagLengths;
    DWORD cbResult = 0;
     nStatus = ::BCryptGetProperty(
         hAlg, 
         BCRYPT_AUTH_TAG_LENGTH, 
         (BYTE*)&authTagLengths, 
         sizeof(authTagLengths), 
         &cbResult, 
         0);
   if (!NT_SUCCESS(nStatus)){
       wprintf(L"**** Error 0x%x returned by BCryptGetProperty when calculating auth tag len\n", nStatus);
   }
   // after initialization, hAlg, hKey & authTagLengths should be defined
   
}

DWORD AESGCM::getBlockSize(){
    DWORD sizeOfIV, cbResult = 0;
    nStatus = ::BCryptGetProperty(
        hAlg,
        BCRYPT_BLOCK_LENGTH,
        (BYTE*)&sizeOfIV,
        sizeof(sizeOfIV),
        &cbResult,
        0
    );
    return sizeOfIV;
}

int AESGCM::VerifyTag(BYTE* nonce, size_t nonceLen, BYTE* macTag, size_t macTagLen){
    if(!hKey){
        return 0;
    }

    // we need to initalize the BCRYPT_AUTHENTICATED_CIPHER_MODE_INFO structure variable
    // https://docs.microsoft.com/en-us/windows/win32/api/bcrypt/ns-bcrypt-bcrypt_authenticated_cipher_mode_info
    // https://docs.microsoft.com/en-us/windows/win32/api/bcrypt/nf-bcrypt-bcrypt_init_auth_mode_info
    BCRYPT_AUTHENTICATED_CIPHER_MODE_INFO paddingInfo;
    ::BCRYPT_INIT_AUTH_MODE_INFO(paddingInfo);
    paddingInfo.pbNonce = nonce;
    paddingInfo.cbNonce = nonceLen;
    paddingInfo.pbAuthData = NULL;
    paddingInfo.cbAuthData = 0;
    paddingInfo.pbTag = macTag;
    paddingInfo.cbTag = macTagLen;
    paddingInfo.pbMacContext = NULL;
    paddingInfo.cbMacContext = 0;
    paddingInfo.cbAAD = 0;
    paddingInfo.cbData = 0;
    paddingInfo.dwFlags = 0;

    //verify padding
    ULONG byteCopiedToBuffer = 0;
    nStatus = ::BCryptDecrypt(
        hKey,
        NULL,
        0,
        &paddingInfo,
        NULL,
        nonceLen,
        NULL,
        0, 
        &byteCopiedToBuffer,
        0
    );
    if (!NT_SUCCESS(nStatus)){
        printf("Something terrible has happened with the padding info:( 0x%x\n", nStatus);
        return 1;
    }
    return byteCopiedToBuffer;
}
/*
https://docs.microsoft.com/en-us/windows/win32/api/bcrypt/nf-bcrypt-bcryptdecrypt
*/
int AESGCM::Decrypt(BYTE* nonce, size_t nonceLen, BYTE* data, size_t dataLen, BYTE* macTag, size_t macTagLen){
    if(!hKey){
        return 1;
    }

    // we need to initalize the BCRYPT_AUTHENTICATED_CIPHER_MODE_INFO structure variable
    // https://docs.microsoft.com/en-us/windows/win32/api/bcrypt/ns-bcrypt-bcrypt_authenticated_cipher_mode_info
    // https://docs.microsoft.com/en-us/windows/win32/api/bcrypt/nf-bcrypt-bcrypt_init_auth_mode_info
    BCRYPT_AUTHENTICATED_CIPHER_MODE_INFO paddingInfo;
    ::BCRYPT_INIT_AUTH_MODE_INFO(paddingInfo);
    paddingInfo.pbNonce = nonce;
    paddingInfo.cbNonce = nonceLen;
    paddingInfo.pbAuthData = NULL;
    paddingInfo.cbAuthData = 0;
    paddingInfo.pbTag = macTag;
    paddingInfo.cbTag = macTagLen;
    paddingInfo.pbMacContext = NULL;
    paddingInfo.cbMacContext = 0;
    paddingInfo.cbAAD = 0;
    paddingInfo.cbData = 0;
    paddingInfo.dwFlags = 0;

    ULONG byteCopiedToBuffer = 0;
    nStatus = ::BCryptDecrypt(
        hKey,
        data,
        dataLen,
        &paddingInfo,
        nonce,
        nonceLen,
        NULL,
        dataLen, 
        &byteCopiedToBuffer,
        0
    );
    if (!NT_SUCCESS(nStatus)){
        printf("Something terrible has happened :(\n");
    }

    BYTE* plaintextbuffer = (BYTE*)::HeapAlloc(GetProcessHeap(), 0, byteCopiedToBuffer);
    if(plaintextbuffer == NULL){
        wprintf(L"*** Failed to allocate memory for plaintext buffer\n");
    }

    nStatus = ::BCryptDecrypt(
        hKey,
        data,
        dataLen,
        &paddingInfo,
        nonce,
        nonceLen,
        (PUCHAR)plaintextbuffer,
        dataLen, 
        &byteCopiedToBuffer,
        0
    );
    if (!NT_SUCCESS(nStatus)){
        if(STATUS_MISMATCHTAG(nStatus)){
            //wprintf(L"**** Error, mac authenication failed (tag mismatch)\n");
        }
        //wprintf(L"**** Error 0x%x returned by BCryptEncrypt when calculating ciphertext size\n", nStatus);
        ::HeapFree(GetProcessHeap(), 0, (LPVOID)plaintext);
        plaintext = NULL;
        return 1;
    }else{
        plaintext = plaintextbuffer;
        ptBufferSize = byteCopiedToBuffer;
    } 
    return 0;
}

/*
https://docs.microsoft.com/en-us/windows/win32/api/bcrypt/nf-bcrypt-bcryptencrypt
*/
void AESGCM::Encrypt(BYTE* nonce, size_t nonceLen, BYTE* data, size_t dataLen){
    // check if we have valid handles before continuing, initalization should've initialzed them
    if(!hKey){
        return;
    }

    // we need to initalize the BCRYPT_AUTHENTICATED_CIPHER_MODE_INFO structure variable
    // https://docs.microsoft.com/en-us/windows/win32/api/bcrypt/ns-bcrypt-bcrypt_authenticated_cipher_mode_info
    // https://docs.microsoft.com/en-us/windows/win32/api/bcrypt/nf-bcrypt-bcrypt_init_auth_mode_info
    BCRYPT_AUTHENTICATED_CIPHER_MODE_INFO paddingInfo;
    ::BCRYPT_INIT_AUTH_MODE_INFO(paddingInfo);
    paddingInfo.pbNonce = nonce;
    paddingInfo.cbNonce = nonceLen;
    BYTE* tagBuffer = (BYTE*)::HeapAlloc(GetProcessHeap(), 0, authTagLengths.dwMinLength);
    if(tagBuffer == NULL){
        wprintf(L"*** Failed to allocate memory for tag buffer\n");
    }
    paddingInfo.pbTag = tagBuffer;
    paddingInfo.cbTag = authTagLengths.dwMinLength;

    // dynamically allocate memory in the heap using heapalloc, Cleanup() uses heap free()
    // https://docs.microsoft.com/en-us/windows/win32/api/heapapi/nf-heapapi-heapalloc
    auto ciphertextBuffer = (BYTE*)::HeapAlloc(GetProcessHeap(), 0, dataLen);
    if(ciphertextBuffer == NULL){
        wprintf(L"*** Failed to allocate memory for ciphertext buffer\n");
    }
    ULONG byteCopiedToBuffer = 0;
    nStatus = ::BCryptEncrypt(
        hKey,
        data,
        dataLen,
        &paddingInfo,
        nonce,
        nonceLen,
        (PUCHAR)ciphertextBuffer,
        dataLen, 
        &byteCopiedToBuffer,
        0
    );
    if (!NT_SUCCESS(nStatus)){
        wprintf(L"**** Error 0x%x returned by BCryptEncrypt when calculating ciphertext size\n", nStatus);
    }

    //save our results into struct's public variables 
    tag = tagBuffer;
    ciphertext = ciphertextBuffer;
    for(int i = 0; i < dataLen; i++){
        wprintf(L"%2X ", ciphertextBuffer[i]);
    }
    wprintf(L"\nNow tag\n");
    for(int i = 0; i < authTagLengths.dwMinLength; i++){
        wprintf(L"%2X ", tagBuffer[i]);
    }
}

void AESGCM::Cleanup(){
    if(hAlg){
        ::BCryptCloseAlgorithmProvider(hAlg,0);
        hAlg = NULL;
    }
    if(hKey){
        ::BCryptDestroyKey(hKey);
        hKey = NULL;
    }
    if(tag){
          ::HeapFree(GetProcessHeap(), 0, (LPVOID)tag);
          tag = NULL;
    }
    if(ciphertext){
        ::HeapFree(GetProcessHeap(), 0, (LPVOID)ciphertext);
        ciphertext = NULL;
    }
    if(plaintext){
        ::HeapFree(GetProcessHeap(), 0, (LPVOID)plaintext);
        plaintext = NULL;
    }
}
