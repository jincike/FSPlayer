// SdlDemo.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <iostream>

#define SDL_MAIN_HANDLED
extern "C"
{
#include "SDL.h" 
//#include "SDL_main.h" 
}
int main()
{
	if (0 == SDL_Init(SDL_INIT_VIDEO))
	{
		std::cout << "SDL init success";
	}
    return 0;
}

