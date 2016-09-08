#ifndef _SENSOR_H
#define _SENSOR_H

class Sensor{
	public:
		Sensor(int sensor_type);

		void read(int& result);
		void read(char*& result);
		void read(bool& result);
}

#endif