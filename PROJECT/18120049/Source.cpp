#include <iostream>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <Windows.h>
#include <thread>
#include <string>
#include <fstream>

using namespace std;

#define MAX_SIZE_SNAKE 8*5
#define MAX_SIZE_FOOD 4
#define DEFAULT_SPEED 6
#define SPEED_INCREASE 4
#define MAX_SPEED DEFAULT_SPEED+SPEED_INCREASE*(MAX_SIZE_SNAKE-8)/(MAX_SIZE_FOOD)


//Biến toàn cục
string BODY = "18120049";					//Dãy ký tự mặc định cho thân rắn
string file = "D:\\SnakeGame\\";	//Đường dẫn mặc định cho lưu file 
POINT snake[50];	//Con rắn
POINT food[4];		// Thức ăn
int CHAR_LOCK;		//Biến xác định hướng không thể di chuyển (Ở một thời điểm có một hướng rắn không thể di chuyển)
int MOVING;			//Biến xác định hướng di chuyển của snake (Ở một thời điểm có ba hướng rắn có thể di chuyển) 
int SPEED;			// Có thể hiểu như level, level càng cao thì tốc độ càng nhanh
int HEIGH_CONSOLE;	// Độ cao của màn hình console
int WIDTH_CONSOLE;	// Độ rộng và độ cao của màn hình console
int FOOD_INDEX;		// Chỉ số food hiện hành đang có trên màn hình
int SIZE_SNAKE;		// Kích thước của snake, lúc đầu có 6 và tối đa lên tời 10
int STATE;			// Trạng thái sống hay chết của rắn
COORD SizeConsole;	//Kích thước max của console
int type;			//Loại cửa sổ console type = 1 Cửa sổ window, type =2 cửa sổ tràn màn hình
int FOOD;			//Xác định food có tồn tại hay không =0 food đã bị ăn, =1 food đã được tạo, =2 food chờ bị ăn và không tạo lại.
bool GAME_EXIT;		//Biến điều kiện kết thúc thread
bool flag = false;	//Biến đánh dấu đã thực hiện xong phím duy chuyển
int GATE = 0;		// GATE =0 chưa mở gate, gate =1 mở gate chưa qua, gate =2 mở gate đã qua.



					//overload toán tử >> để nhập mảng snake và food
istream& operator >> (istream& inDEV, POINT &p)
{
	inDEV >> p.x >> p.y;
	return inDEV;
}


//overload toán tử << để xuất mảng snake và food
ostream& operator << (ostream& outDEV, POINT p)
{
	outDEV << p.x << " " << p.y << endl;
	return outDEV;
}


//Cố định kích thước màn hình
void FixConsoleWindow()
{
	HWND consoleWindow = GetConsoleWindow();					//Lấy handle console
	LONG style = GetWindowLong(consoleWindow, GWL_STYLE);		//Lấy các thuộc tính hiện tại của console
	style = style & ~(WS_MAXIMIZEBOX) & ~(WS_THICKFRAME);		//Bỏ ô phóng to và bỏ viền cửa sổ console
	SetWindowLong(consoleWindow, GWL_STYLE, style);				//Thiết lập thuộc tính đã điều chỉnh
}


//Ẩn con trỏ
void Nocursortype()
{
	CONSOLE_CURSOR_INFO Info;
	Info.bVisible = FALSE;	//Ẩn con trỏ
	Info.dwSize = 20;
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &Info);	//Thiết lập thuộc tính cho con trỏ theo biến info
}


//Đổi màu kí tự
void ChangeColor(int i)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), i);	//Đổi màu ký tự theo mã i
}


//Hàm di chuyển con trỏ đến tọa độ (x,y)
void GotoXY(int x, int y)
{
	COORD coord;
	coord.X = x;
	coord.Y = y;
	int exit = SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}


//Hàm kiểm tra tọa độ điểm và mảng rắn có trùng nhau không. Trùng == false
bool IsValid(int x, int y)
{
	for (int i = 0; i < SIZE_SNAKE; i++)
	{
		if (snake[i].x == x && snake[i].y == y)
		{
			return false;
		}
	}
	return true;
}


//Hàm tạo mảng thức ăn
void GenerateFood()
{
	int x, y;
	srand(time(NULL));
	for (int i = 0; i < MAX_SIZE_FOOD; i++)
	{
		do
		{
			x = rand() % (WIDTH_CONSOLE - 1) + 1;
			y = rand() % (HEIGH_CONSOLE - 1) + 1;
		} while (IsValid(x, y) == false);
		food[i] = { x,y };
	}
}


