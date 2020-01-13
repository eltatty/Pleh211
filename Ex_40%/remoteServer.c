#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <regex.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/wait.h>
#include <netdb.h> 

// Signal handlers.
void sighandler2();
void sighandler3();

// Pipes. buffer, buffferLength, commandCategory, portLength.
int p[2], m[2], l[2], k[2];

void main(int argc, char ** argv){

    // Ignore SIGPIPE for parent.
    signal(SIGPIPE, SIG_IGN);

    // Variable declaration.
    int theSocket, client_socket, max_fd, rcvMsg;
    int opt =1, check = 0;
    struct sockaddr_in srvCreds, clntCreds, udpClnt;
    socklen_t clntLen = sizeof(clntCreds);
    char rcvBuffer[512];
    regex_t regex[4];
    fd_set master, readfs;
    int portNumber = atoi(argv[1]);

    // Demo variable declaration.
    unsigned char buff[100], msg[100], chlen[4], len[4], *token, cC[2], chV[2], chM[2], cM[2];
    int numfChildren = atoi(argv[2]);
    __pid_t child[numfChildren];
    int yok, tmp =0, cV, y=0, cN;

    // Last part var decl.
    FILE * pf;
    int count, udpSocket;
    char udpPort[5], *udpBuffer, c;

    // Creating regex.
    regcomp(&regex[0], "^\\s*(ls|cat|cut|grep|tr)", REG_EXTENDED);
    regcomp(&regex[1], "^\\s*(end)\\s*$", REG_EXTENDED);
    regcomp(&regex[2], "^\\s*(timeToStop)\\s*$", REG_EXTENDED);

    // Creating pipes.
    if (pipe(p) < 0 || pipe(m) < 0 || pipe(l) < 0 || pipe(k) < 0){
        perror("[-] Not piped!\n");
        exit(1);
    }

    //------------- TCP OPERATION ----------------------------------------------------//
    // Creating the main socket.
    theSocket = socket(PF_INET, SOCK_STREAM, 0);

    // Setting socket options.
    if(setsockopt(theSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) == -1){
        perror("[-] Error on socket options!");
        exit(1);
    }
    
    // Filling the struct sockaddr.
    memset(&srvCreds, 0, sizeof(srvCreds));
    srvCreds.sin_family = PF_INET;
    srvCreds.sin_port = htons(portNumber);
    srvCreds.sin_addr.s_addr = INADDR_ANY;

    // Bind operation.
    if(bind(theSocket, (struct sockaddr *) &srvCreds, sizeof(srvCreds))){
        perror("[-] Error on binding!");
        exit(-1);
    }

    // Listen operation.
    if (listen(theSocket, 5) < 0){
        perror("[-] Error on listening!");
        exit(-1);
    } else {
        printf("[+] Listening on any..\n");
    }
    //--------------------------------------------------------------------------------//

    //------------ Children Proccesses-----------------------------------------------//
    for(int i = 0; i < numfChildren; i++){
        child[i] = fork();
        if(child[i] < 0){
            perror("[-] Not created!\n");
        } else if(child[i] == 0){
            
            // Ignore SIGPIPE for child.
            signal(SIGPIPE, SIG_IGN);

            while(1){

                // Signal handling.
                signal(SIGUSR1, sighandler3);

                // Close unnecessary pipeEnds.
                close(p[1]);
                close(m[1]);
                close(l[1]);
                close(k[1]);

                // Read length of the incoming command.
                read(m[0], chlen, 4);
                tmp = chlen[0] << 24 | chlen[1] << 16 | chlen[2] << 8 | chlen[3];

                // Read command category.
                read(l[0], cC, 2);
                cV = atoi(cC);

                // Read buffer.
                read(p[0], buff, tmp);   
                
                // Read port for udp. -------------------> PROBLEM
                cN = 0;
                read(k[0], cM, 2);
                cN = atoi(cM);
                // memset(udpPort, 0, sizeof(udpPort));
                memcpy(udpPort, buff, cN);

                // printf("Child [%d] with length of [%lu] and category [%d] and port [%.*s] says ==>\n%s", getpid(), strlen(buff), cV, cN, buff, buff);
                printf("Child [%d] with length of [%lu] and category [%d] and port [%s] says ==>\n%s", getpid(), strlen(buff), cV, udpPort, buff);

                if(cV == 2){
                    // End command.
                    fprintf( stderr, "IM DEAD[%d]\n", getpid()); // Print this message to STDERR.
                    close(m[0]);
                    close(p[0]);
                    close(l[0]);
                    close(k[0]);
                    kill(getppid(), SIGCHLD);
                    exit(0);
                } else if(cV == 3){
                    // TimeToStop command.
                    kill(getppid(), SIGINT);
                } else if(cV == 1){
                    // Illegal command.
                }else if(cV == 0){
                    //Legal command.
                    pf = popen(&buff[cN], "r");
                    
                    
                    // Take command results from pf.
                    c = fgetc(pf); 
                    while (c != EOF) { 
                        printf (" %c", c);
                        strcat(udpBuffer, &c);
                        c = fgetc(pf); 
                    }

                    strcat(udpBuffer, "fotis");
                    
                    // Reset
                    pf = NULL;
                }


                if(cN > 0){

                    char *hello = "Hello from client";
                    udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
                    memset(&udpClnt, 0, sizeof(udpClnt));
                    udpClnt.sin_family = AF_INET;
                    udpClnt.sin_port = htons(atoi(udpPort));
                    udpClnt.sin_addr.s_addr = INADDR_ANY; // Maybe get the ip from clntCreds through another pipe.

                    sendto(udpSocket, (const char *)udpBuffer, strlen(udpBuffer), 
                    MSG_CONFIRM, (const struct sockaddr *) &udpClnt,  
                    sizeof(udpClnt));
                }

                // Reset.
                memset(chlen, 0, sizeof(chlen));
                memset(buff, 0, sizeof(buff));
                memset(cC, 0, sizeof(cC));
                memset(cM, 0, sizeof(cM));
                memset(udpPort, 0, sizeof(udpPort));
                memset(udpBuffer, 0, sizeof(udpBuffer));

                sleep(4);
            }
        }
    }
    //-----------------------------------------------------------------------------//

    //-------- Father proccess ---------------------------------------------------//
    
    // Prepare for select.
    FD_ZERO(&master);
    FD_ZERO(&readfs);

    FD_SET(theSocket, &master);
    max_fd = theSocket;

    while(1){

        readfs = master;        

        if(select(max_fd + 1, &readfs, NULL, NULL, NULL) == -1){
            perror("[-] Error on select!\n");
            exit(-1);
        }

        for(int i = 0; i <= max_fd; i++){
            
            // Check every slot in fd_set.
            if(FD_ISSET(i, &readfs)){
                
                // New inbound connection or data from existing client.
                if(i == theSocket){

                    // Accept operation.
                    client_socket = accept(theSocket, (struct sockaddr *) &clntCreds, &clntLen);
                    if (client_socket == -1){
                        perror("[-] Error on accepting!");
                        exit(-1);
                    }  

                    FD_SET(client_socket, &master);
                    printf("[+] Connection accepted from %s:%d\n", inet_ntoa(clntCreds.sin_addr), ntohs(clntCreds.sin_port));
                    
                    if(client_socket > max_fd){
                        max_fd = client_socket;
                    }

                } else{

                    //Receive operation from the existing client.
                    rcvMsg = recv(i, rcvBuffer, 512, 0);

                    if ( rcvMsg == -1){
                        perror("[-] Error on receiving!\n");
                        exit(-1);
                    } else if( rcvMsg == 0){
                        close(i);
                        FD_CLR(i, &master);
                    }else { 
                
                        // SET credentials of i client socket != server socket (getsockname)!!!
                        getpeername(i, (struct sockaddr *) &clntCreds, &clntLen);

                        // Extract port length.
                        count = 0;
                        while( isdigit(rcvBuffer[count]) != 0){
                            count += 1;
                        }

                        // Command validation.
                        check = 0;

                        if(strlen(rcvBuffer) > 100 + count){
                            // printf("[-] Illegal command!\n");
                            check = 1;
                        } else {
                            
                            // Classic command
                            if(regexec(&regex[0], &rcvBuffer[count], 0 , NULL, 0) == 0){

                                // printf("[+] Legal command! --> %s from %d\n", rcvBuffer, ntohs(clntCreds.sin_port));

                                for(int j=1+count; j<strlen(rcvBuffer); j++){
                                    
                                    // Command need trimming;
                                    if(rcvBuffer[j+count-1] == ';' || rcvBuffer[j+count-1] == '|' && regexec(&regex[0], &rcvBuffer[j+count], 0 , NULL, 0) != 0){
                                        
                                        memset(&rcvBuffer[j-1+count], 0, sizeof(&rcvBuffer[j-1+count]));
                                        // printf("[-] Trim the command! --> %s from %d\n\n", rcvBuffer, ntohs(clntCreds.sin_port));

                                    }
                                }
                            } else if(regexec(&regex[1], &rcvBuffer[count], 0 , NULL, 0) == 0) {

                                // printf("[+] End Command! --> %s from %d\n", rcvBuffer, ntohs(clntCreds.sin_port));
                                check = 2;

                            } else if(regexec(&regex[2], &rcvBuffer[count], 0 , NULL, 0) == 0) {

                                // printf("[+] TimeToStop Command! --> %s from %d\n", rcvBuffer, ntohs(clntCreds.sin_port));
                                check = 3;

                            } else {
                                // printf("[-] Illegal command!\n");
                                check = 1;
                            }
                        }

                        // Clean product for processes.
                        if(check != 1){
                            printf("[+] Legal command --> %s --> %d\n", rcvBuffer, ntohs(clntCreds.sin_port));
                        }else{
                            printf("[-] Illegal command --> %s --> %d\n", rcvBuffer, ntohs(clntCreds.sin_port));
                        }

                        // From here on the parent has the info.
                        
                        // Close unnecessary pipeEnds.
                        close(m[0]);
                        close(p[0]);
                        close(l[0]);
                        close(k[0]);

                        // Send to children.

                        // Make the length of the command into 4 byte integer.
                        yok = strlen(rcvBuffer);
                        len[0] = (yok >> 24) & 0xFF;
                        len[1] = (yok >> 16) & 0xFF;
                        len[2] = (yok >> 8) & 0xFF;
                        len[3] = yok & 0xFF;

                        // Convert.
                        sprintf(chV, "%d", check);
                        sprintf(chM, "%d", count);

                        // Write for children.
                        write(m[1], len, 4);
                        write(l[1], chV, 2);
                        write(k[1], chM, 2);
                        write(p[1], rcvBuffer, yok);

                        // sleep(3);

                        // Reset.
                        memset(len, 0, sizeof(len));
                        memset(chV, 0, sizeof(chV));
                        memset(chM, 0, sizeof(chM));

                        // Signal handling.
                        signal(SIGINT, sighandler2);
                    }

                    memset(rcvBuffer, 0, sizeof(rcvBuffer));

                }
            }
        }

    }

    // Close operation.
    regfree(regex);
    close(theSocket);
}

void sighandler2(){
    __pid_t wpid;
    int status =0;
    signal(SIGINT, sighandler2);
    signal(SIGUSR1, SIG_IGN);
    kill(0, SIGUSR1);
    while ((wpid = wait(&status)) > 0)
    // sleep(2);
    exit(0);
}

void sighandler3(){
    signal(SIGUSR1, sighandler3);

    // Gonna need a loop for closing pipes.
    close(p[0]);
    close(p[1]);
    close(m[0]);
    close(m[1]);
    close(l[0]);
    close(l[1]);
    close(k[0]);
    close(k[1]);
    exit(0);
}