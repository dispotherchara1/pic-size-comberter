#include <Windows.h>
#include <CommCtrl.h>
#include "resource.h"
#define TITLE TEXT("TestWindow")
#define IDC_EDIT 9999
#pragma comment(lib, "comctl32.lib")
#pragma region  �O���[�o���ϐ� 
HWND    Master_hWnd;    /* window�p�e�n���h��   */
HWND    Child_hWnd;     /* window�p�q�n���h��   */
HWND    W_hwnd_bottom;  /* window���̃{�^��     */
HWND    hEdit;          /* Edit�p�̃n���h��     */
HFONT   hFont;          /* �t�H���g�p�n���h��   */
HANDLE  hFile;          /* �t�@�C���p�n���h��   */
HANDLE  hMemory;        /* �������p�n���h��     */
WNDCLASS eClass;        /* �G�f�B�^�[�p�̃N���X */
OPENFILENAME ofn,ofns;  /*  */
wchar_t*     lpBuff;
HWND         hToolBar;       /* �c�[���o�[�p�n���h��     */
HWND         hStatus;        /* �X�e�[�^�X�o�[�p�n���h�� */
OPENFILENAME infile = { 0 }; /* ���g�O�̃t�@�C���m�۔�   */	
HBITMAP      hBitmap;        /* �r�b�g�}�b�v�p�n���h��   */ /* �摜��ɕ\�����邽�߂̃{�^����p�� */
HDC hdc , hBuffer;    
RECT        rc;
TEXTMETRIC  tm;
PAINTSTRUCT ps;
LPCREATESTRUCT lpcsWind;
wchar_t  szFilePath[MAX_PATH];
wchar_t  szFileTitle[MAX_PATH];
DWORD    ErrCode;
wchar_t  szErrMsg[64];
wchar_t szDir[MAX_PATH];
#pragma endregion
#include "Macro.h"

TCHAR strFile[MAX_PATH], strCustom[256] = TEXT("Brfor files\0*.*\0\0");TBBUTTON tbButton[] = {
	/* �w���v�{�^�� */
	BUTTOMSTATUS(STD_HELP   ,11),
	/* �\�����Ȃ��{�^���ƃ{�^���̊Ԃ���������ʂȃ{�^�� */
	{ NULL , NULL , TBSTATE_ENABLED, TBSTYLE_SEP, 0, 0, 0 },

	BUTTOMSTATUS(STD_FILEOPEN,1),
	BUTTOMSTATUS(STD_REPLACE ,2),
    BUTTOMSTATUS(STD_FILESAVE,3),

	/* Debug�p�{�^�� */
	BUTTOMSTATUS(STD_UNDO,4)
};

