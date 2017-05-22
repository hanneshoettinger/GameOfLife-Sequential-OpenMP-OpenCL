#pragma once
#include "stdafx.h"
#include "Timer.h"

TimerFunc::TimerFunc()
{
}


TimerFunc::~TimerFunc()
{
}

void TimerFunc::startTimer()
{
	// get ticks per second
	QueryPerformanceFrequency(&frequency);

	// start timer
	QueryPerformanceCounter(&t1);
}

void TimerFunc::stopTimer(int measure)
{
	// stop timer
	QueryPerformanceCounter(&t2);

	// compute the elapsed time
	elapsedTime = (t2.QuadPart - t1.QuadPart) * 1000.0 / frequency.QuadPart;

	// if argument --measure is parsed, output elapsed time with specific time format
	if (measure == 1)
	{
		int milliseconds = (int)(elapsedTime) % 1000;
		int seconds = (int)(elapsedTime / 1000) % 60;
		int minutes = (int)(elapsedTime / (1000 * 60)) % 60;
		int hours = (int)(elapsedTime / (1000 * 60 * 60)) % 24;

		printf("%.2d:%.2d:%.2d.%.3d; ", hours, minutes, seconds, milliseconds);
	}

	elapsedTime = 0;
}