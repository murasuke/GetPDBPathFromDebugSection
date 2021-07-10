# exeやdllに含まれるpdbファイルのパスを取得するサンプル

## はじめに
exeやdllには、デバッグの際に必要となる`.pdb`ファイルの**フルパス**が含まれています。
VisualStudioの規定フォルダでプロジェクトを作成すると、パスにユーザ名が含まれる(C:\Users\ユーザー名)ため、個人情報保護的に気持ちが良いものではありません。

という訳ではないですが、訳あって`.exe`や`.dll`ファイルに含まれる`.pdb`のパスを取するプログラムを作成しました。

.NET FrameworkではPInvokeが必要になるため、久しぶりにC言語です。
試行錯誤しながら、PEフォーマットを調べて目的のPDBにたどり着くことができました。

> ちなみに.NET Coreだと[PEHeader クラス](https://docs.microsoft.com/ja-jp/dotnet/api/system.reflection.portableexecutable.peheader?view=net-5.0)が追加されたため、簡単に取得できるようです。

## プログラム概要

* 引数に渡したプログラム内に`.pdb`ファイルのパスが含まれている場合、それを標準出力に出力します。
* .NETのexe でも ネイティブexe両方に対応しています(同じPEフォーマット)

## プログラムの説明

短いプログラムなのでインラインのコメントをご確認ください。

```C++
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

```


## 参考ページ

[PE(Portable Executable)ファイルフォーマットの概要](http://shopping2.gmobb.jp/htdmnr/www08/mcc/doc/pe.html)

[PE ファイルについて (5) - IMAGE_DATA_DIRECTORY](https://tech.blog.aerie.jp/entry/2015/12/27/144045)

[Getting your PDB name from a running executable (Windows)](https://deplinenoise.wordpress.com/2013/06/14/getting-your-pdb-name-from-a-running-executable-windows/)
 ⇒ 実行中のプロセスから取得する方法

[How to find corresponding .pdb inside .NET assembly?](https://stackoverflow.com/questions/38821662/how-to-find-corresponding-pdb-inside-net-assembly)
 ⇒.NET CoreのPEHeaderを利用した方法