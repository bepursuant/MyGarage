#ifndef SENSOR_H_
#define SENSOR_H_

class Sensor{
	public:
		Sensor(int sensor_type);

		void read(int& result);
		void read(char*& result);
		void read(bool& result);
}

#endif