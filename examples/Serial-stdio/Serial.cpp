#include <stdio.h>
#include "Serial.h"


int main() {
    hal::Serial0.init(9600, true);
	printf("Init!\r\n");
	
    while (true) {
		printf("Write a number:\r\n");
        int x;
		scanf("%d", &x);
		printf("readed: %d\r\n", x);
    }
}
