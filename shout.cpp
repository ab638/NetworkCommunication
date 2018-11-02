// Austin Brummett 
// CS375 Project 6

#include <cstdlib>	// exit
#include <iostream>	// cout 
#include <fstream>	// fstream
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <streambuf> // istreambuf

using namespace std;
void run(int sockfd, int nread);
int main(int argc, char *argv[]) {

	int sockfd;
	int len;
	struct addrinfo *r;
	int nread = 0, result;

	if (argc != 3) {
		cerr << "Syntax: shout hostname port" << endl;
		exit(1);
	}

	getaddrinfo(argv[1], argv[2], NULL, &r);
	// create a socket for the client
	sockfd = socket(r->ai_family, r->ai_socktype, r->ai_protocol);
	// Connect this socket to the server socket
	if ((result = connect(sockfd, r->ai_addr, r->ai_addrlen)) == -1) {
		cerr << "Client: connect failed" << endl;
		exit(1);
	}
	string inName, outName;	// fule names
	ifstream inFile;		// in file stream
	ofstream outFile;		// output file stream
	static char buffer[BUFSIZ];
	while (true) {
		inName.clear();
		outName.clear();
		buffer[0] = '0';
		cout << "Enter input and output file names: ";
		cin >> inName >> outName;
		if (cin.eof()) {
			cout << "^D" << endl;
			break;
		}
		inFile.open(inName.c_str()); 		// input file stream

		// check if the file exists
		while (!inFile.is_open()) {
			cout << "File:" << inName << "does not exist" << endl;
			cout << "Enter an existing file name: ";
			cin >> inName;
			inFile.open(inName.c_str());
		}
		outFile.open(outName.c_str());		// output file stream

		// store input file in the string using istreambuffer iterator
		// and string constructor
		string sent((istreambuf_iterator<char>(inFile)),
		            istreambuf_iterator<char>());
		sent.at(sent.length() - 1) = '\0';
		int fileLength = sent.length() + 1;;

		write(sockfd, sent.c_str(), fileLength);
		while (true) {
			nread = read(sockfd, buffer, BUFSIZ);
			outFile << buffer;
			// runs through and checks if it
			if (buffer[nread - 1] == '\0')
				break;
		}
		inFile.close();
		outFile.close();
		cout << "Conversion is complete." << endl;
	}
	close(sockfd);
	exit(0);
}