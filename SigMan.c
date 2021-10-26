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
#include <signal.h>
#include "InputHandler.h"
#include "BuiltIns.h"
#include "Util.h"
#include "ChildMan.h"
#include "SigMan.h"

void handle_SIGTSTOP(int signo) {
	toggleBG();	// In ChildMan in order to maintain global bool of foreground-only mode state
}

void registerSigHandlers(void) {

	// Initialize SIGINT_action struct to be empty
	struct sigaction SIGINT_action = { 0 };
	// We just want to ignore SIG_IGN
	SIGINT_action.sa_handler = SIG_IGN;
	// No flags set
	SIGINT_action.sa_flags = 0;
	// Install our signal handler
	sigaction(SIGINT, &SIGINT_action, NULL);

	// Initialize SIGINT_action struct to be empty
	struct sigaction SIGTSTP_action = { 0 };
	// Setup another handler for SIGSTP
	SIGTSTP_action.sa_handler = handle_SIGTSTOP;
	// SA_RESTART to ensure we are able to write and toggle our bool
	SIGTSTP_action.sa_flags = SA_RESTART;
	// Install our signal handler
	sigaction(SIGTSTP, &SIGTSTP_action, NULL);
}

/* If we are forking a non - built in FG process, we need to temporarily block signals.
* Because it is possible (though perhaps unlikely) that we receive a SIGINT
*		after forking, but before assigning the child's event handler to respect SIGINT.
* Thus, we want to block the signals until we have re-registered the new child.
* Additionally, the parent needs to block SIGTSTP when another program has FG, and
*		then unblock it, when it is prepared to receive it.
*/
void setSIGBlock(ParsedInput* pi) {
	// This set is only relevant for the aformentied scenario of a non-built in FG process.
	sigset_t block_sig_set;
	int tempStatus = sigemptyset(&block_sig_set);
	if (tempStatus == -1) {
		perror("sigemptyset() failed");
		exit(1);
	}
	tempStatus = sigaddset(&block_sig_set, SIGINT);
	if (tempStatus == -1) {
		perror("sigaddset() failed");
		exit(1);
	}
	
	// This only applies to non-built in functions 
	if ((pi->command != "exit" && pi->command != "status" && pi->command != "cd")) {
		// We only block SIG_INT for BG processes
		if (!(pi->isBG)) {
			tempStatus = sigprocmask(SIG_BLOCK, &block_sig_set, NULL);		// Block SIG_INT for the time being
			if (tempStatus == -1) {
				perror("sigprocmask() BLOCK failed");
				exit(1);
			}
		}

		// Both FG and BG processes block SIGTSTP
		tempStatus = sigaddset(&block_sig_set, SIGTSTP);
		if (tempStatus == -1) {
			perror("sigaddset() failed");
			exit(1);
		}
		tempStatus = sigprocmask(SIG_BLOCK, &block_sig_set, NULL);		// Block SIG_INT for the time being
		if (tempStatus == -1) {
			perror("sigprocmask() BLOCK failed");
			exit(1);
		}
	}
}

/*
* Used to overwrite the previous handler which ignores SIGINT.
* This is used for when we have a child process that must now listen for SIGINT.
*/
void stopIgnoreSIGINT(void) {
	// Initialize SIGINT_action struct to be empty
	struct sigaction SIGINT_action = { 0 };
	// Don't ignore anymore.
	SIGINT_action.sa_handler = SIG_DFL;
	// No flags set
	SIGINT_action.sa_flags = 0;
	// Install our signal handler
	sigaction(SIGINT, &SIGINT_action, NULL);

	sigset_t just_sigint_set;
	int tempStatus = sigemptyset(&just_sigint_set);
	if (tempStatus == -1) {
		perror("sigemptyset() failed");
		exit(1);
	}
	tempStatus = sigaddset(&just_sigint_set, SIGINT);
	if (tempStatus == -1) {
		perror("sigaddset() failed");
		exit(1);
	}

	tempStatus = sigprocmask(SIG_UNBLOCK, &just_sigint_set, NULL);		// Unblock SIG_INT
	if (tempStatus == -1) {
		perror("Could not unblock sigprocmask() in child.");
	}
}

/*
* Setting up the handler to ignore SIGTSTP calls in the children.
*/
void childIgnoreSIGTSTP(void) {
	// Initialize SIGINT_action struct to be empty
	struct sigaction SIGTSTP_action = { 0 };
	// Don't ignore anymore.
	SIGTSTP_action.sa_handler = SIG_IGN;
	// No flags set
	SIGTSTP_action.sa_flags = 0;
	// Install our signal handler
	sigaction(SIGTSTP, &SIGTSTP_action, NULL);
}

/*
* Used to unblock SIGTSTP in the in the sigstopmask
*/
void unblockSIGTSTP(void) {
	sigset_t block_sig_set;
	sigemptyset(&block_sig_set);
	int tempStatus = sigaddset(&block_sig_set, SIGTSTP);
	if (tempStatus == -1) {
		perror("sigaddset() failed");
		exit(1);
	}
	tempStatus = sigprocmask(SIG_UNBLOCK, &block_sig_set, NULL);
	if (tempStatus == -1) {
		perror("sigprocmask() UNBLOCK failed");
		exit(1);
	}
}