//Hàm khởi tạo các giá trị mặc định
void ResetData()
{
	//Khởi tạo các giá trị toàn cục
	CHAR_LOCK = 'A';
	MOVING = 'D';
	SPEED = DEFAULT_SPEED;
	FOOD_INDEX = 0;
	WIDTH_CONSOLE = 60;
	HEIGH_CONSOLE = 27;
	SIZE_SNAKE = 8;
	FOOD = 0;
	GATE = 0;
	//Khởi tạo giá trị mặc định cho snake
	snake[0] = { 3, 15 };
	snake[1] = { 4, 15 };
	snake[2] = { 5, 15 };
	snake[3] = { 6, 15 };
	snake[4] = { 7, 15 };
	snake[5] = { 8, 15 };
	snake[6] = { 9, 15 };
	snake[7] = { 10, 15 };
	GenerateFood();//Tạo mảng thức ăn food
}


//Hàm vẽ khung game
void DrawBoard(int x, int y, int width, int height, int curPosX = 0, int curPosY = 0)
{
	ChangeColor(15);
	GotoXY(x, y);
	char top = 220;
	char bottom = 223;
	char left = 222;
	char right = 221;
	printf(" ");
	for (int i = 1; i < width; i++)
	{
		printf("%c", top);
	}
	printf(" ");
	GotoXY(x, height + y);
	printf(" ");
	for (int i = 1; i < width; i++)
	{
		printf("%c", bottom);
	}
	printf(" ");
	for (int i = y + 1; i < height + y; i++)
	{
		GotoXY(x, i); printf("%c", left);
		GotoXY(x + width, i); printf("%c", right);
	}
	GotoXY(curPosX, curPosY);
}


//Mở cổng
void ShowTheGate()
{
	ChangeColor(11);
	CONSOLE_SCREEN_BUFFER_INFO csbInfo;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbInfo);  //Lấy vị trí rắn hiện tại
	GotoXY(WIDTH_CONSOLE, HEIGH_CONSOLE / 2);								//Mở cổng
	printf("%c", ' ');
	GotoXY(WIDTH_CONSOLE, HEIGH_CONSOLE / 2 - 1);
	printf("%c%c", 223, 223);
	GotoXY(WIDTH_CONSOLE, HEIGH_CONSOLE / 2 + 1);
	printf("%c%c", 220, 220);
	GotoXY(csbInfo.dwCursorPosition.X, csbInfo.dwCursorPosition.Y);			//Phục hồi con trỏ tại vị trí rắn
	GATE = 1;
}


//Đóng cổng
void CloseTheGate()
{
	ChangeColor(15);
	GotoXY(WIDTH_CONSOLE, HEIGH_CONSOLE / 2);
	printf("%c ", 221);
	GotoXY(WIDTH_CONSOLE, HEIGH_CONSOLE / 2 - 1);
	printf("%c ", 221);
	GotoXY(WIDTH_CONSOLE, HEIGH_CONSOLE / 2 + 1);
	printf("%c ", 221);
	GATE = 2;
}


//Xử lí cho rắn chạy qua cổng
void MoveThroughGate()
{
	//Khi rắn chạy qua cổng sẽ biến mất dần
	int tempSN = SIZE_SNAKE;
	while (tempSN > 0)
	{
		for (int i = 0; i < SIZE_SNAKE - 1; i++)
		{
			snake[i].x = snake[i + 1].x;
			snake[i].y = snake[i + 1].y;
		}
		snake[SIZE_SNAKE - 1].x++;
		for (int i = 0; i < tempSN; i++)
		{
			ChangeColor(15);
			GotoXY(snake[i].x, snake[i].y);
			printf("%c", BODY[i % 8]);
		}
		Sleep(1000 / SPEED);
		for (int i = 0; i < tempSN; i++)
		{
			ChangeColor(0);
			GotoXY(snake[i].x, snake[i].y);
			printf(" ");
		}
		tempSN--;
	}
}


//Vẽ khung và cho Thread chạy
void StartGame()
{
	system("cls");
	DrawBoard(0, 0, WIDTH_CONSOLE, HEIGH_CONSOLE);	// Vẽ màn hình game
	STATE = 1;										//Bắt đầu cho Thread chạy
}


