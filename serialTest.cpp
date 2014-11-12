int main(void)
{
	int xbee = serialOpen("/dev/xbee");

	while(serialDataAvail(xbee) == 0);
	while(serialDataAvail(xbee) > 0)
	{
		serialPutchar(xbee, serialGetchar(xbee));
	}

	serialClose(xbee);
}