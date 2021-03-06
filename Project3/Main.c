#include <Windows.h>
#include <CommCtrl.h>
#include "resource.h"
#define TITLE TEXT("TestWindow")
#define IDC_EDIT 9999
#pragma comment(lib, "comctl32.lib")
#pragma region  グローバル変数 
HWND    Master_hWnd;    /* window用親ハンドル   */
HWND    Child_hWnd;     /* window用子ハンドル   */
HWND    W_hwnd_bottom;  /* window内のボタン     */
HWND    hEdit;          /* Edit用のハンドル     */
HFONT   hFont;          /* フォント用ハンドル   */
HANDLE  hFile;          /* ファイル用ハンドル   */
HANDLE  hMemory;        /* メモリ用ハンドル     */
WNDCLASS eClass;        /* エディター用のクラス */
OPENFILENAME ofn,ofns;  /*  */
wchar_t*     lpBuff;
HWND         hToolBar;       /* ツールバー用ハンドル     */
HWND         hStatus;        /* ステータスバー用ハンドル */
OPENFILENAME infile = { 0 }; /* 中身０のファイル確保箱   */	
HBITMAP      hBitmap;        /* ビットマップ用ハンドル   */ /* 画像上に表示するためのボタンを用意 */
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
	/* ヘルプボタン */
	BUTTOMSTATUS(STD_HELP   ,11),
	/* 表示しないボタンとボタンの間をあける特別なボタン */
	{ NULL , NULL , TBSTATE_ENABLED, TBSTYLE_SEP, 0, 0, 0 },

	BUTTOMSTATUS(STD_FILEOPEN,1),
	BUTTOMSTATUS(STD_REPLACE ,2),
    BUTTOMSTATUS(STD_FILESAVE,3),

	/* Debug用ボタン */
	BUTTOMSTATUS(STD_UNDO,4)
};

