#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <netdb.h> 

void main (int argc, char **argv){

    // Var declaration
    int theSocket, otherSocket;
    struct sockaddr_in srvCreds, udpSrv;
    int connection;
    char *line = NULL, *receivePort = argv[3], *extline, *result = NULL, *endResult = NULL;
    char buff[512], bigBuff[4096];
    size_t len = 0;
    ssize_t reAd;
    __pid_t child;
    int tmp = 0, nbytes, check = 0;

    // IPC
    int commandNumber = 0, filesCreated = 0;
    int p[2];

    if(pipe(p) < 0){
        perror("[-] Not piped!\n");
        exit(1);
    }

    // File Creation.
    char fileBase[15];
    char fileName[15];
    strcpy(fileBase, "output."); //output.
    strcat(fileBase, argv[3]);  //output.receivePORT
    strcat(fileBase, ".");     //output.receivePORT.

    // Check if file exists.
    FILE *fp = fopen(argv[4], "r");
    
    if ( fp == NULL ){
        perror("[-] Error on opening the file!\n");
        exit(-1);
    }

    // Socket operation.
    theSocket = socket(PF_INET, SOCK_STREAM, 0); //--TCP
    otherSocket = socket(PF_INET, SOCK_DGRAM, 0); //--UDP

    // Filling the struct sockaddr.--TCP
    memset(&srvCreds, 0, sizeof(srvCreds));
    srvCreds.sin_family = AF_INET;
    srvCreds.sin_port = htons(atoi(argv[2])); // Parameter of the user. --> ServerPort
    // Parameter of the user. --> ServerName (maybe needs change)
    srvCreds.sin_addr.s_addr = INADDR_ANY;//inet_addr(inet_ntoa(*((struct in_addr*)gethostbyname(argv[1])->h_addr_list[0])));

    // Filling the struct sockaddr.--UDP
    memset(&udpSrv, 0, sizeof(udpSrv));
    udpSrv.sin_family = AF_INET;
    udpSrv.sin_port = htons(atoi(argv[3])); // Parameter of the user. --> receivePort
    udpSrv.sin_addr.s_addr = INADDR_ANY;

    // Bind operation.--UDP
    if (bind(otherSocket, (struct sockaddr*) &udpSrv, sizeof(udpSrv)) < 0){
        perror("[-] Error on binding!\n");
        exit(-1);
    } else {
        printf("[+] Successfull bind udp!\n");
    }

    // Connection operation.--TCP
    connection = connect(theSocket, (struct sockaddr*) &srvCreds, sizeof(srvCreds));
    if (connection == -1){
        perror("[-]Connection error!\n");
        exit(-1);
    }else { 
        printf("[+] Connection established!\n");
    }

    
    // Processes from here on.
    child = fork();
    if(child < 0){
        perror("[-] Unable to give birth\n");
        exit(-1);
    } else if (child == 0){
        // close(p[1]);
        // Receive command results and store them on files.--UDP
        memset(&buff, 0, sizeof(buff));
        memset(&bigBuff, 0, sizeof(bigBuff));
        int i = 0;
        char ci[10];
        while(1){
            nbytes = recvfrom(otherSocket, (char *)buff, 512, MSG_WAITALL, NULL, NULL);
            // File name prepare.
            memset(&ci, 0, sizeof(ci));
            memset(&fileName, 0, sizeof(fileName));

            // Custom check for byte write.
            result = strstr(buff, "@");
            endResult = strstr(buff, "fotis");

            if(result != NULL){
                if(check){
                    memcpy(bigBuff, buff, strlen(buff));
                    check = 0;
                } else{
                    strcat(bigBuff, buff);
                }
            } else if(endResult != NULL){
                // File name registration.
                i++;
                sprintf(ci, "%d", i);
                strcpy(fileName, fileBase);
                strcat(fileName, ci);
                
                // Last concat.
                strcat(bigBuff, buff);

                // Actual bytes writen.
                FILE *cmd = fopen(fileName, "w");
                fputs(bigBuff, cmd);
                fclose(cmd);
                // cmd = NULL;

                filesCreated++;
                check = 1;
                memset(&bigBuff, 0, sizeof(bigBuff));
            }

            // read(p[0], &commandNumber, sizeof(commandNumber));

            // if(filesCreated == commandNumber && check == 1){
            //     // Success time to end
            //     exit(0);
            // }

            printf("Client: %s and maybe %d\n", buff, commandNumber);
            memset(&buff, 0, sizeof(buff));
        }
    } else if (child > 0){
        // close(p[0]);

        // reAd line by line and send.--TCP
        while ((reAd = getline(&line, &len, fp)) != -1) {
            extline = strdup(receivePort);
            strcat(extline, line);
            send(theSocket, extline, strlen(extline), 0);
            sleep(1);
            if(tmp == 5){
                tmp = 0;
                sleep(5);
            }
            tmp++;
            commandNumber++;
        }

        // write(p[1], &commandNumber, sizeof(commandNumber));

        wait(NULL);
        close(theSocket);
        exit(0);
    }
}