/*****************************
**  �E�B���h�E�v���V�[�W��  **
*****************************/
#pragma region WindowSuppoter
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	int nYesNo=NULL;    /* MessageBox�̎擾������������ */
	
	switch (msg)
	{
			/* Window����鎞 */
		case WM_CREATE:
		{
			/* hBitmap��IDB_BITMAP1���L�������� */
			hBitmap = LoadBitmap(
			((LPCREATESTRUCT)lp)->hInstance,
			TAJIMA);

			InitCommonControls();
			hToolBar = CreateToolbarEx(
				hWnd, WS_CHILD | WS_VISIBLE,
				0, 5,
				(HINSTANCE)HINST_COMMCTRL,
				IDB_STD_SMALL_COLOR,/* �{�^���p�G�̎Q��(��`����ĂȂ���G���[����Ώo��̂Ŗ���) */
				tbButton,   /* �{�^���̎Q�� */
				6, 48, 48,  /* �{�^���̐��A�{�^���ɕ`�悷��摜�T�C�Y */
				48, 48,     /* �{�^���̑傫�� */
				sizeof(TBBUTTON));

			/* �c�[���o�[ */
			hStatus = CreateStatusWindow(
				WS_CHILD | WS_VISIBLE |
				CCS_BOTTOM | SBARS_SIZEGRIP,
				TITLE, hWnd, 1);

			/* �t�H���g�̐ݒ� */
			hFont = CreateFont(
				14, 0, 0, 0,
				FW_NORMAL, FALSE, FALSE, 0,
				SHIFTJIS_CHARSET,
				OUT_DEFAULT_PRECIS,
				CLIP_DEFAULT_PRECIS,
				DEFAULT_QUALITY,
				DEFAULT_PITCH,
				L"�l�r �S�V�b�N");

			/* �N���C�A���g�G���A�̈�̃T�C�Y���擾 */
			GetClientRect(hWnd, &rc);

			/* �G�f�B�b�g�{�b�N�X�̃T�L���o�X */
			hEdit = CreateWindow(
				WS_EX_CLIENTEDGE,     /* �g���X�^�C�� */
				L"EDIT",   /* EDIT�N���X�� */
				L"",       /* �\�������� */
				/* �X�^�C��*/
				WS_CHILD | WS_VISIBLE | ES_WANTRETURN | ES_MULTILINE | WS_VSCROLL | ES_AUTOVSCROLL,
				rc.left,                       /* x */
				rc.top,                        /* y */
				rc.right,                      /* ��   */
				rc.bottom,                     /* ���� */
				Master_hWnd,                   /* �e�E�B���h�E�̃n���h�� */
				(HMENU)IDC_EDIT,               /* �G�f�B�b�g���\�[�XID   */
				((LPCREATESTRUCT)lp)->hInstance,   /* �C���X�^���X�n���h��*/
				NULL);                             /* CREATESTRUCT�\���� */

			/* �G�f�B�b�g�̍ő啶�����̕ύX */
			SendMessage(hEdit, EM_SETLIMITTEXT,0xffff, 0);
			
			/* �t�H���g�ݒ� */
			SendMessage(hEdit, WM_SETFONT,
				(WPARAM)hFont, MAKELPARAM(TRUE, 0));
				
			
			return 0;
		}
		break;

		/* CLOSE(�~�{�^��)����������DESTROY�͂���Ă��Ȃ��̂Ń^s�X�N�����S�ɏI������ɂ�CLOSE�����ޕK�v������ */
		/* ����{�^���������ꂽ�� */
		case WM_CLOSE:
		{
			/* ��Ԃ�DESTROY�֕ύX */
			PostMessage(hWnd, WM_DESTROY, 0, 0);
			return 0;
		}
		break;

		/* �c�[���o�[�̃{�^���������ꂽ�Ƃ� */
		case WM_COMMAND :
		{
			/* �ǂ̃{�^���������ꂽ�H */
			switch (LOWORD(wp))
			{
			default:
			case NULL:
				break;

			case 1:
			{
				/* �t�@�C�����J�����߂̉����� */
				ofn.lStructSize = sizeof(OPENFILENAME);
				ofn.hwndOwner   = hWnd;
				/* �J����E�Z�[�u�ł���g���q */
				ofn.lpstrFilter =
					TEXT("Text files {*.txt}\0*.txt\0")
					TEXT("HTML files {*.htm}\0*.html;*.htm\0")
					TEXT("PNG files {*.png}\0*.png\0")
					TEXT("BITMAP files {*.bmp}\0*.bmp\0")
					TEXT("JPEG files {*.jpeg}\0*jpg;*.jpg;*.jpe;*jfif\0")
					TEXT("All files {*.*}\0*.*\0\0");
				/*ofn.lpstrCustomFilter = strCustom;*/
				ofn.nFilterIndex    = 1;
				ofn.lpstrFile       = szFilePath;
				ofn.lpstrFile[0]    = '\0';
				ofn.nMaxFile        = MAX_PATH;
				ofn.nMaxFileTitle   = MAX_PATH;
				ofn.lpstrFileTitle  = szFileTitle;
				ofn.lpstrInitialDir = NULL;
				ofn.lpstrTitle      = "�t�@�C�����J��";
				/*ofn.nMaxCustFilter = 256;*/
				ofn.Flags       = OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST;
				ofn.lpstrDefExt = "txt";

				/* [�J��]�_�C�A���O */
				if (!GetOpenFileName(&ofn)) {
					ErrCode = CommDlgExtendedError();
					if (ErrCode) {
						/*SetWindowText(hWnd, strFile);*/
						wsprintf(szErrMsg, L"ErrorCode:%d", ErrCode);
						MessageBox(NULL,
							szErrMsg,
							"GetOpenFileName",
							MB_OK|MB_ICONWARNING);
						return FALSE;
					}
					/* �L�����Z�������ꍇ */
					return FALSE;
				}

				/* �t�@�C�������ɃI�[�v������Ă���ꍇ */
				if (!CleanMemFile()) return FALSE;
				
				/* �t�@�C���̃I�[�v���Ɠǂݎ�� */
				if (!FileOpenRead(szFilePath)) return FALSE;
				
				/* �t�@�C�������E�B���h���^�C�g���ɕ\�� */
				SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)szFileTitle);
			}
			break;

			case 2:
				break;

			case 3:
			{
				DWORD dwWritten;
				/* �t�@�C�����I�΂�ĂȂ��Əo�͂ł��Ȃ� */
				if(ofn.lpstrFileTitle == NULL
					||ofn.lpstrFile   == NULL
					||ofn.nFileOffset == 0)
				{

					MessageBox(
						Master_hWnd,
						"�t�@�C�����I�΂�Ă��Ȃ����ߕۑ��ł��܂���",
						"�x��",
						MB_OK|MB_ICONWARNING);
					return FALSE;
				}
				
				/* ���݂̃t�@�C�������f�t�H���g�̕ۑ����ɐݒ� */
				wcscpy_s(szFilePath, MAX_PATH, ofn.lpstrFileTitle);
				/* ���݂̃f�B���N�g���ɏ����l��ݒ� */
				wcscpy_s(szDir, MAX_PATH, ofn.lpstrFile);
				szDir[ofn.nFileOffset - 1] = '\0';
				
				ofns.lStructSize = sizeof(OPENFILENAME);
                ofns.hwndOwner   = hWnd;
				ofns.lpstrFilter =
					TEXT("Text files {*.txt}\0*.txt\0")
					TEXT("HTML files {*.htm}\0*.html;*.htm\0")
					TEXT("PNG files {*.png}\0*.png\0")
					TEXT("BITMAP files {*.bmp}\0*.bmp\0")
					TEXT("JPEG files {*.jpeg}\0*jpg;*.jpg;*.jpe;*jfif\0")
					TEXT("All files {*.*}\0*.*\0\0");
                ofns.nFilterIndex = 1;
                ofns.lpstrFile    = szFilePath;
                ofns.nMaxFile     = MAX_PATH;
                ofns.lpstrFileTitle  = szFileTitle;
                ofns.nMaxFileTitle   = MAX_PATH;
                ofns.lpstrInitialDir = szDir;
                ofns.lpstrTitle = "���O��t���ĕۑ�";
				ofns.Flags      = OFN_PATHMUSTEXIST |
					OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;
				
				ofns.lpstrDefExt = "txt";/* �f�t�H���g�̊g���q */


				/* ���O��t���ĕۑ��_�C�A���O�{�b�N�X */
				if (!GetSaveFileName(&ofns)) 
				{
					ErrCode = CommDlgExtendedError();
					if(ErrCode)
					{
						wsprintf(szErrMsg, "�G���[�R�[�h:%d", ErrCode);
						MessageBox(Master_hWnd,
							szErrMsg,
							"GetOpenFileName",
							MB_OK|MB_ICONWARNING);
						return FALSE;
					}
					/* �L�����Z�������ꍇ */
					return FALSE;
				}

				/* �t�@�C�����N���[�Y */
				CloseHandle(hFile);

				// �t�@�C����V�K�쐬
                hFile = CreateFile(szFilePath,       // �I�������t�@�C����
                    GENERIC_READ | GENERIC_WRITE,    // �ǂݎ��ŃI�[�v��
                    0,                               // ���L�Ȃ�
                    NULL,                            // �f�t�H���g�Z�L�����e�B
                    CREATE_NEW,                      // �V�K�쐬
                    FILE_ATTRIBUTE_NORMAL,           // �m�[�}������
                    NULL);                           // �����e���v���[�g�Ȃ�

				/* ���ɑ��݂���t�@�C�����̏ꍇ�ۑ��ł��Ȃ��悤�ɂ��� */
                /*if (hFile == INVALID_HANDLE_VALUE) {
                    ErrCode = GetLastError();
                    wsprintf(szErrMsg, "�t�@�C�����쐬�ł��܂���: %d", ErrCode);
                    MessageBox(Master_hWnd,
						szErrMsg,
						"CreateFile",
						MB_OK|MB_ICONWARNING);
                    return FALSE;
                }*/

                // �G�f�B�b�g�̃o�b�t�@�擾
                HANDLE   hMem   = (HANDLE)SendMessage(hEdit, EM_GETHANDLE, 0, 0);
                wchar_t* lpEdit = ((wchar_t*)LocalLock(/*(HLOCAL)*/hMem));
                wchar_t* p = lpEdit;

                // �o�b�t�@�̃T�C�Y�擾
                DWORD dwCount = 0;
                while (*p++)
                    dwCount++;
                dwCount *= sizeof(wchar_t);
				
                // ��������
                WriteFile(hFile, lpEdit, dwCount, &dwWritten, NULL);
                if (dwCount != dwWritten) {
                    MessageBox(Master_hWnd,
						"�������݂Ɏ��s���܂���",
						"WriteFile",
						MB_OK|MB_ICONWARNING);
                    return FALSE;
                }

                // �G�f�B�b�g�̃o�b�t�@���A�����b�N
                LocalUnlock((HLOCAL)hMem);
                   
                // �t�@�C�����N���[�Y
                CloseHandle(hFile);

                // �t�@�C���̍ăI�[�v��
                if (!FileOpenRead(szFilePath))
                    return FALSE;
                // �t�@�C�������E�B���h�E�^�C�g���ɕ\��
                SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)szFileTitle);
			}
			break;

			case 4:
				MessageBox(
					NULL,
					"��",
					"�f�o�b�O�p",
					MB_OK);
				break;

			case 11:
			{
				nYesNo = MessageBox(
					Master_hWnd,
					"���̃{�^���Ńt�@�C�����J���܂�\n"
					"�^�񒆂̃{�^���ŉ摜�T�C�Y���ς��܂�\n"
					"�E�̃{�^���ŕۑ����܂�\n"
					"���̓_�C�A���O�ȊO������"
					/*"https://i.imgur.com/OW9U72u.jpg"*/,
					"Help",
					MB_OK);
			}
			break;
			}
			return 0;
		}
		break;

			/* DESTROY�Ȃ̂�window���j�󂳂ꂽ��ɔ������� */
			/* Window�̔j�� */
		case WM_DESTROY:
		{
			DeleteObject(hBitmap);
			PostQuitMessage(0);
			return 0;
		}
		break;

			/* ��������L�[�������ꂽ�� */
		case WM_SYSKEYUP:
			break;

			/* �N���C�A���g�G���A�ɂ��낢��`��ł��� */
		case WM_PAINT:
		{
			hdc = BeginPaint(hWnd, &ps);
			hBuffer = CreateCompatibleDC(hdc);
			SelectObject(hBuffer, hBitmap);

			/* GHOST of TAJIMA�̕`�� */
			BitBlt(hdc, -500,-100, 1900, 1100, hBuffer, 0, 0, SRCCOPY);

			DeleteDC(hBuffer);
			EndPaint(hWnd, &ps);
			return 0;
		};
		break;

		case WM_SIZE:
		{
			/* �N���C�A���g�̈�̃T�C�Y�擾 */
			GetClientRect(hWnd, &rc);
			/* �E�B���h�E�̃T�C�Y�ɍ��킹�ăG�f�B�b�g�̃T�C�Y�ύX */
			MoveWindow(hEdit,
				rc.left,rc.top,rc.right,
				rc.bottom,TRUE);
		};
		break;
	}
	/* Window�Ƃ��Ċ�{�̓��������^�[�� */
	return DefWindowProc(hWnd, msg, wp, lp);
};
#pragma endregion 

