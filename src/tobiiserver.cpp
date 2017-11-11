// tobiiserver.cpp : 定义控制台应用程序的入口点。
//

//#include "stdafx.h"
#include "Tobii.h"
#include <iostream>
#include "ScreenCapture.h"
#include "Timer.h"

using namespace std;
//#pragma comment(linker,"/subsystem:\"Windows\" /entry:\"mainCRTStartup\"")


int main()
{
	//打开tobii录制
	init_SendPpt();
	init_SendscreenSize();
	tobii_init();
	thread SendScreenSize(Send_ScreenSize);
	thread SendScreen(Screen);

	SendScreenSize.join();
	SendScreen.join();

	//Timer t;
	//t.StartTimer(1000, std::bind(Screen));
	while (true)
	{
		;
	}
	//system("pause");
	return 0;
}