//Hàm dừng game
void PauseGame(HANDLE t)
{
	SuspendThread(t);
	flag = false;
}


//Hàm lưu game vào file
void SaveGame(HANDLE handle_t1)
{
	ChangeColor(15);
	string str, save_file = file;
	PauseGame(handle_t1);
	CreateDirectoryA(file.c_str(), NULL);			//Tạo thư mục lưu game

													//Nhập tên file
	GotoXY(WIDTH_CONSOLE + ((SizeConsole.X - WIDTH_CONSOLE) / 2 - 14), HEIGH_CONSOLE / 2);
	printf("Nhap ten file de luu du lieu");
	GotoXY(WIDTH_CONSOLE + ((SizeConsole.X - WIDTH_CONSOLE) / 2 - 9), HEIGH_CONSOLE / 2 + 1);
	printf("(Toi da 10  ky tu)");
	GotoXY(WIDTH_CONSOLE + ((SizeConsole.X - WIDTH_CONSOLE) / 2 - 5), HEIGH_CONSOLE / 2 + 2);
	printf(">>");
	cin >> str;
	save_file.append(str);
	ofstream save(save_file);
	if (save)
	{
		save << CHAR_LOCK << endl;
		save << MOVING << endl;
		save << SPEED << endl;
		save << FOOD_INDEX << endl;
		save << WIDTH_CONSOLE << endl;
		save << HEIGH_CONSOLE << endl;
		save << SIZE_SNAKE << endl;
		save << GATE << endl;
		for (int i = 0; i < SIZE_SNAKE; i++)
		{
			save << snake[i];
		}
		for (int i = FOOD_INDEX; i < MAX_SIZE_FOOD; i++)
		{
			save << food[i];
		}
		GotoXY(WIDTH_CONSOLE + ((SizeConsole.X - WIDTH_CONSOLE) / 2 - 24), HEIGH_CONSOLE / 2 + 3);
		printf("Da luu thanh cong. Chon phim bat ky de tiep tuc");
		_getch();
	}
	else
	{
		GotoXY(WIDTH_CONSOLE + ((SizeConsole.X - WIDTH_CONSOLE) / 2 - 22), HEIGH_CONSOLE / 2 + 3);
		printf("Luu khong thanh cong. Hay thu ten file khac");
		GotoXY(WIDTH_CONSOLE + ((SizeConsole.X - WIDTH_CONSOLE) / 2 - 14), HEIGH_CONSOLE / 2 + 4);
		printf("Chon phim bat ky de tiep tuc");
		_kbhit();
	}
	StartGame();
	if (GATE != 0)
		ShowTheGate();
	ResumeThread(handle_t1);
	FOOD = 0;
	flag = false;
}


//Hàm cập nhật dữ liệu toàn cục
void Eat()
{
	snake[SIZE_SNAKE] = food[FOOD_INDEX];
	if (FOOD_INDEX == MAX_SIZE_FOOD - 1)		//Ăn hết thức ăn sẽ mở cổng lên cấp, đồng thời khởi tạo lại mảng thức ăn
	{
		if (GATE == 0)
		{
			ShowTheGate();
			GATE = 1;
		}
		FOOD_INDEX = 0;
		GenerateFood();
	}
	else
	{
		FOOD_INDEX++;

		//Neu thuc an tiep theo nam trung vao vi tri snake thi tao vi tri moi
		while (IsValid(food[FOOD_INDEX].x, food[FOOD_INDEX].y) == false)
		{
			food[FOOD_INDEX].x = rand() % (WIDTH_CONSOLE - 1) + 1;
			food[FOOD_INDEX].y = rand() % (HEIGH_CONSOLE - 1) + 1;
		}
	}
	SIZE_SNAKE++;
}


//Hàm vẽ rắn và thức ăn màn hình
void DrawSnakeAndFood(char* str)
{

	//Xử lí tránh in lặp đi lặp lại một thức ăn
	if ((GATE == 0) && (FOOD == 0) && (str == " "))
	{
		ChangeColor(0);
		GotoXY(food[FOOD_INDEX].x, food[FOOD_INDEX].y);
		printf(str);
		FOOD = 1;
	}
	if ((FOOD == 1) && (str == "O"))
	{
		ChangeColor(15);
		GotoXY(food[FOOD_INDEX].x, food[FOOD_INDEX].y);
		printf(str);
		FOOD = 2;
	}

	//Xử lí in thân rắn là mssv
	for (int i = 0; i < SIZE_SNAKE; i++)
	{
		GotoXY(snake[i].x, snake[i].y);
		if (str == "O")
		{
			ChangeColor(15);
			printf("%c", BODY[i % 8]);
		}
		else
		{
			ChangeColor(0);
			printf(str);
		}
	}
}