#pragma region WindowCreater
void IniWindow(HINSTANCE hInstance)
{
	TCHAR szAppName[] = TEXT("TestApp");
	MSG msg;
	WNDCLASS winc;
	
	winc.style       = CS_HREDRAW | CS_VREDRAW;  /**/
	winc.lpfnWndProc = WndProc;                  /* �E�B���h�E�v���V�[�W���Ăяo�� */
	winc.cbClsExtra  = winc.cbWndExtra = 0;      /**/
	winc.hInstance   = hInstance;                /**/
	winc.hIcon   = LoadIcon(NULL, IDI_APPLICATION);      /**/
	winc.hCursor = LoadCursor(NULL, IDC_ARROW);          /* �}�E�X�̎�� */
	winc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);  /**/
	winc.lpszMenuName  = NULL;
	winc.lpszClassName = szAppName;

	eClass.style       = CS_HREDRAW | CS_VREDRAW;  /**/
	eClass.lpfnWndProc = WndProc;                  /* �E�B���h�E�v���V�[�W���Ăяo�� */
	eClass.cbClsExtra  = winc.cbWndExtra = 0;      /**/
	eClass.hInstance   = hInstance;                /**/
	eClass.hIcon   = LoadIcon(NULL, IDI_APPLICATION);      /**/
	eClass.hCursor = LoadCursor(NULL, IDC_ARROW);          /* �}�E�X�̎�� */
	eClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);  /**/
	eClass.lpszMenuName  = NULL;
	eClass.lpszClassName = WS_EX_CLIENTEDGE;

	/* �E�B���h�E�N���X�ւ̓o�^ */
	/* winc�̃A�h���X��Master_hWnd�̑�����o�^�A���s��Null��Ԃ�return 0 */
	if (!RegisterClass(&winc))return -1;

	/* �E�B���h�E�쐬 */
	Master_hWnd = CreateWindowA(
		szAppName,
		TITLE,
		WS_OVERLAPPED | WS_SYSMENU,
		CW_USEDEFAULT, CW_USEDEFAULT, /* X,Y(�ʒu), */
		1400, 1000, /* X,Y(�X�P�[��)    */
		NULL,       /* window�̃n���h�� */
		NULL,       /* mennyu�̃n���h�� */
		hInstance,  /* window�𐶐����郂�W���[���̃n���h�� */
		NULL);     /* WM_CREATE��LPARAM�ɓn�������l        */

	if (Master_hWnd == NULL)return -1;
}
#pragma endregion

#pragma region Main
int WINAPI WinMain(
	HINSTANCE hinstansce,
	HINSTANCE hpreInstance,
	LPSTR lpCmdLine,
	int nCmdSHow)
{
	MSG hMsg;
	IniWindow(hinstansce);
	ShowWindow(Master_hWnd, nCmdSHow);
	UpdateWindow(Master_hWnd);

	ZeroMemory(&hMsg, sizeof(hMsg));
	while (hMsg.message != WM_QUIT)
	{
		if (PeekMessage(&hMsg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&hMsg);
			DispatchMessage(&hMsg);
		}
		else
		{

		}
	};
	return 0;
};
#pragma endregion