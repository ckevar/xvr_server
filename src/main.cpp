#include "UdpListener.h"

#include <cstdio>
#include <csignal>


// UdpListener udpServer("0.0.0.0", 50023);
UdpListener udpServer("0.0.0.0", 80);

void signalHandler(int signum) {
	printf("Terminating Server\n");
	udpServer.stop();
}

int main(int argc, char const *argv[]) {

	signal(SIGINT, signalHandler);

	if (udpServer.init() != 0)
		return -1;

	if(udpServer.run() < 0) return -1;

	return 0;
}
