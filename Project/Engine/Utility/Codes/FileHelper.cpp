#include "FileHelper.h"
#include <shobjidl.h> 

std::filesystem::path 
Engine::FileHelper::OpenDialogBox() 
{
	HRESULT Result = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
		COINIT_DISABLE_OLE1DDE);
	std::filesystem::path FsFilePath;
	if (SUCCEEDED(Result))
	{
		IFileOpenDialog* FileOpenDiaLog;

		// Create the FileOpenDialog object.
		Result = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
			IID_IFileOpenDialog, reinterpret_cast<void**>(&FileOpenDiaLog));

		if (SUCCEEDED(Result))
		{
			// Show the Open dialog box.
			Result = FileOpenDiaLog->Show(NULL);

			// Get the file name from the dialog box.
			if (SUCCEEDED(Result))
			{
				IShellItem* pItem;
				Result = FileOpenDiaLog->GetResult(&pItem);
				if (SUCCEEDED(Result))
				{
					PWSTR FilePath;
					Result = pItem->GetDisplayName(SIGDN_FILESYSPATH, &FilePath);

					// Display the file name to the user.
					if (SUCCEEDED(Result))
					{
						FsFilePath = FilePath;
						CoTaskMemFree(FilePath);
					}
					pItem->Release();
				}
			}
			FileOpenDiaLog->Release();
		}
		CoUninitialize();
	}
	return 	FsFilePath;

};