//Hàm xử lý khi snake chết
void ProcessDead()
{
	// Khi chết rắn nhấp nháy 5 lần
	for (int i = 0; i < 5; i++)
	{
		//ChangeColor(112);
		DrawSnakeAndFood(" ");
		Sleep(250);
		DrawSnakeAndFood("O");
		Sleep(250);
	}

	// Và xuất hiện thông báo chết, hỏi tiếp tục hay thoát game
	ChangeColor(12);
	STATE = 0;
	FOOD = 0;
	GAME_EXIT = true;
	int x = WIDTH_CONSOLE + ((SizeConsole.X - WIDTH_CONSOLE) / 2);
	int y = HEIGH_CONSOLE / 2;
	GotoXY(x - 2, y - 2);
	printf("DEAD");
	GotoXY(x - 12, y);
	printf("Tro lai menu nhan phim Y");
	GotoXY(x - 11, y + 2);
	printf("De thoat nhan phim ESC");
}


//Hàm xử lí lên cấp
void Levelup()
{

	if (SPEED + SPEED_INCREASE >= MAX_SPEED)	// Khi lên hết cấp, trở về độ dài và tốc độ mặc định
	{
		SIZE_SNAKE = 8;
		SPEED = DEFAULT_SPEED;
	}
	else
		SPEED += SPEED_INCREASE;				//Chưa hết cấp thì tăng tốc mỗi lần lên cấp

												//Mỗi lần lên cấp, rắn xuất hiện trên màn hình và chờ nhấn phím để duy chuyển
	for (int i = 0; i < SIZE_SNAKE; i++)
	{
		snake[i] = { i + 3,15 };
	}
	snake[SIZE_SNAKE] = { 0,0 };
	GATE = 0;
	GotoXY(snake[0].x, snake[0].y);
	DrawSnakeAndFood("O");
	FOOD = 1;
	_kbhit();
}


//Hàm di chuyển ứng với phím D
void MoveRight()
{
	/*
	Xử lí chết khi rắn chạm vào bản thân hoặc vào tường trong khi đang chơi một cấp.
	Ngoại trừ trường hợp đã hết cấp mở cổng, rắn chỉ có thể đi qua tường tại cổng
	*/
	if (((GATE == 0 || ((GATE == 1) && (snake[SIZE_SNAKE - 1].y != HEIGH_CONSOLE / 2))) && snake[SIZE_SNAKE - 1].x + 1 == WIDTH_CONSOLE) || (!IsValid(snake[SIZE_SNAKE - 1].x + 1, snake[SIZE_SNAKE - 1].y)))
	{
		ProcessDead();
	}
	else
	{
		if (snake[SIZE_SNAKE - 1].x + 1 == food[FOOD_INDEX].x && snake[SIZE_SNAKE - 1].y == food[FOOD_INDEX].y)
		{
			Eat();
			FOOD = 0;
		}
		if ((snake[SIZE_SNAKE - 1].x + 1 != WIDTH_CONSOLE) || (GATE == 1))
		{
			for (int i = 0; i < SIZE_SNAKE - 1; i++)
			{
				snake[i].x = snake[i + 1].x;
				snake[i].y = snake[i + 1].y;
			}
			snake[SIZE_SNAKE - 1].x++;
		}
	}
}


//Hàm di chuyển ứng với phím A
void MoveLeft()
{
	// Xử lí chết khi rắn chạm vào bản thân hoặc vào tường
	if ((snake[SIZE_SNAKE - 1].x - 1 == 0) || (!IsValid(snake[SIZE_SNAKE - 1].x - 1, snake[SIZE_SNAKE - 1].y)))
	{
		ProcessDead();

	}
	else
	{
		if (snake[SIZE_SNAKE - 1].x - 1 == food[FOOD_INDEX].x && snake[SIZE_SNAKE - 1].y == food[FOOD_INDEX].y)
		{
			Eat();
			FOOD = 0;
		}
		if (snake[SIZE_SNAKE - 1].x - 1 != 0)
		{
			for (int i = 0; i < SIZE_SNAKE - 1; i++)
			{
				snake[i].x = snake[i + 1].x;
				snake[i].y = snake[i + 1].y;
			}
			snake[SIZE_SNAKE - 1].x--;
		}
	}
}


