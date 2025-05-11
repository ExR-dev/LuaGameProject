#include "WindowsWrapped.h"
#include <windows.h>
#include <shobjidl.h>
#include <filesystem>

#ifdef LEAK_DETECTION
#define new			DEBUG_NEW
#endif

void Windows::SleepW(int milliseconds)
{
	Sleep(milliseconds);
}

bool Windows::IsDebuggerPresentW()
{
	return IsDebuggerPresent() != 0;
}

int Windows::IDABORTW()
{
	return IDABORT;
}
int Windows::IDRETRYW()
{
	return IDRETRY;
}

int Windows::MessageBoxAW(const char *msg, const char *caption)
{
	return MessageBoxA(0, msg, caption, MB_ABORTRETRYIGNORE | MB_ICONERROR);
}

Windows::HWNDW Windows::GetConsoleWindowW()
{
	Windows::HWNDW hwnd(GetConsoleWindow());
	return hwnd;
}

bool Windows::OpenFileCatalog(std::string &fileName, std::string &filePath, const std::string &startDir)
{
	//  CREATE FILE OBJECT INSTANCE
    HRESULT f_SysHr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(f_SysHr))
        return FALSE;

    // CREATE FileOpenDialog OBJECT
    IFileOpenDialog *f_FileSystem = nullptr;
    f_SysHr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&f_FileSystem));
    if (FAILED(f_SysHr)) {
        CoUninitialize();
        return FALSE;
    }

    // SET THE INITIAL DIRECTORY IF SPECIFIED
    if (!startDir.empty()) {
        std::string absolutePath = std::filesystem::canonical(startDir).string();

        // Convert the startDir string to wide string
        std::wstring wideStartDir(absolutePath.begin(), absolutePath.end());

        // Create a shell item from the path
        IShellItem *psiFolder;
        f_SysHr = SHCreateItemFromParsingName(wideStartDir.c_str(), NULL, IID_PPV_ARGS(&psiFolder));

        if (SUCCEEDED(f_SysHr)) {
            // Set the default folder
            f_FileSystem->SetFolder(psiFolder);
            psiFolder->Release();
        }
        // Continue even if setting folder fails
    }

    //  SHOW OPEN FILE DIALOG WINDOW
    f_SysHr = f_FileSystem->Show(NULL);
    if (FAILED(f_SysHr)) {
        f_FileSystem->Release();
        CoUninitialize();
        return FALSE;
    }
    //  RETRIEVE FILE NAME FROM THE SELECTED ITEM
    IShellItem* f_Files;
    f_SysHr = f_FileSystem->GetResult(&f_Files);
    if (FAILED(f_SysHr)) {
        f_FileSystem->Release();
        CoUninitialize();
        return FALSE;
    }

    //  STORE AND CONVERT THE FILE NAME
    PWSTR f_Path;
    f_SysHr = f_Files->GetDisplayName(SIGDN_FILESYSPATH, &f_Path);
    if (FAILED(f_SysHr)) {
        f_Files->Release();
        f_FileSystem->Release();
        CoUninitialize();
        return FALSE;
    }

    //  FORMAT AND STORE THE FILE PATH
    std::wstring path(f_Path);
    std::string c(path.begin(), path.end());
    filePath = c;

    //  FORMAT STRING FOR EXECUTABLE NAME
    const size_t slash = filePath.find_last_of("/\\");
    fileName = filePath.substr(slash + 1);

    //  SUCCESS, CLEAN UP
    CoTaskMemFree(f_Path);
    f_Files->Release();
    f_FileSystem->Release();
    CoUninitialize();

    return TRUE;
}





