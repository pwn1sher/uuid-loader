#include "headers.h"


PPEB get_peb()
{
#ifdef _M_IX86 
	return (PPEB)__readfsdword(0x30);
#elif defined(_M_AMD64)
	return (PPEB)__readgsqword(0x60);
#endif
}


PIMAGE_DATA_DIRECTORY get_data_dir(LPBYTE lpBaseAddress, WORD wIndex)
{
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)lpBaseAddress;
	PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)(lpBaseAddress + pDosHeader->e_lfanew);
	return &pNtHeaders->OptionalHeader.DataDirectory[wIndex];
}

LPBYTE find_module(DWORD dwModuleHash)
{
	PPEB pPeb = get_peb();
	LIST_ENTRY* pListEntry = pPeb->Ldr->InMemoryOrderModuleList.Flink;

	do
	{
		LDR_DATA_TABLE_ENTRY* pLdrDataTableEntry = (PLDR_DATA_TABLE_ENTRY)pListEntry;

		UNICODE_STRING dllName = pLdrDataTableEntry->FullDllName;

		if (compute_hash(dllName.Buffer, dllName.Length) == dwModuleHash)
			return (LPBYTE)pLdrDataTableEntry->Reserved2[0];

		pListEntry = pListEntry->Flink;

	} while (pListEntry != pPeb->Ldr->InMemoryOrderModuleList.Flink);

	return 0;
}

FARPROC find_api(DWORD dwModuleHash, DWORD dwProcHash)
{
	LPBYTE lpBaseAddress = find_module(dwModuleHash);
	PIMAGE_DATA_DIRECTORY pDataDir = get_data_dir(lpBaseAddress, IMAGE_DIRECTORY_ENTRY_EXPORT);
	PIMAGE_EXPORT_DIRECTORY pExportDir = (PIMAGE_EXPORT_DIRECTORY)(lpBaseAddress + pDataDir->VirtualAddress);
	LPDWORD pNames = (LPDWORD)(lpBaseAddress + pExportDir->AddressOfNames);
	LPWORD pOrdinals = (LPWORD)(lpBaseAddress + pExportDir->AddressOfNameOrdinals);

	for (SIZE_T i = 0; i < pExportDir->NumberOfNames; ++i)
	{
		char* szName = (char*)lpBaseAddress + (DWORD_PTR)pNames[i];

		if (compute_hash(szName, 0) == dwProcHash)
			return (FARPROC)(lpBaseAddress + ((DWORD*)(lpBaseAddress + pExportDir->AddressOfFunctions))[pOrdinals[i]]);
	}

	return NULL;
}
