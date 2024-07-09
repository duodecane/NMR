#pragma once
#include <string>
#include <Windows.h>
#include <Shlwapi.h>
#include <ShObjIdl.h>
namespace OS {
    bool SaveFileDialog(std::string& path, std::string defaultFileName = "", std::pair<COMDLG_FILTERSPEC*, int>* pFilterInfo = nullptr);
    bool SaveFolderDialog(std::string& path, std::string defaultFolder = "");
    std::wstring ToWstring(std::string a);
    std::string ToString(std::wstring a);
};