//Hàm di chuyển ứng với phím D
void MoveDown()
{
	//Xử lí chết khi rắn chạm vào bản thân hoặc vào tường
	if ((snake[SIZE_SNAKE - 1].y + 1 == HEIGH_CONSOLE) || (!IsValid(snake[SIZE_SNAKE - 1].x, snake[SIZE_SNAKE - 1].y + 1)))
	{
		ProcessDead();
	}
	else
	{
		if (snake[SIZE_SNAKE - 1].x == food[FOOD_INDEX].x && snake[SIZE_SNAKE - 1].y + 1 == food[FOOD_INDEX].y)
		{
			Eat();
			FOOD = 0;
		}
		if (snake[SIZE_SNAKE - 1].y + 1 != HEIGH_CONSOLE)
		{
			for (int i = 0; i < SIZE_SNAKE - 1; i++)
			{
				snake[i].x = snake[i + 1].x;
				snake[i].y = snake[i + 1].y;
			}
			snake[SIZE_SNAKE - 1].y++;

		}
	}
}


//Hàm di chuyển ứng với phím W
void MoveUp()
{
	//Xử lí chết khi rắn chạm vào bản thân hoặc vào tường
	if ((snake[SIZE_SNAKE - 1].y - 1 == 0) || (!IsValid(snake[SIZE_SNAKE - 1].x, snake[SIZE_SNAKE - 1].y - 1)))
	{
		ProcessDead();
	}
	else
	{
		if (snake[SIZE_SNAKE - 1].x == food[FOOD_INDEX].x && snake[SIZE_SNAKE - 1].y - 1 == food[FOOD_INDEX].y)
		{
			Eat();
			FOOD = 0;
		}
		if (snake[SIZE_SNAKE - 1].y - 1 != 0)
		{
			for (int i = 0; i < SIZE_SNAKE - 1; i++)
			{
				snake[i].x = snake[i + 1].x;
				snake[i].y = snake[i + 1].y;
			}
			snake[SIZE_SNAKE - 1].y--;

		}
	}
}


//Thủ tục cho thread
void ThreadFunc()
{
	while (1)
	{
		if (STATE == 1)			//Nếu vẫn còn snake vẫn còn sống
		{
			DrawSnakeAndFood(" ");
			if (GATE == 1 && snake[SIZE_SNAKE - 1].x >= WIDTH_CONSOLE && snake[SIZE_SNAKE - 1].y == HEIGH_CONSOLE / 2)    //Cho rắn chạy qua và xử lí lên cấp
			{
				flag = true;
				MoveThroughGate();
				CloseTheGate();
				Levelup();
				continue;
			}
			switch (MOVING)
			{
			case 'A':
				MoveLeft();
				break;
			case 'D':
				MoveRight();
				break;
			case 'W':
				MoveUp();
				break;
			case 'S':
				MoveDown();
				break;
			}
			if (GAME_EXIT)
				return;
			flag = false;
			DrawSnakeAndFood("O");

			//Hàm ngủ theo tốc độ SPEED nhưng cần điều chỉnh để trông cân bằng hơn
			switch (MOVING)
			{
			case 'A':case 'D':
				Sleep(1000 / SPEED);
				break;
			case 'W':case 'S':
				Sleep(1000 / (4 * SPEED / 5));
				break;
			}
		}

	}

}


//Chế độ đầy màn hình
void FullScreenMode()
{
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleDisplayMode(handle, CONSOLE_FULLSCREEN_MODE, 0);
	CONSOLE_SCREEN_BUFFER_INFO csbInfo;
	GetConsoleScreenBufferInfo(handle, &csbInfo);
	SizeConsole.X = csbInfo.srWindow.Right - csbInfo.srWindow.Left + 1;
	SizeConsole.Y = csbInfo.srWindow.Bottom - csbInfo.srWindow.Top + 1;
}