/*****************************
**  ウィンドウプロシージャ  **
*****************************/
#pragma region WindowSuppoter
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	int nYesNo=NULL;    /* MessageBoxの取得した数を入れる */
	
	switch (msg)
	{
			/* Windowを作る時 */
		case WM_CREATE:
		{
			/* hBitmapにIDB_BITMAP1を記憶させる */
			hBitmap = LoadBitmap(
			((LPCREATESTRUCT)lp)->hInstance,
			TAJIMA);

			InitCommonControls();
			hToolBar = CreateToolbarEx(
				hWnd, WS_CHILD | WS_VISIBLE,
				0, 5,
				(HINSTANCE)HINST_COMMCTRL,
				IDB_STD_SMALL_COLOR,/* ボタン用絵の参照(定義されてないよエラーが絶対出るので無視) */
				tbButton,   /* ボタンの参照 */
				6, 48, 48,  /* ボタンの数、ボタンに描画する画像サイズ */
				48, 48,     /* ボタンの大きさ */
				sizeof(TBBUTTON));

			/* ツールバー */
			hStatus = CreateStatusWindow(
				WS_CHILD | WS_VISIBLE |
				CCS_BOTTOM | SBARS_SIZEGRIP,
				TITLE, hWnd, 1);

			/* フォントの設定 */
			hFont = CreateFont(
				14, 0, 0, 0,
				FW_NORMAL, FALSE, FALSE, 0,
				SHIFTJIS_CHARSET,
				OUT_DEFAULT_PRECIS,
				CLIP_DEFAULT_PRECIS,
				DEFAULT_QUALITY,
				DEFAULT_PITCH,
				L"ＭＳ ゴシック");

			/* クライアントエリア領域のサイズを取得 */
			GetClientRect(hWnd, &rc);

			/* エディットボックスのサキュバス */
			hEdit = CreateWindow(
				WS_EX_CLIENTEDGE,     /* 拡張スタイル */
				L"EDIT",   /* EDITクラス名 */
				L"",       /* 表示文字列 */
				/* スタイル*/
				WS_CHILD | WS_VISIBLE | ES_WANTRETURN | ES_MULTILINE | WS_VSCROLL | ES_AUTOVSCROLL,
				rc.left,                       /* x */
				rc.top,                        /* y */
				rc.right,                      /* 幅   */
				rc.bottom,                     /* 高さ */
				Master_hWnd,                   /* 親ウィンドウのハンドル */
				(HMENU)IDC_EDIT,               /* エディットリソースID   */
				((LPCREATESTRUCT)lp)->hInstance,   /* インスタンスハンドル*/
				NULL);                             /* CREATESTRUCT構造体 */

			/* エディットの最大文字数の変更 */
			SendMessage(hEdit, EM_SETLIMITTEXT,0xffff, 0);
			
			/* フォント設定 */
			SendMessage(hEdit, WM_SETFONT,
				(WPARAM)hFont, MAKELPARAM(TRUE, 0));
				
			
			return 0;
		}
		break;

		/* CLOSE(×ボタン)を押した時DESTROYはされていないのでタsスクを安全に終了するにはCLOSEを挟む必要がある */
		/* 閉じるボタンが押されたら */
		case WM_CLOSE:
		{
			/* 状態をDESTROYへ変更 */
			PostMessage(hWnd, WM_DESTROY, 0, 0);
			return 0;
		}
		break;

		/* ツールバーのボタンが押されたとき */
		case WM_COMMAND :
		{
			/* どのボタンが押された？ */
			switch (LOWORD(wp))
			{
			default:
			case NULL:
				break;

			case 1:
			{
				/* ファイルを開くための下準備 */
				ofn.lStructSize = sizeof(OPENFILENAME);
				ofn.hwndOwner   = hWnd;
				/* 開ける・セーブできる拡張子 */
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
				ofn.lpstrTitle      = "ファイルを開く";
				/*ofn.nMaxCustFilter = 256;*/
				ofn.Flags       = OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST;
				ofn.lpstrDefExt = "txt";

				/* [開く]ダイアログ */
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
					/* キャンセルした場合 */
					return FALSE;
				}

				/* ファイルが既にオープンされている場合 */
				if (!CleanMemFile()) return FALSE;
				
				/* ファイルのオープンと読み取り */
				if (!FileOpenRead(szFilePath)) return FALSE;
				
				/* ファイル名をウィンドをタイトルに表示 */
				SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)szFileTitle);
			}
			break;

			case 2:
				break;

			case 3:
			{
				DWORD dwWritten;
				/* ファイルが選ばれてないと出力できない */
				if(ofn.lpstrFileTitle == NULL
					||ofn.lpstrFile   == NULL
					||ofn.nFileOffset == 0)
				{

					MessageBox(
						Master_hWnd,
						"ファイルが選ばれていないため保存できません",
						"警告",
						MB_OK|MB_ICONWARNING);
					return FALSE;
				}
				
				/* 現在のファイル名をデフォルトの保存名に設定 */
				wcscpy_s(szFilePath, MAX_PATH, ofn.lpstrFileTitle);
				/* 現在のディレクトリに初期値を設定 */
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
                ofns.lpstrTitle = "名前を付けて保存";
				ofns.Flags      = OFN_PATHMUSTEXIST |
					OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;
				
				ofns.lpstrDefExt = "txt";/* デフォルトの拡張子 */


				/* 名前を付けて保存ダイアログボックス */
				if (!GetSaveFileName(&ofns)) 
				{
					ErrCode = CommDlgExtendedError();
					if(ErrCode)
					{
						wsprintf(szErrMsg, "エラーコード:%d", ErrCode);
						MessageBox(Master_hWnd,
							szErrMsg,
							"GetOpenFileName",
							MB_OK|MB_ICONWARNING);
						return FALSE;
					}
					/* キャンセルした場合 */
					return FALSE;
				}

				/* ファイルをクローズ */
				CloseHandle(hFile);

				// ファイルを新規作成
                hFile = CreateFile(szFilePath,       // 選択したファイル名
                    GENERIC_READ | GENERIC_WRITE,    // 読み取りでオープン
                    0,                               // 共有なし
                    NULL,                            // デフォルトセキュリティ
                    CREATE_NEW,                      // 新規作成
                    FILE_ATTRIBUTE_NORMAL,           // ノーマル属性
                    NULL);                           // 属性テンプレートなし

				/* 既に存在するファイル名の場合保存できないようにする */
                /*if (hFile == INVALID_HANDLE_VALUE) {
                    ErrCode = GetLastError();
                    wsprintf(szErrMsg, "ファイルを作成できません: %d", ErrCode);
                    MessageBox(Master_hWnd,
						szErrMsg,
						"CreateFile",
						MB_OK|MB_ICONWARNING);
                    return FALSE;
                }*/

                // エディットのバッファ取得
                HANDLE   hMem   = (HANDLE)SendMessage(hEdit, EM_GETHANDLE, 0, 0);
                wchar_t* lpEdit = ((wchar_t*)LocalLock(/*(HLOCAL)*/hMem));
                wchar_t* p = lpEdit;

                // バッファのサイズ取得
                DWORD dwCount = 0;
                while (*p++)
                    dwCount++;
                dwCount *= sizeof(wchar_t);
				
                // 書き込み
                WriteFile(hFile, lpEdit, dwCount, &dwWritten, NULL);
                if (dwCount != dwWritten) {
                    MessageBox(Master_hWnd,
						"書き込みに失敗しました",
						"WriteFile",
						MB_OK|MB_ICONWARNING);
                    return FALSE;
                }

                // エディットのバッファをアンロック
                LocalUnlock((HLOCAL)hMem);
                   
                // ファイルをクローズ
                CloseHandle(hFile);

                // ファイルの再オープン
                if (!FileOpenRead(szFilePath))
                    return FALSE;
                // ファイル名をウィンドウタイトルに表示
                SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)szFileTitle);
			}
			break;

			case 4:
				MessageBox(
					NULL,
					"あ",
					"デバッグ用",
					MB_OK);
				break;

			case 11:
			{
				nYesNo = MessageBox(
					Master_hWnd,
					"左のボタンでファイルを開きます\n"
					"真ん中のボタンで画像サイズが変わります\n"
					"右のボタンで保存します\n"
					"今はダイアログ以外未実装"
					/*"https://i.imgur.com/OW9U72u.jpg"*/,
					"Help",
					MB_OK);
			}
			break;
			}
			return 0;
		}
		break;

			/* DESTROYなのでwindowが破壊された後に発動する */
			/* Windowの破壊 */
		case WM_DESTROY:
		{
			DeleteObject(hBitmap);
			PostQuitMessage(0);
			return 0;
		}
		break;

			/* 何かしらキーが押されたら */
		case WM_SYSKEYUP:
			break;

			/* クライアントエリアにいろいろ描画できる */
		case WM_PAINT:
		{
			hdc = BeginPaint(hWnd, &ps);
			hBuffer = CreateCompatibleDC(hdc);
			SelectObject(hBuffer, hBitmap);

			/* GHOST of TAJIMAの描画 */
			BitBlt(hdc, -500,-100, 1900, 1100, hBuffer, 0, 0, SRCCOPY);

			DeleteDC(hBuffer);
			EndPaint(hWnd, &ps);
			return 0;
		};
		break;

		case WM_SIZE:
		{
			/* クライアント領域のサイズ取得 */
			GetClientRect(hWnd, &rc);
			/* ウィンドウのサイズに合わせてエディットのサイズ変更 */
			MoveWindow(hEdit,
				rc.left,rc.top,rc.right,
				rc.bottom,TRUE);
		};
		break;
	}
	/* Windowとして基本の動きをリターン */
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
	winc.lpfnWndProc = WndProc;                  /* ウィンドウプロシージャ呼び出し */
	winc.cbClsExtra  = winc.cbWndExtra = 0;      /**/
	winc.hInstance   = hInstance;                /**/
	winc.hIcon   = LoadIcon(NULL, IDI_APPLICATION);      /**/
	winc.hCursor = LoadCursor(NULL, IDC_ARROW);          /* マウスの種類 */
	winc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);  /**/
	winc.lpszMenuName  = NULL;
	winc.lpszClassName = szAppName;

	eClass.style       = CS_HREDRAW | CS_VREDRAW;  /**/
	eClass.lpfnWndProc = WndProc;                  /* ウィンドウプロシージャ呼び出し */
	eClass.cbClsExtra  = winc.cbWndExtra = 0;      /**/
	eClass.hInstance   = hInstance;                /**/
	eClass.hIcon   = LoadIcon(NULL, IDI_APPLICATION);      /**/
	eClass.hCursor = LoadCursor(NULL, IDC_ARROW);          /* マウスの種類 */
	eClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);  /**/
	eClass.lpszMenuName  = NULL;
	eClass.lpszClassName = WS_EX_CLIENTEDGE;

	/* ウィンドウクラスへの登録 */
	/* wincのアドレスにMaster_hWndの属性を登録、失敗はNullを返しreturn 0 */
	if (!RegisterClass(&winc))return -1;

	/* ウィンドウ作成 */
	Master_hWnd = CreateWindowA(
		szAppName,
		TITLE,
		WS_OVERLAPPED | WS_SYSMENU,
		CW_USEDEFAULT, CW_USEDEFAULT, /* X,Y(位置), */
		1400, 1000, /* X,Y(スケール)    */
		NULL,       /* windowのハンドル */
		NULL,       /* mennyuのハンドル */
		hInstance,  /* windowを生成するモジュールのハンドル */
		NULL);     /* WM_CREATEでLPARAMに渡したい値        */

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