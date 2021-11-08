#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <Windows.h>

#define ADD_ENEMY(x, y) objInit(newObject(), x, y, 50, 50, 'e');

typedef struct
{
	float x, y;
} TPoint;

typedef struct
{
	TPoint pos;
	TPoint size;
	TPoint speed;
	COLORREF color;
	char type;
	float range, vecSpeed;
	BOOL isDel;
} TObj, * PObj;

TObj player;
PObj arr = NULL;
int arrCnt = 0;

TPoint cam;
RECT rct;

int score = 0;

BOOL gameOver = FALSE;

void winInit();

PObj newObject()
{
	arrCnt++;
	arr = realloc(arr, sizeof(*arr) * arrCnt);

	return arr + arrCnt - 1;
}


TPoint point(float x, float y)
{
	TPoint pnt;
	pnt.x = x;
	pnt.y = y;

	return pnt;
}


BOOL isCollision(TObj obj1, TObj obj2)
{
	return (((obj1.pos.x + obj1.size.x) > obj2.pos.x) &&
		((obj2.pos.x + obj2.size.x) > obj1.pos.x) &&
		((obj1.pos.y + obj1.size.y) > obj2.pos.y) &&
		((obj2.pos.y + obj2.size.y) > obj1.pos.y));
}


void setCameraFocus(TObj obj)
{
	cam = point((obj.pos.x + obj.size.x / 2) - rct.right / 2,
		(obj.pos.y + obj.size.y / 2) - rct.bottom / 2);
}


void objSetDestPoint(PObj obj, float x, float y, float vecSpeed)
{
	// расстояние от объекта до цели
	TPoint lenght = point(x - obj->pos.x, y - obj->pos.y);

	// путь от объекта до цели
	float xyLen = sqrt(lenght.x * lenght.x + lenght.y * lenght.y);

	obj->speed.x = lenght.x / xyLen * vecSpeed;
	obj->speed.y = lenght.y / xyLen * vecSpeed;

	obj->vecSpeed = vecSpeed;
}


void objDelete()
{
	for (int i = 0; i < arrCnt; )
	{
		if ((arr + i)->isDel)
		{
			arrCnt--;
			arr[i] = arr[arrCnt];
			arr = realloc(arr, sizeof(*arr) * arrCnt);
		}
		else
		{
			i++;
		}
	}
}


void objInit(PObj obj, float x, float y, float width, float height, char type)
{
	obj->pos = point(x, y);
	obj->size = point(width, height);
	obj->color = RGB(0, 255, 0);
	obj->speed = point(0, 0);
	obj->type = type;
	obj->isDel = FALSE;

	if (obj->type == 'e')
	{
		obj->color = RGB(255, 0, 0);
	}

	if ((obj->type == 'b') || (obj->type == 'B'))
	{
		obj->color = RGB(255, 140, 0);
		obj->range = 300;

		if (obj->type == 'B')
		{
			obj->range = 200;
		}
	}
}


void objShow(HDC dc, TObj obj)
{
	SelectObject(dc, GetStockObject(DC_PEN));
	SetDCPenColor(dc, RGB(0, 0, 0));
	SelectObject(dc, GetStockObject(DC_BRUSH));
	SetDCBrushColor(dc, obj.color);

	BOOL(WINAPI * shape)(HDC, int, int, int, int);
	shape = (obj.type == 'e') || (obj.type == 'b') || (obj.type == 'B') ? Ellipse : Rectangle;

	shape(dc, obj.pos.x - cam.x, obj.pos.y - cam.y, obj.pos.x + obj.size.x - cam.x,
		obj.pos.y + obj.size.y - cam.y);
}


void genNewEnemy()
{
	int xRand = rand() % 2 == 0 ? 300 : -300;
	int yRand = rand() % 2 == 0 ? 300 : -300;

	if (rand() % 2 == 0)
	{
		xRand = rand() % 600 - 300;
	}
	else
	{
		yRand = rand() % 600 - 300;
	}

	int k = rand() % 40;

	if (k == 0)
	{
		ADD_ENEMY(player.pos.x + xRand, player.pos.y + yRand);
	}
}


void objMove(PObj obj)
{
	obj->pos.x += obj->speed.x;
	obj->pos.y += obj->speed.y;

	if ((obj->type == 'b') || (obj->type == 'B'))
	{
		obj->range -= obj->vecSpeed;

		if (obj->range < 0)
		{
			obj->isDel = TRUE;
		}

		for (int i = 0; i < arrCnt; i++)
		{
			if ((isCollision(*obj, arr[i])) && (arr[i].type == 'e'))
			{
				score += 10;

				obj->isDel = TRUE;
				arr[i].isDel = TRUE;
			}
		}
	}

	if (obj->type == 'e')
	{
		if (rand() % 40 == 0)
		{
			objSetDestPoint(obj, player.pos.x, player.pos.y, 1.5);
		}

		if (isCollision(*obj, player))
		{
			gameOver = TRUE;
		}
	}

	if (rand() % 20 == 0)
	{
		genNewEnemy();
	}

	if (gameOver)
	{
		winInit();
	}

	objDelete();
}


