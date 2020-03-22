#include "CoronaShoot.h"

#include <cstdio>
#include <csignal>
#include <ctime>
#include <cstdlib>

CoronaShoot coronaServer("0.0.0.0", 80);

void signalHandler(int signum) {
	printf("Terminating Server\n");
	coronaServer.stop();
}

int main(int argc, char const *argv[]) {

	signal(SIGINT, signalHandler);
	srand(time(NULL));

	if (coronaServer.init() != 0)
		return -1;

	if(coronaServer.run() < 0) return -1;

	return 0;
}
