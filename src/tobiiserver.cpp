// tobiiserver.cpp : �������̨Ӧ�ó������ڵ㡣
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
	//��tobii¼��
	init_SendPpt();
	tobii_init();
	Timer t;
	t.StartTimer(1000, std::bind(Screen, "../"));
	while (true)
	{
		;
	}
	system("pause");
	return 0;
}

