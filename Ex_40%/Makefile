all: remoteServer remoteClient

remoteServer: remoteServer.c
	gcc remoteServer.c -o remoteServer

remoteClient: remoteClient.c
	gcc remoteClient.c -o remoteClient

clean:
	rm -f remoteClient remoteServer *.o