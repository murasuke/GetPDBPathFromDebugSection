#include <windows.h>
#include <dbghelp.h>
#include <iostream>

struct PdbInfo
{
	DWORD     Signature;
	BYTE      Guid[16];
	DWORD     Age;
	char      PdbFileName[1];
};

int wmain(int argc, wchar_t* argv[])
{
	if (argc < 2) {
		return 1;
	}

	auto hFile = CreateFile(argv[1], GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	auto hFileMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
	auto pvBase = MapViewOfFile(hFileMapping, FILE_MAP_READ, 0, 0, 0);
	auto pDosHeader = (IMAGE_DOS_HEADER*)pvBase;

	PIMAGE_SECTION_HEADER img;
	ULONG size;
	auto dbg_dir = (IMAGE_DEBUG_DIRECTORY*)ImageDirectoryEntryToDataEx(pvBase, FALSE, IMAGE_DIRECTORY_ENTRY_DEBUG, &size, &img);

	// Check to see that the data has the right type
	if (IMAGE_DEBUG_TYPE_CODEVIEW == dbg_dir->Type)
	{
		auto pdb_info = (PdbInfo*)((LPBYTE)(pvBase)+dbg_dir->PointerToRawData);
		if (0 == memcmp(&pdb_info->Signature, "RSDS", 4))
		{
			std::cout << pdb_info->PdbFileName;
		}
	}

	UnmapViewOfFile(pvBase);
	CloseHandle(hFileMapping);
	CloseHandle(hFile);
	return 0;
}
