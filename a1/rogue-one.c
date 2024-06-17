#define _XOPEN_SOURCE 600
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

#define PORT "2520"
#define PLANS_FILE "deathstarplans.dat"

typedef struct {
    char * data;
    int length;
} buffer;

extern int errno;

/* This function loads the file of the Death Star
   plans so that they can be transmitted to the
   awaiting Rebel Fleet. It takes no arguments, but
   returns a buffer structure with the data. It is the
   responsibility of the caller to deallocate the 
   data element inside that structure.
   */ 
buffer load_plans( );
int sendall( int socket, char *buf, int *len ); // From lecture 8


int main( int argc, char** argv ) {

    if ( argc != 2 ) {
        printf( "Usage: %s IP-Address\n", argv[0] );
        return -1;
    }
    printf("Planning to connect to %s.\n", argv[1]);

    buffer buf = load_plans();

    struct addrinfo hints; 
    struct addrinfo *res; 

// • Establish a TCP/stream connection to the command ship. This is the client-side, so you
// need to get the address information ( getaddrinfo()), 

    // #Lecture 7 example BEGIN
    int sockfd;

    memset(&hints, 0, sizeof( hints )); //~ make sure hints are initialized with 0

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    //maslab-pc07.uwaterloo.ca
    // getaddrinfo("maslab-pc07.uwaterloo.ca", "80", &hints, &res); //TODO: check the port
    // getaddrinfo("www.uwaterloo.ca", "80", &hints, &res);
    getaddrinfo(argv[1], "2520", &hints, &res);


    // create a socket ( socket()), and    
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    // then connect() to the remote server.
    int status = connect(sockfd, res->ai_addr, res->ai_addrlen);
    // printf("Tried connecting via socket: %d \n", status);
    if (status != 0) {
        printf( "Couldn't establish the socket connection. Status: %d. Terminating. \n", status);
        return -1;
    }
    // #Lecture 7 example END




// • Once connected, transmit the plans for the Death Star! The plans are loaded for you in the
// load_plans function (which loads the file into memory for you -- remember to deallocate
// the data when done). The plans are large, so you perhaps cannot send it all in one call to
// send(); so check the results and send repeatedly as needed.
    int  sentall = sendall ( sockfd, buf.data, &buf.length);

    if (status != 0) {
        printf( "Couldn't send all data. Terminating. \n");
        return -1;
    }
// • After you make your transmission, try to receive (using recv()) a response that could be
    char *response_buf = malloc (64);
    int received = recv(sockfd, response_buf, 64, 0); //TODO: check flags
    // int recv( int sockfd, void * buffer, int length, int flags );
// up to 64 characters long. If your upload was unsuccessful, you will get an error of some

// sort, otherwise you will get back a message that indicates success. You should print this
    printf("Response: %s", response_buf);
// message so you can see what it says. If it says “Success”, it worked!
    // • Close the connection.
    close(sockfd);
// • Your program should not leak memory; be sure to destroy/deallocate anything
// initialized/allocated.
    
    freeaddrinfo ( res );
    free ( response_buf );
    // freeaddrinfo( &hints );
    free (buf.data);

    return 0;
}

buffer load_plans( ) {
    struct stat st;
    stat( PLANS_FILE, &st );
    ssize_t filesize = st.st_size;
    char* plansdata = malloc( filesize );
    int fd = open( PLANS_FILE, O_RDONLY );
    memset( plansdata, 0, filesize );
    read( fd, plansdata, filesize );
    close( fd );

    buffer buf;
    buf.data = plansdata;
    buf.length = filesize;

    return buf;
}

// given in lecture 8
int sendall( int socket, char *buf, int *len ) {
  int total = 0;        // how many bytes we've sent
  int bytesleft = *len; // how many we have left to send
  int n;

  while( total < *len ) {
    n = send( socket, buf + total, bytesleft, 0 );
    if (n == -1) { 
      break; 
    }
    total += n;
    bytesleft -= n;
  }
  *len = total; // return number actually sent here
  return n == -1 ? -1 : 0; // return -1 on failure, 0 on success
}
