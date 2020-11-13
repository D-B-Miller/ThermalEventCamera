#include "teventcamera.h"

bool thresh(uint16_t c, uint16_t p)
{
	if(c>p)
	{
		return (c-p)>10;
	}
	else if(c<p)
	{
		return (p-c)>10;
	}
	else
	{
		return false;
	}
}

int main()
{
	ThermalEventCamera cam(32); // create camera
	// update comparison function
	cam.setCompare(thresh);
}
