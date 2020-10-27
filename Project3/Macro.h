#pragma once

/* �ǂ̕� ���Ԃ̃{�^�� */
#define BUTTOMSTATUS(status,num) { status , num, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0 }

/***********************************************
**  �������ƃt�@�C���n���h���̍폜������֐�  **
***********************************************/
BOOL CleanMemFile() 
{
	/* �q�[�v�̉���A�������̍폜 */
	if (hMemory)
	{
		if (!HeapDestroy(hMemory)) 
		{
			MessageBox(NULL, 
				L"�������[�̉���Ɏ��s���܂���",
				L"HeapDestroy",
				MB_OK);
			return FALSE;
		}
		hMemory = NULL;
	}

	/* �t�@�C���̃N���[�Y */
	if (hFile) 
	{
		if (!CloseHandle(hFile)) 
		{
			MessageBox(NULL,
				L"�t�@�C���̃N���[�Y�Ɏ��s���܂���",
				L"CloseHandle",
				MB_OK);
			return FALSE;
		}
		hFile = NULL;
	}
	return TRUE;
}


/***************************************
**  �t�@�C���̃I�[�v���Ɠǂݎ��֐�  **
***************************************/
BOOL FileOpenRead(wchar_t* szFilePath) 
{
	MEMORYSTATUSEX msex = { sizeof(MEMORYSTATUSEX) };
	LARGE_INTEGER  liSize;
	DWORD dwBytesRead;
	DWORD ErrCode;
	wchar_t szErrMsg[64];

	/* �t�@�C���̃I�v�[�i */
	hFile = CreateFile(
		szFilePath,                    /* �I�������t�@�C���� */
		GENERIC_READ | GENERIC_WRITE,  /* �ǂݎ��ŃI�v�[�i */
		0,                             /* ���L�Ȃ� */
		NULL,                          /* �Z�L�����e�B�f�t�H���g */
		OPEN_EXISTING,                 /* �����̃t�@�C���̂� */
		FILE_ATTRIBUTE_NORMAL,         /* �m�[�}������ */
		NULL);                         /* �����e���v���Ȃ� */

	if (hFile == INVALID_HANDLE_VALUE) 
	{
		ErrCode = GetLastError();
		wsprintf(szErrMsg, L"�t�@�C�����I�[�v���ł��܂���:%d",ErrCode);
		MessageBox(NULL, szErrMsg, L"Create", MB_OK);
		return FALSE;
	}

	/* �t�@�C���T�C�Y�̎擾 �t�@�C����ǂݍ��ނ킯�ł͂Ȃ��̂Œ��� */
	GetFileSizeEx(hFile, &liSize);

	/* ���������̎擾 */
	GlobalMemoryStatusEx(&msex);
	if(msex.ullAvailVirtual<(unsigned)liSize.QuadPart)
	{
		MessageBox(NULL, "������������܂���", "GlobalMemoryStatusEX",MB_OK);
		CleanMemFile();
		return FALSE;
	}

	/* �������̊m�� */
	hMemory = HeapCreate(NULL, 0, 0);
	if (!(lpBuff = (wchar_t*)HeapAlloc(hMemory, NULL, (SIZE_T)liSize.QuadPart))) 
	{
		MessageBox(NULL, L"�������̊m�ۂɎ��s���܂���", L"HeapCreate", MB_OK);
		CleanMemFile();
		return FALSE;
	}

	/* �t�@�C���̓ǂݍ��� */
	if(!ReadFile(hFile,lpBuff,(DWORD)liSize.QuadPart,&dwBytesRead,NULL))
	{
		MessageBox(NULL,
			L"�t�@�C���̓ǂݍ��݂Ɏ��s���܂���",
			L"ReadFile",
			MB_OK);
		CleanMemFile();
		return FALSE;
	}

	/* �ǂݎ��̃T�C�Y�`�F�b�N */
	if((DWORD)liSize.QuadPart != dwBytesRead)
	{
		MessageBox(NULL,
			"�t�@�C���̓ǂݍ��݂Ɏ��s���܂���", 
			"ReadFile",
			MB_OK);
		CleanMemFile();
		return FALSE;
	}

	/* ������I�[NULL����   */
	lpBuff[dwBytesRead / sizeof(wchar_t)] = '\0';

	/* �t�@�C���̓��e��]�� */
	SendMessage(hEdit, WM_SETTEXT, 0, (LPARAM)lpBuff);

	return TRUE;
}
