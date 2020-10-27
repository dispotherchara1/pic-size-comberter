#pragma once

/* どの柄 何番のボタン */
#define BUTTOMSTATUS(status,num) { status , num, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0 }

/***********************************************
**  メモリとファイルハンドルの削除をする関数  **
***********************************************/
BOOL CleanMemFile() 
{
	/* ヒープの解放、メモリの削除 */
	if (hMemory)
	{
		if (!HeapDestroy(hMemory)) 
		{
			MessageBox(NULL, 
				L"メモリーの解放に失敗しました",
				L"HeapDestroy",
				MB_OK);
			return FALSE;
		}
		hMemory = NULL;
	}

	/* ファイルのクローズ */
	if (hFile) 
	{
		if (!CloseHandle(hFile)) 
		{
			MessageBox(NULL,
				L"ファイルのクローズに失敗しました",
				L"CloseHandle",
				MB_OK);
			return FALSE;
		}
		hFile = NULL;
	}
	return TRUE;
}


/***************************************
**  ファイルのオープンと読み取り関数  **
***************************************/
BOOL FileOpenRead(wchar_t* szFilePath) 
{
	MEMORYSTATUSEX msex = { sizeof(MEMORYSTATUSEX) };
	LARGE_INTEGER  liSize;
	DWORD dwBytesRead;
	DWORD ErrCode;
	wchar_t szErrMsg[64];

	/* ファイルのオプーナ */
	hFile = CreateFile(
		szFilePath,                    /* 選択したファイル名 */
		GENERIC_READ | GENERIC_WRITE,  /* 読み取りでオプーナ */
		0,                             /* 共有なし */
		NULL,                          /* セキュリティデフォルト */
		OPEN_EXISTING,                 /* 既存のファイルのみ */
		FILE_ATTRIBUTE_NORMAL,         /* ノーマル属性 */
		NULL);                         /* 属性テンプレなし */

	if (hFile == INVALID_HANDLE_VALUE) 
	{
		ErrCode = GetLastError();
		wsprintf(szErrMsg, L"ファイルをオープンできません:%d",ErrCode);
		MessageBox(NULL, szErrMsg, L"Create", MB_OK);
		return FALSE;
	}

	/* ファイルサイズの取得 ファイルを読み込むわけではないので注意 */
	GetFileSizeEx(hFile, &liSize);

	/* メモリ情報の取得 */
	GlobalMemoryStatusEx(&msex);
	if(msex.ullAvailVirtual<(unsigned)liSize.QuadPart)
	{
		MessageBox(NULL, "メモリが足りません", "GlobalMemoryStatusEX",MB_OK);
		CleanMemFile();
		return FALSE;
	}

	/* メモリの確保 */
	hMemory = HeapCreate(NULL, 0, 0);
	if (!(lpBuff = (wchar_t*)HeapAlloc(hMemory, NULL, (SIZE_T)liSize.QuadPart))) 
	{
		MessageBox(NULL, L"メモリの確保に失敗しました", L"HeapCreate", MB_OK);
		CleanMemFile();
		return FALSE;
	}

	/* ファイルの読み込み */
	if(!ReadFile(hFile,lpBuff,(DWORD)liSize.QuadPart,&dwBytesRead,NULL))
	{
		MessageBox(NULL,
			L"ファイルの読み込みに失敗しました",
			L"ReadFile",
			MB_OK);
		CleanMemFile();
		return FALSE;
	}

	/* 読み取りのサイズチェック */
	if((DWORD)liSize.QuadPart != dwBytesRead)
	{
		MessageBox(NULL,
			"ファイルの読み込みに失敗しました", 
			"ReadFile",
			MB_OK);
		CleanMemFile();
		return FALSE;
	}

	/* 文字列終端NULL文字   */
	lpBuff[dwBytesRead / sizeof(wchar_t)] = '\0';

	/* ファイルの内容を転送 */
	SendMessage(hEdit, WM_SETTEXT, 0, (LPARAM)lpBuff);

	return TRUE;
}
