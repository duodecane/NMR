#include "OS.h"
#include <sstream>
std::wstring  OS::ToWstring(std::string a) {
    std::wstring s(a.begin(), a.end());
    return s;
}
std::string  OS::ToString(std::wstring a) {
    std::string s(a.begin(), a.end());
    return s;
   
 
}
bool OS::SaveFolderDialog(std::string& path, std::string defaultFolder) {
    IFileSaveDialog* p_file_save = nullptr;
    bool are_all_operation_success = false;
    while (!are_all_operation_success)
    {
        HRESULT hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL,
            IID_IFileSaveDialog, reinterpret_cast<void**>(&p_file_save));
        if (FAILED(hr))
            break;

     
           // hr = p_file_save->SetOptions(FOS_PICKFOLDERS);
         //   if (FAILED(hr))
         //       break;
         
            COMDLG_FILTERSPEC save_filter[1];
            save_filter[0].pszName = L"All files";
            save_filter[0].pszSpec = L"*.*";
            hr = p_file_save->SetFileTypes(1, save_filter);
            if (FAILED(hr))
                break;
            hr = p_file_save->SetFileTypeIndex(1);
            if (FAILED(hr))
                break;

        if (!defaultFolder.empty())
        {
            hr = p_file_save->SetFileName(ToWstring(defaultFolder).c_str());
            if (FAILED(hr))
                break;
        }

        hr = p_file_save->Show(NULL);
        if (hr == HRESULT_FROM_WIN32(ERROR_CANCELLED)) // No item was selected.
        {
            are_all_operation_success = true;
            break;
        }
        else if (FAILED(hr))
            break;

        IShellItem* p_item;
        hr = p_file_save->GetResult(&p_item);
        if (FAILED(hr))
            break;

        PWSTR item_path;
        hr = p_item->GetDisplayName(SIGDN_FILESYSPATH, &item_path);
        if (FAILED(hr))
            break;
        path = ToString(item_path);
        CoTaskMemFree(item_path);
        p_item->Release();

        are_all_operation_success = true;
    }

    if (p_file_save)
        p_file_save->Release();
    return are_all_operation_success;
}
bool OS::SaveFileDialog(std::string& path, std::string defaultFileName, std::pair<COMDLG_FILTERSPEC*, int>* pFilterInfo )
{
    //fix this must be changed late A!MC bug
    IFileSaveDialog* p_file_save = nullptr;
    bool are_all_operation_success = false;
    while (!are_all_operation_success)
    {
        HRESULT hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL,
            IID_IFileSaveDialog, reinterpret_cast<void**>(&p_file_save));
        if (FAILED(hr))
            break;

        if (!pFilterInfo)
        {
            COMDLG_FILTERSPEC save_filter[1];
            save_filter[0].pszName = L"All files";
            save_filter[0].pszSpec = L"*.*";
            hr = p_file_save->SetFileTypes(1, save_filter);
            if (FAILED(hr))
                break;
            hr = p_file_save->SetFileTypeIndex(1);
            if (FAILED(hr))
                break;
        }
        else
        {
            hr = p_file_save->SetFileTypes(pFilterInfo->second, pFilterInfo->first);
            if (FAILED(hr))
                break;
            hr = p_file_save->SetFileTypeIndex(1);
            if (FAILED(hr))
                break;
        }

        if (!defaultFileName.empty())
        {
            hr = p_file_save->SetFileName(ToWstring(defaultFileName).c_str());
            if (FAILED(hr))
                break;
        }

        hr = p_file_save->Show(NULL);
        if (hr == HRESULT_FROM_WIN32(ERROR_CANCELLED)) // No item was selected.
        {
            are_all_operation_success = true;
            break;
        }
        else if (FAILED(hr))
            break;

        IShellItem* p_item;
        hr = p_file_save->GetResult(&p_item);
        if (FAILED(hr))
            break;

        PWSTR item_path;
        hr = p_item->GetDisplayName(SIGDN_FILESYSPATH, &item_path);
        if (FAILED(hr))
            break;
        path = ToString(item_path);
        CoTaskMemFree(item_path);
        p_item->Release();

        are_all_operation_success = true;
    }

    if (p_file_save)
        p_file_save->Release();
    return are_all_operation_success;
}