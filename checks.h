// add checks to check Domain Name before Spawning

#include "headers.h"
#include <winbase.h>

#define UNLEN 256;

typedef BOOL(WINAPI* _GetComputerNameExA)(
    COMPUTER_NAME_FORMAT NameType,
    LPSTR                lpBuffer,
    LPDWORD              nSize
    );

int DomainCheck(){

    // To Get AD Domain Name
    COMPUTER_NAME_FORMAT name = ComputerNamePhysicalDnsDomain;

    HMODULE Kernel32 = GetModuleHandleA("Kernel32.dll");
   
    _GetComputerNameExA GetComputerNameExA = (_GetComputerNameExA)GetProcAddress(Kernel32, "GetComputerNameExA");
    if (GetComputerNameExA == NULL) {
        return -1;
    }

    // Init some important variables
    LPSTR lpBuffer[MAX_PATH];
    DWORD dwLength = UNLEN + 1;
    ZeroMemory(lpBuffer, MAX_PATH);

    // Retrieve name
    BOOL valid = GetComputerNameExA(name, (LPSTR)lpBuffer, &dwLength);
    
    if (!valid) {
        printf("Failed to Get Domain Name\n");
    }

    printf("Domain is: %s\n", (LPSTR)lpBuffer);

    char domnam[256] = "radiantcorp.local";

    if (strcmp(domnam, (LPSTR)lpBuffer) == 0) {
        // detonate only if domain name matched
        printf("\nmatch\n");
    }

    return 0;
}


int checkinternet() {

    // resolve github 

 // dont wanna get those windns headers :(

    return 0;
}