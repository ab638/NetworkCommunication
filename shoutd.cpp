// Austin Brummett 
// CS375 Project 6

#include <iostream> // cout
#include <cstring>	// memset
#include <cstdio>	// BUFSIZ
#include <sys/types.h>
#include <sys/socket.h> // socket
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h> // struct timeval
#include <sys/ioctl.h>
#include <unistd.h>
#include <netdb.h>
#include <algorithm> // transform
#include <cstdlib>	// exit
#include <errno.h>	// errno

using namespace std;

void e_error(const char *m);
void ee_error(const char *m, int errcode);

int main(int argc, char* argv[]) {

	int server_sfd, client_sfd;
	struct sockaddr server_addr, client_addr;
	socklen_t server_len, client_len;
	struct addrinfo hints, *res;

	fd_set readfds, testfds;
	static char buffer[BUFSIZ];
	int buflen = sizeof(buffer);
	int result;
	string input;
	char name[1024];
	char port[1024];

	// getaddrinfo to ger local socket address
	// use port 0 for dynamic assignment
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if ((result = getaddrinfo(NULL, "0", &hints, &res)) == -1)
		e_error("bind");

	// Make a socket
	if ((server_sfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1)
		e_error("socket");
	cout << "Server is Accepting Clients" << endl;

	// name the socket
	if ((result = bind(server_sfd, res->ai_addr, res->ai_addrlen)) == -1)
		e_error("socket");

	// Destermine assigned Port Number
	server_len = sizeof(server_addr);
	if ((result = getsockname(server_sfd, &server_addr, &server_len)) == -1)
		e_error("getsockname");

	int flags = NI_NUMERICHOST | NI_NUMERICSERV;
	if ((result = getnameinfo(&server_addr, sizeof(server_addr), name,
	                          sizeof(name), port, sizeof(port), flags)) == -1)
		ee_error("getnameinfo", result);

	cout << "Looking on Port: " << port << endl;
	freeaddrinfo(res);

	// Create a connection queue and initialize readfds to handle
	// input from server socket fd
	listen(server_sfd, 128); // 128 is generally a system max
	FD_ZERO(&readfds);
	FD_SET(server_sfd, &readfds);

	//  Now wait for clients and requests.
	int numClients = 3; // STDIN, STDOUT, STDERR

	while (1) {

		int fd; // generic file descriptor
		int nread = 0; // number of read in bytes

		testfds = readfds;

		cout << "Server waiting" << endl;
		if ((result = select(numClients + 1, &testfds, (fd_set *)0,
		                     (fd_set *)0, (struct timeval *)0)) < 1) {
			cerr << "Server: Error in select call" << endl;
			exit(1);
		}

		// once we have activity, find which descriptor
		// on by checking FD_ISSET
		for (fd = 0; fd < numClients + 1; fd++) {
			if (FD_ISSET(fd, &testfds)) {
				// if server activity
				if (fd == server_sfd) {
					client_len = sizeof(client_addr);
					// accept connection
					client_sfd = accept(server_sfd, &client_addr, &client_len);
					// add fd to set
					FD_SET(client_sfd, &readfds);
					// increment the file descriptor position by 1
					cout << "Adding a client on fd " << client_sfd <<  endl;
					numClients++;
				}
				// Client activity
				else {
					input.clear(); // clear the input buffer
					while (true) {
						// keep on reading
						nread = read(fd, buffer, buflen);
						if (nread == 0) {
							close(fd);
							FD_CLR(fd, &readfds);
							cout << "Removing the client on fd " << fd << endl;
							numClients--;
							break;
						}
						// check for nulll terminator
						if (buffer[nread - 1] == '\0') {
							input.append(buffer, nread - 1);
							break;
						}
						// just appened it
						else
							input.append(buffer, nread);

					}
					// use transform call to toupper string
					transform(input.begin(), input.end(), input.begin(), ::toupper);

					// send string back
					write(fd, input.c_str(), input.length() + 1);
				}

			}
		}
	}
}

// error functions to make things a bit clearer
void e_error(const char *m)
{
	// general error message
	cerr << m << ": " << strerror(errno) << endl;
	exit(errno);
}

void ee_error(const char *m, int errcode)
{
	// get addrinfo error message
	cerr << m << ": " << gai_strerror(errcode) << endl;
	exit(errcode);
}