void playerControl()
{
	static int playerSpeed = 2;
	
	player.speed.x = 0;
	player.speed.y = 0;

	if (GetKeyState('W') < 0)
	{
		player.speed.y = -playerSpeed;
	}

	if (GetKeyState('S') < 0)
	{
		player.speed.y = playerSpeed;
	}

	if (GetKeyState('A') < 0)
	{
		player.speed.x = -playerSpeed;
	}

	if (GetKeyState('D') < 0)
	{
		player.speed.x = playerSpeed;
	}

	if ((player.speed.x != 0) && (player.speed.y != 0))
	{
		player.speed = point(player.speed.x * 0.7, player.speed.y * 0.7);
	}
}


void createBullet(PObj obj, float x, float y, char bulType)
{
	if (bulType == 'b')
	{
		objInit(obj, player.pos.x + player.size.x / 2, player.pos.y + player.size.y / 2,
			10, 10, bulType);
		objSetDestPoint(obj, x, y, 2.5);
	}
	else if (bulType == 'B')
	{
		objInit(obj, player.pos.x + player.size.x / 2, player.pos.y + player.size.y / 2,
			20, 20, bulType);
		objSetDestPoint(obj, x, y, 1.5);
	}
}


void winInit()
{
	score = 0;
	gameOver = FALSE;
	arrCnt = 0;
	arr = realloc(arr, 0);
	objInit(&player, 100, 100, 50, 50, 'p');
}


void winShow(HDC dc)
{
	HDC memDC = CreateCompatibleDC(dc);
	HBITMAP memBM = CreateCompatibleBitmap(dc, rct.right - rct.left, rct.bottom - rct.top);
	SelectObject(memDC, memBM);

	static int rctSize = 100;
	int dx = (int)(cam.x) % rctSize;
	int dy = (int)(cam.y) % rctSize;

	SelectObject(memDC, GetStockObject(DC_PEN));
	SetDCPenColor(memDC, RGB(105, 105, 105));
	SelectObject(memDC, GetStockObject(DC_BRUSH));

	if (score >= 100 && score < 500)
	{
		SetDCBrushColor(memDC, RGB(240, 230, 140));
	}
	else if (score >= 500 && score < 1000)
	{
		SetDCBrushColor(memDC, RGB(255, 218, 185));
	}
	else if (score >= 1000 && score < 2000)
	{
		SetDCBrushColor(memDC, RGB(255, 239, 213));
	}
	else
	{
		SetDCBrushColor(memDC, RGB(211, 211, 211));
	}

	for (int i = -1; i < (rct.right / rctSize) + 2; i++)
	{
		for (int j = -1; j < (rct.bottom / rctSize) + 2; j++)
		{
			Rectangle(memDC, -dx + (i * rctSize), -dy + (j * rctSize),
				-dx + ((i + 1) * rctSize), -dy + ((j + 1) * rctSize));
		}
	}

	objShow(memDC, player);

	for (int i = 0; i < arrCnt; i++)
	{
		objShow(memDC, arr[i]);
	}

	char scoreText[32];
	sprintf(scoreText, "Score: %d   \0", score);
	TextOutA(memDC, 10, 10, scoreText, strlen(scoreText) + 1);

	BitBlt(dc, 0, 0, rct.right - rct.left, rct.bottom - rct.top, memDC, 0, 0, SRCCOPY);
	DeleteDC(memDC);
	DeleteObject(memBM);
}


void winMove()
{
	setCameraFocus(player);
	playerControl();
	objMove(&player);

	for (int i = 0; i < arrCnt; i++)
	{
		objMove(arr + i);
	}
}


LRESULT WINAPI wndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			break;
		}
		case WM_SIZE:
		{
			GetClientRect(hwnd, &rct);
			break;
		}
		case WM_LBUTTONDOWN:
		{
			float xMouse = LOWORD(lparam);
			float yMouse = HIWORD(lparam);
			createBullet(newObject(), xMouse + cam.x, yMouse + cam.y, 'b');
			break;
		}
		case WM_RBUTTONDOWN:
		{
			float xMouse = LOWORD(lparam);
			float yMouse = HIWORD(lparam);
			createBullet(newObject(), xMouse + cam.x, yMouse + cam.y, 'B');
			break;
		}
		default:
		{
			return DefWindowProcA(hwnd, msg, wparam, lparam);
		}
	}
}


int main()
{
	srand(7);

	WNDCLASSA win;
		memset(&win, 0, sizeof(win));
		win.lpszClassName = "win_shooter";
		win.lpfnWndProc = wndProc;
		win.hCursor = LoadCursor(NULL, IDC_CROSS);
	RegisterClassA(&win);

	HWND hwnd = CreateWindow("win_shooter", "2D Shooter", WS_OVERLAPPEDWINDOW,
		50, 50, 1280, 720, NULL, NULL, NULL, NULL);

	ShowWindow(hwnd, SW_SHOWNORMAL);

	winInit();

	MSG msg;
	HDC dc = GetDC(hwnd);

	while (1)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				break;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			if (GetKeyState(VK_ESCAPE) < 0)
			{
				break;
			}

			winMove();
			winShow(dc);
			Sleep(5);
		}
	}

	return 0;
}


