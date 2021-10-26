// If you are not compiling with the gcc option --std=gnu99, then
// uncomment the following line or you might get a compiler warning
//#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dirent.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include "InputHandler.h"
#include "BuiltIns.h"
#include "Util.h"
#include "ChildMan.h"
#include "SigMan.h"
#include "IO.h"

bool BGEnabled = true;
pid_t** bgChildren;
int bgChildIndex;
int bgChildrenSize;

/*
* Called once in main() to initialize global vars.
*/
void initChildren(void) {
	bgChildren = calloc(5, sizeof(pid_t*));
	bgChildIndex = 0;
	bgChildrenSize = 5;
}

/*
* Adds the designated pid_t to our array of pid_t's.
* Crucial for killing processes at exit.
*/
void addChildPid(pid_t childPid) {

	// Grow our array if need be
	if (bgChildIndex == bgChildrenSize - 1) {
		bgChildren = realloc(bgChildren, bgChildrenSize * 2 * sizeof(pid_t*));
		bgChildrenSize *= 2;
	}

	bgChildren[bgChildIndex] = calloc(1, sizeof(pid_t*));
	*(bgChildren[bgChildIndex]) = childPid;

	bgChildIndex++;
}

/*
* Removes the designated pid_t from our array of pid_t's.
* Ensures we do not try to kill a non-existant process at exit.
*/
void removeChildPid(pid_t childPid) {
	free(bgChildren[bgChildIndex]);
	bgChildren[bgChildIndex] = NULL;
	bgChildIndex--;

	// Shrink our array if need be
	if (bgChildIndex + 1 == bgChildrenSize/4 && bgChildrenSize > 5) {
		bgChildren = realloc(bgChildren, bgChildrenSize / 4 * sizeof(pid_t*));
		bgChildrenSize /= 4;
	}
}

/*
* Called on exit. Cleans up all children processes that may still be running.
* Not worried about removing the pids or freeing memory since if this function
*	is run, the shell must terminate anyway.
*/
void killAllChildren(void) {

	// If no children
	if (bgChildIndex == 0) {
		return;
	}

	int killIndex = 0;
	int childStatus;
	while (bgChildren[killIndex] != NULL) {
		kill(*(bgChildren[killIndex]), SIGTERM);				// Ask it to terminate
		waitpid(*(bgChildren[killIndex]), &childStatus, 0);		// Block until it terminates to ensure we clean it up
		killIndex++;
	}

}

/*
* Toggles whether foreground only mode is enabled.
* Uses write since it is reentrent safe
*/
void toggleBG(void) {
	
	if (BGEnabled) {
		write(STDOUT_FILENO, "\nEntering foreground-only mode (& is now ignored)\n", 50);		// Using write because this is called during a SIG HANDLER
		fflush(stdout);
	}
	else {
		write(STDOUT_FILENO, "\nExiting foreground-only mode\n", 30);
		fflush(stdout);
	}

	BGEnabled = !BGEnabled;
}

/*
* For forking, and exec()ing an external program.
* Blocks and checks exit statuses for foreground commands.
*/
void externalFunc(ParsedInput* pi, int* hasBGChild, int** decodedExit) {

	int childStatus;

	// If we are in foreground only mode, force parsed input to be a foreground command
	if (!BGEnabled) {
		pi->isBG = false;
	}

	// Only blocks if this is a non-built in FG command.
	// See SigMan.c for more detailed documentation.
	setSIGBlock(pi);

	pid_t childPid = forkSafe();

	if (childPid == -1) {
		perror("fork() failed!\n");
		exit(1);
	}
	else if (childPid == 0) {	// The forked child

		// Redirect the input and output now that we've forked, but before we exec()
		// Logic for if we should is handled in the functions themselves rather than here.
		redirectInput(pi);
		redirectOutput(pi);

		// Register closeFiles() as the exit function to ensure we close any open files.
		atexit(closeFiles);

		// Before we exec() we need to deal with our signal handler.
		// All child processes will ignore SIGTSTP.
		childIgnoreSIGTSTP();
		// We also want to undo our sigstopmask. Technically, we could get by without doing so
		//		but is preferrable to let SIGTSTP pass and get ignored rather than sitting in pending forever.
		unblockSIGTSTP();

		// In the case of a BG process, it should simply ignore SIGINT
		// which will be inherited, even through execvp, which means no further action required.
		if (pi->isBG) {
			execvp(pi->command, pi->args);
		}
		// But a FG command needs to listen for SIGINT and terminate on receipt.
		// Thus, before exec'ing we need to change the child's signal handler.
		else {
			stopIgnoreSIGINT();		// Return to default behaviour for SIG_INT and undoes the signal mask block.
		}

		execvp(pi->command, pi->args);
		
		// exec only returns if there is an error
		perror("Could not run program");
		exit(1);
	}
	else {						// The parent
		// We unblock SIGINT for the parent. Don't need to check if it was never blocked because it will just move on anyway.
		sigset_t emptySet;
		sigemptyset(&emptySet);
		int tempStatus = sigprocmask(SIG_UNBLOCK, &emptySet, NULL);
		if (tempStatus == -1) {
			perror("sigprocmask() UNBLOCK failed in parent after fork.");
			exit(1);
		}

		// If this is a foreground command
		if (!pi->isBG) {
			waitpid(childPid, &childStatus, 0);
			
			// Now that the FG command has ended, we unblock receipt of SIGTSTP
			unblockSIGTSTP();
			
			if (WIFEXITED(childStatus) != 0) {				  // Normal termination
				*(decodedExit[1]) = 0;						  // Set the status flag to have terminated normally
				*(decodedExit[0]) = WEXITSTATUS(childStatus); // Set the actual value of the exit status
			}
			else {											  // ABNORMAL termination
				*(decodedExit[1]) = 1;						  // Set the status flag to have terminated abnormally
				*(decodedExit[0]) = WTERMSIG(childStatus);	  // Set the actual value of the signal
				printf("terminated by signal %d\n", *(decodedExit[0]));
				fflush(stdout);
			}

		}
		// BG Command
		else {
			// We unblock SIGTSTP for the parent
			unblockSIGTSTP();

			(*hasBGChild) += 1;	// A BG process has started, so we increment our counter
			printf("The pid of the background process is: %d\n", childPid);
			fflush(stdout);

			// We only need to track the child PIDs of BG processes because we only use them for the purpose of exiting.
			addChildPid(childPid);
		}

	}
	
}

/*
* Checks if any BG processes finished, and if so prints out the exit status.
* Called by main() at the start of each loop.
*/
void bgCheck(int* hasBGChild) {
	//At the start of our loop, we will check if any of our background processes finished, and if so, print out their statuses
	// Also cleans up zombies
	pid_t childPid;
	int childExitMethod;
	int decodedExit;
	do {
		// Check if any process has completed, return immediately with 0 if none have.
		childPid = waitpid(-1, &childExitMethod, WNOHANG);

		if (childPid != 0) {
			if (WIFEXITED(childExitMethod) != 0) {		// Normal termination
				decodedExit = WEXITSTATUS(childExitMethod);
				printf("The background process with pid: %d finished: exit value %d\n", childPid, decodedExit);
			}
			else {										// ABNORMAL termination
				decodedExit = WTERMSIG(childExitMethod);
				printf("The background process with pid: %d exited due to signal: %d\n", childPid, decodedExit);
			}

			(*hasBGChild) -= 1;	// A BG process has finished, so we decrement our counter
			removeChildPid(childPid);
			if (*hasBGChild == 0) {
				break;			// If we just ended the last BG child, break the loop
			}
		}
	} while (childPid != 0);
}