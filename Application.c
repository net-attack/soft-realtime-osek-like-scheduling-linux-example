#include "Application.h"

/* SIGNALS */
uint16_t signal1;
uint16_t signal2;
uint16_t signal3;
uint16_t signal4;

void Application_step0() /* Sample time: [0.001s, 0.0s] */
{
	printf("Start Step 0\n");
	printf("End Step 0\n");

}

void Application_step1() /* Sample time: [0.01s, 0.0s] */
{
	printf("Start Step 1\n");
	printf("End Step 1\n");
}

void Application_step2() /* Sample time: [0.05s, 0.0s] */
{
	printf("Start Step 2\n");
	printf("End Step 2\n");
}

void Application_step3() /* Sample time: [0.1s, 0.0s] */
{
	printf("Start Step 3\n");
	printf("End Step 3\n");
}

void Application_step4() /* Sample time: [0.25s, 0.0s] */
{
	printf("Start Step 4\n");
	printf("End Step 4\n");
}

void Application_initialize() {
	printf("Start Init\n");
	signal1 = 0;
	signal2 = 0;
	signal3 = 0;
	signal4 = 0;
	printf("End Init\n");
}

void Application_terminate(void) {
	printf("Start Terminate\n");
	printf("End Terminate\n");
}
