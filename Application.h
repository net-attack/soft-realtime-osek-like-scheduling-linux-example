#ifndef APPLICATION
#define APPLICATION

#include <stdio.h>
#include <stdint.h>

#include<time.h> //deb

/// Initialization
/** This function initializes all software component objects, normally by calling their constructors. The order is important since some objects refer to other objects  */
extern void Application_initialize();
/// Step0 - not used yet
extern void Application_step0();
/// Step1
extern void Application_step1();
/// Step2
extern void Application_step2();
/// Step3
extern void Application_step3();
/// Step4
extern void Application_step4();
/// Termination
extern void Application_terminate();

#endif /* APPLICATION */
