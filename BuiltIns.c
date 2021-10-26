// If you are not compiling with the gcc option --std=gnu99, then
// uncomment the following line or you might get a compiler warning
//#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include "InputHandler.h"
#include "BuiltIns.h"
#include "Util.h"
#include "ChildMan.h"
#include "SigMan.h"

void shellExit(void) {
    utilPrintf("Exiting!\n");
    killAllChildren();
    exit(0);
}

void shellCD(char* filePath) {

    // DEBUG - RMOVE
    debugPWD();

    // If cd is entered without an argument, it should change
    //      to the directory as saved in the environment variable: "HOME"
    if (filePath == NULL) {
        char* homeEnvVar = getenv("HOME");
        chdir(homeEnvVar);
    }
    // Otherwise simply change directory to the relative/absolute path given.
    else {
        chdir(filePath);
    }

    // DEBUG - RMOVE
    debugPWD();
}

void shellStatus(int** fgExitStatus) {
    
    if (*(fgExitStatus[1]) == 0) {      // Normal Termination
        int exitStatusVal = *(fgExitStatus[0]);
        printf("The most recent foreground process exited normally with the exit status: %d\n", exitStatusVal);
    }
    else {                              // Abnormal Termination
        int exitStatusVal = *(fgExitStatus[0]);
        printf("The most recent foreground process terminated via signal %d\n", exitStatusVal);
    }

}

