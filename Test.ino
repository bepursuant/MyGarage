
#DEFINE SPIFFS_FILENAME MyStruct.bin

Struct MyStruct{
	String field1 = "test";
	int field2 = 1;
}

theStruct = new MyStruct();

File configFile = SPIFFS.open(SPIFFS_FILENAME, "w+");

if (configFile)
{
	Serial.println(("Opened SPIFFS_FILENAME for WRITE.");
	Serial.printf("Start Position=%u \r\n", configFile.position());

	unsigned char * data = reinterpret_cast<unsigned char*>(theStruct); // use unsigned char, as uint8_t is not guarunteed to be same width as char...
	size_t bytes = configFile.write(data, sizeof(MyStruct)); // C++ way

	Serial.printf("END Position=%u \r\n", configFile.position());
	configFile.close();

	Serial.println("Write MyStruct from MyStruct.bin. field1=" + theStruct.field1 + ", field2=" + theStruct.field2);

} else {
	Serial.println("Failed to open SPIFFS_FILENAME for WRITE.");
}







File configFile = SPIFFS.open(SPIFFS_FILENAME, "r");
        
if (configFile)
{
	Serial.println("Opened SPIFFS_FILENAME for READ.");

	uint8_t binaryStructData[sizeof(MyStruct)];

	for(int i=0; i<sizeof(MyStruct); i++){
		binaryStructData[i] = configFile.read();
	}

	configFile.close();

	MyStruct *theStruct = (MyStruct *)binaryStructData;

	Serial.println("Read MyStruct from MyStruct.bin. field1=" + theStruct.field1 + ", field2=" + theStruct.field2);

} else {

	Serial.println("Failed to open SPIFFS_FILENAME for READ.");

}