//Chế độ cửa sổ
void NormalScreenMode()
{
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleDisplayMode(handle, CONSOLE_WINDOWED_MODE, 0);
	CONSOLE_SCREEN_BUFFER_INFO csbInfo;
	GetConsoleScreenBufferInfo(handle, &csbInfo);
	SizeConsole.X = csbInfo.srWindow.Right - csbInfo.srWindow.Left + 1;
	SizeConsole.Y = csbInfo.srWindow.Bottom - csbInfo.srWindow.Top + 1;
}


//Tải game đã lưu
void LoadGame(string save_file, int &exitcode)
{
	ChangeColor(15);
	string str;

	//Nhập tên file để tải dữ liệu game đã lưu
	char exit = 'Y';
	do
	{
		printf("\nNhap ten file: ");
		cin >> str;
		save_file.append(str);
		ifstream save(save_file);
		str.erase();
		if (!save)
		{
			printf("Khong tim thay file! Muon nhap lai ten file khong? (Co chon Y, Khong chon phim bat ky)\n");
			exit = toupper(_getch());

			if (exit == 'Y')
				continue;
			else
				break;
		}
		else
		{
			save >> CHAR_LOCK;
			save >> MOVING;
			save >> SPEED;
			save >> FOOD_INDEX;
			save >> WIDTH_CONSOLE;
			save >> HEIGH_CONSOLE;
			save >> SIZE_SNAKE;
			save >> GATE;
			for (int i = 0; i<SIZE_SNAKE; i++)
				save >> snake[i];
			for (int i = FOOD_INDEX; i< MAX_SIZE_FOOD; i++)
				save >> food[i];
			FOOD = 0;
			exitcode = 0;
			break;
		}
	} while (1);
	if (exit != 'Y')
	{
		printf("Ban muon choi game moi khong? (Co chon Y, Khong chon phim bat ky)\n");
		exit = toupper(_getch());
		if (exit == 'Y')
		{
			ResetData();
			exitcode = 0;    // Tiếp tục chương  trình
		}
		else
			exitcode = 1;    // exitcode 1 đánh dấu thoát khỏi chương trình
		return;
	}
}


//Bắt đầu chơi game
int PlayGame()
{
	int temp = 'D';				// Biến lấy giá trị phím nhập từ bàn phím, P tạm dừng, L lưu dữ liệu, esc thoát chương trình , Y Tiếp tục game, và các hướng di chuyển WASD
	FixConsoleWindow();			//Cố định kích thước màn hình
	Nocursortype();				//Ẩn con trỏ
	StartGame();				//Vẽ khung và cho thread chạy
	if (GATE != 0)
		ShowTheGate();
	DrawSnakeAndFood("O");
	thread t1(ThreadFunc);				//Tạo thread cho snake
	HANDLE handle_t1;					//Handle của thread
	handle_t1 = t1.native_handle();		//Lay handle cua thread
	PauseGame(handle_t1);
	while (1)
	{
		if ((!flag) && (!GAME_EXIT))
		{
			temp = toupper(_getch());
			flag = true;
		}
		if (STATE == 1)
		{
			if (temp == 'P')				//Tạm dừng thread
				PauseGame(handle_t1);
			else
				if (temp == 'L')			//Lưu Game vào file
					SaveGame(handle_t1);
				else
					if (temp == 27)			//Thoát game
					{
						ResumeThread(handle_t1);
						GAME_EXIT = true;
						t1.join();
						t1.~thread();
						return 1;
					}
					else
					{
						ResumeThread(handle_t1);		//Tiếp tục thread
						if ((temp != CHAR_LOCK) && (temp == 'D' || temp == 'A' || temp == 'W' || temp == 'S'))
						{
							if (temp == 'D')
								CHAR_LOCK = 'A';
							else
								if (temp == 'W')
									CHAR_LOCK = 'S';
								else
									if (temp == 'S')
										CHAR_LOCK = 'W';
									else
										CHAR_LOCK = 'D';
							MOVING = temp;
						}
					}
		}
		else
		{
			t1.join();
			t1.~thread();		//Hủy thread sau khi rắn chết
			do
			{
				if (kbhit())
					temp = toupper(_getch());
			} while ((temp != 'Y') && (temp != 27));
			if (temp == 'Y')	//Trở về menu game
			{
				system("cls");
				flag = false;
				GAME_EXIT = false;
				return 0;		//Hiện menu và chơi lại từ đầu
			}
			else
			{
				return 1;		//Thoát game
			}
		}
	}
}


