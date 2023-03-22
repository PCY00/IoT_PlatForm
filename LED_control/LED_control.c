#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define GPIO_NUMBER "14"
#define GPIO4_PATH "/sys/class/gpio/gpio14/"
#define GPIO_SYSFS "/sys/class/gpio/"

void writeGPIO(char filename[], char value[]){
	FILE* fp;
	fp = fopen(filename, "w+");
	fprintf(fp, "%s", value);
	fclose(fp);
}

int main(int argc, char* argv[]){
	if(argc != 2){
		printf("Usage is makeLEDC and one of:\n");
		printf("	setup, on, off, status, or close\n");
		printf(" e.g. makeLEDC on\n");
		return 2;
	}
	printf("Starting the makeLED program\n");

	if(strcmp(argv[1], "setup") == 0){
		printf("Setting up the LED on the GPIO\n");
		writeGPIO(GPIO_SYSFS "export", GPIO_NUMBER);
		usleep(100000);
		writeGPIO(GPIO4_PATH "direction", "out");
	}else if(strcmp(argv[1], "close") == 0){
		printf("Closing the LED on the GPIO\n");
		writeGPIO(GPIO_SYSFS "unexport", GPIO_NUMBER);
	}else if(strcmp(argv[1], "on")==0){
		printf("Turning the LED on\n");
		writeGPIO(GPIO4_PATH "value", "1");
	}else if(strcmp(argv[1], "off")==0){
		printf("Turning the LED off\n");
		writeGPIO(GPIO4_PATH "value", "0");
	}else if(strcmp(argv[1], "status") == 0){
		FILE* fp;
		char line[80], fullFilename[100];
		sprintf(fullFilename, GPIO4_PATH "\value");
		fp = fopen(fullFilename, "rt");
		while(fgets(line, 80, fp) != NULL){
			printf("The state of the LED is %s", line);
		}
		fclose(fp);
	}else{
		printf("Invalid command!\n");
	}
	printf("Finished the makeLED Program\n");
	return 0;
}
