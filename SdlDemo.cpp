// SdlDemo.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>
extern "C"
{
#include "SDL.h" 
}
int main()
{
	if (0 == SDL_Init(SDL_INIT_VIDEO))
	{
		std::cout << "SDL init success";
	}
    return 0;
}