//Menu chính
void MainMenu()
{
	system("cls");
	ChangeColor(14);
	GotoXY(SizeConsole.X / 2 - 8, SizeConsole.Y / 2 - 4);
	printf("Choi game moi(N)\n");
	GotoXY(SizeConsole.X / 2 - 11, SizeConsole.Y / 2 - 3);
	printf("Tai lai game da luu(T)\n");
	GotoXY(SizeConsole.X / 2 - 5, SizeConsole.Y / 2 - 2);
	printf("Cat dat(C)\n");
	GotoXY(SizeConsole.X / 2 - 7, SizeConsole.Y / 2 - 1);
	printf("Thoat game(ESC)\n");
	GotoXY(SizeConsole.X / 2 - 14, SizeConsole.Y / 2 - 1);
	printf(">");
	GotoXY(SizeConsole.X / 2 + 14, SizeConsole.Y / 2 - 1);
	printf("<");
}


//Menu Load game
void LoadGameMenu(int &exitcode)
{
	string str, save_file = file;
	ChangeColor(15);
	GotoXY(SizeConsole.X / 2 + 14, SizeConsole.Y / 2);
	LoadGame(save_file, exitcode);
}


//Menu cài đặt
void SettingMenu()
{
	system("cls");
	if (type == 1)
		ChangeColor(6);
	else
		ChangeColor(14);
	GotoXY(SizeConsole.X / 2 - 10, SizeConsole.Y / 2 - 3);
	printf("Che do cua so\n");
	if (type == 1)
		ChangeColor(14);
	else
		ChangeColor(6);
	GotoXY(SizeConsole.X / 2 - 10, SizeConsole.Y / 2 - 2);
	printf("Che do day man hinh\n");
	GotoXY(SizeConsole.X / 2 - 10, SizeConsole.Y / 2 - 1);
	ChangeColor(14);
	printf("Tro ve\n");
	ChangeColor(14);
	if (type == 1)
	{
		GotoXY(SizeConsole.X / 2 - 14, SizeConsole.Y / 2 - 2);
		printf(">");
		GotoXY(SizeConsole.X / 2 + 14, SizeConsole.Y / 2 - 2);
		printf("<");
	}
	else
	{
		GotoXY(SizeConsole.X / 2 - 14, SizeConsole.Y / 2 - 3);
		printf(">");
		GotoXY(SizeConsole.X / 2 + 14, SizeConsole.Y / 2 - 3);
		printf("<");
	}
}


