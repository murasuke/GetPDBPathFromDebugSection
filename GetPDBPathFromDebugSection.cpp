#include <windows.h>
#include <dbghelp.h>
#include <iostream>

struct RSDS_DEBUG_FORMAT
{
	DWORD     signature;
	BYTE      guid[16];
	DWORD     age;
	char      pdbpath[1];
};

/**
 * プログラム内に含まれる「.pdb」ファイルのパスを抽出して表示する。
 * 引数:exeまたはdllのパス
 */
int wmain(int argc, wchar_t* argv[])
{
	if (argc < 2) {
		return 1;
	}

	// プログラムをメモリ上にマップする
	auto hFile = CreateFile(argv[1], GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return 1;
	}
	auto hFileMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
	auto pvBase = MapViewOfFile(hFileMapping, FILE_MAP_READ, 0, 0, 0);

	// ディレクトリエントリ(Debug)を取得
	PIMAGE_SECTION_HEADER imgSecHeader;
	ULONG dirEntrySize;
	auto dbgDir = (IMAGE_DEBUG_DIRECTORY*)ImageDirectoryEntryToDataEx(pvBase, FALSE, IMAGE_DIRECTORY_ENTRY_DEBUG, &dirEntrySize, &imgSecHeader);

	if (IMAGE_DEBUG_TYPE_CODEVIEW == dbgDir->Type)
	{
		auto pdb_info = (RSDS_DEBUG_FORMAT*)((LPBYTE)pvBase + dbgDir->PointerToRawData);
		if (memcmp(&pdb_info->signature, "RSDS", 4) == 0)
		{
			//RSDSシグネチャが見つかれば、pdfが含まれるので出力する
			std::cout << pdb_info->pdbpath;
		}
	}

	UnmapViewOfFile(pvBase);
	CloseHandle(hFileMapping);
	CloseHandle(hFile);
	return 0;
}