//Tạo menu
void ShowMenu()
{
	GAME_EXIT = false;

	FixConsoleWindow();			//Cố định kích thước màn hình
	Nocursortype();				//Ẩn con trỏ
	CONSOLE_SCREEN_BUFFER_INFO csbInfo;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbInfo);
	SizeConsole.X = csbInfo.srWindow.Right - csbInfo.srWindow.Left + 1;
	SizeConsole.Y = csbInfo.srWindow.Bottom - csbInfo.srWindow.Top + 1;
	type = 1;
	MainMenu();

	int sub = 0;			//biến đại diện cho menu con.
	int exitcode = 0;
	while (exitcode == 0)	//chạy đến khi nào có lỗi hoặc muốn dừng game
	{
		COORD Pos;
		char temp = ' ';
		Pos = { SizeConsole.X / 2 - 14, SizeConsole.Y / 2 - 1 };

		while (1)			//Lựa chọn trong menu
		{
			if (kbhit())
				temp = toupper(getch());
			switch (sub)
			{
			case 0:		//Menu chính
			{
				switch (temp)	//Di chuyển trong menu
				{
				case 'W':	//Di chuyển lên
				{
					if (Pos.Y > SizeConsole.Y / 2 - 4)
					{
						ChangeColor(0);
						GotoXY(Pos.X, Pos.Y);
						printf(" ");
						GotoXY(Pos.X + 28, Pos.Y);
						printf(" ");
						Pos.Y--;
						ChangeColor(14);
						GotoXY(Pos.X, Pos.Y);
						printf(">");
						GotoXY(Pos.X + 28, Pos.Y);
						printf("<");
					}
					break;
				}
				case 'S':	//Di chuyển xuống
				{
					if (Pos.Y < SizeConsole.Y / 2 - 1)
					{
						ChangeColor(0);
						GotoXY(Pos.X, Pos.Y);
						printf(" ");
						GotoXY(Pos.X + 28, Pos.Y);
						printf(" ");
						Pos.Y++;
						ChangeColor(14);
						GotoXY(Pos.X, Pos.Y);
						printf(">");
						GotoXY(Pos.X + 28, Pos.Y);
						printf("<");
					}
					break;
				}
				case 'N':	//Chơi game mới
				{
					ChangeColor(15);
					ResetData();
					exitcode = PlayGame();
					if (exitcode == 1)
						return;
					MainMenu();
					Pos = { SizeConsole.X / 2 - 14, SizeConsole.Y / 2 - 1 };
					break;
				}
				case 'T':	//Tải game đã lưu
				{
					LoadGameMenu(exitcode);
					if (exitcode == 1)
						return;
					exitcode = PlayGame();
					if (exitcode == 1)
						return;
					MainMenu();
					Pos = { SizeConsole.X / 2 - 14, SizeConsole.Y / 2 - 1 };
					break;
				}
				case 'C':	//Cài đặt cửa sổ
				{
					SettingMenu();
					sub = 1;
					break;
				}
				case 27:	//Thoát game
				{
					return;
				}
				case 13:	//xử lí các lựa chọn khi nhất enter
				{
					switch (Pos.Y - SizeConsole.Y / 2)
					{
					case -4:
					{
						ChangeColor(15);
						ResetData();
						exitcode = PlayGame();
						if (exitcode == 1)
							return;
						MainMenu();
						Pos = { SizeConsole.X / 2 - 14, SizeConsole.Y / 2 - 1 };
						break;
					}
					case -3:
					{
						LoadGameMenu(exitcode);
						if (exitcode == 1)
							return;
						exitcode = PlayGame();
						if (exitcode == 1)
							return;
						MainMenu();
						Pos = { SizeConsole.X / 2 - 14, SizeConsole.Y / 2 - 1 };
						break;
					}
					case -2:
					{
						SettingMenu();
						sub = 1;
						if (type == 1)
							Pos = { SizeConsole.X / 2 - 14, SizeConsole.Y / 2 - 2 };
						else
							Pos = { SizeConsole.X / 2 - 14, SizeConsole.Y / 2 - 3 };
						break;
					}
					case -1:
					{
						return;
					}
					}
					break;
				}
				}
				break;
			}
			case 1:		//Menu Phụ
			{
				switch (temp)
				{
				case 'W':
				{
					if (Pos.Y > SizeConsole.Y / 2 - 3)
					{
						ChangeColor(0);
						GotoXY(Pos.X, Pos.Y);
						printf(" ");
						GotoXY(Pos.X + 28, Pos.Y);
						printf(" ");
						Pos.Y--;
						ChangeColor(14);
						GotoXY(Pos.X, Pos.Y);
						printf(">");
						GotoXY(Pos.X + 28, Pos.Y);
						printf("<");
					}
					break;
				}
				case 'S':
				{
					if (Pos.Y < SizeConsole.Y / 2 - 1)
					{
						ChangeColor(0);
						GotoXY(Pos.X, Pos.Y);
						printf(" ");
						GotoXY(Pos.X + 28, Pos.Y);
						printf(" ");
						Pos.Y++;
						ChangeColor(14);
						GotoXY(Pos.X, Pos.Y);
						printf(">");
						GotoXY(Pos.X + 28, Pos.Y);
						printf("<");
					}
					break;
				}
				case 13:
				{
					switch (Pos.Y - SizeConsole.Y / 2)
					{
					case -3:
					{
						if (type == 2)
						{
							NormalScreenMode();
							type = 1;
							MainMenu();
							sub = 0;
							Pos = { SizeConsole.X / 2 - 14, SizeConsole.Y / 2 - 1 };

						}
						break;
					}
					case -2:
					{
						if (type == 1)
						{
							FullScreenMode();
							type = 2;
							MainMenu();
							sub = 0;
							Pos = { SizeConsole.X / 2 - 14, SizeConsole.Y / 2 - 1 };

						}
						break;
					}
					case -1:
					{
						MainMenu();
						sub = 0;
						Pos = { SizeConsole.X / 2 - 14, SizeConsole.Y / 2 - 1 };
						break;
					}
					}
					break;
				}
				}
				break;
			}
			}
			temp = ' ';
		}

	}
}


//Hàm main
void main()
{
	ShowMenu();
}