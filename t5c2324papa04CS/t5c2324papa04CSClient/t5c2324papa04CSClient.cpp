/*
nome file		:t5c2324papa04CSclient.c
nome progetto	:t5c2324papa04CSclient.vcxproj
nome soluzione	:t5c2324papa04CS.sln
autore			:papa cesare
data			:3/11/2023
scopo			:primi passi con libreria winsock (lato client)
note tecniche	:

analisi problema/strategie risolutive:

pseudocodice	:

				inizializzare Winsock.
				Creare un socket.
				Connettersi al server.
				Inviare e ricevere dati.
				Eseguire la disconnessione.
*/

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include <windows.h>
#include "../t5c2324papa04CS.h"

#pragma comment (lib, "Ws2_32.lib")

int main(int argc, char* argv[])
{
	int iResult;
	struct addrinfo* result = NULL;
	struct addrinfo* ptr = NULL;
	struct addrinfo hints;

	//Creare un oggetto WSADATA denominato wsaData
	WSADATA wsaData;	//istanziazine dell'ohhryyo

	printf("Client avviato\n");

	//Chiamare WSAStartup e restituire il relativo valore come intero e verificare la presenza di errori
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (iResult != 0)
	{
		printf("WSAStartup failed: %d\n", iResult);
		return 1;
	}

	printf("WSAStartup eseguita\n");

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;	//definito IPV4
	hints.ai_socktype = SOCK_STREAM;	//definisce l'uso come stream
	hints.ai_protocol = IPPROTO_TCP;	//protocollo TCP

	printf("creazione variabili eseguita\n");

	// Resolve the server address and port
	iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	printf("getaddrinfo eseguita\n");

	//Creare un oggetto SOCKET denominato ConnectSocket.
	SOCKET ConnectSocket = INVALID_SOCKET;

	// Attempt to connect to the first address returned by the call to getaddrinfo
	ptr = result;

	// Create a SOCKET for connecting to server
	ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

	//Verificare gli errori per assicurarsi che il socket sia un socket valido
	if (ConnectSocket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	printf("Socket creato\n");

	// Connect to server ossia svolgimento handshaking con il server richiesto
	iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("errore di connessioni (handshaking) con il server %s [%s]\n", argv[1], DEFAULT_PORT);
		closesocket(ConnectSocket);
		ConnectSocket = INVALID_SOCKET;
		return 2;
	}

	printf("la connessione (handshaking) con il server %s:%s ha avuto successo!\n", argv[1], DEFAULT_PORT);


	//invio di una stringa verso il server

	char recvbuf[DEFAULT_BUFLEN_NORMAL];
	char sendbuf[DEFAULT_BUFLEN_SMALL];
	int recvbuflen = DEFAULT_BUFLEN_NORMAL;
	int sendbuflen = DEFAULT_BUFLEN_SMALL;
	char fileToAsk[DEFAULT_BUFLEN_SMALL];	//the name of the file to receive from the server

	strcpy_s(fileToAsk, (int)sizeof(fileToAsk), argv[3]);

	strcpy_s(sendbuf, (int)sizeof(sendbuf), argv[2]);

	encryption_algorithm(sendbuf);

	// Invia il contenuto del buffer, valorizzato con la stringa associata ad argv[3]
	iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf) + 1, 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}
	else {
		iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);	//attesa chiave di connessione da parte del server
		//printf("%s\n", recvbuf);
		if (iResult > 0) {
			decryption_algorithm(recvbuf);

			if ((strcmp(recvbuf, PRESENTAZIONE_SERVER) == 0)) {
				sprintf_s(sendbuf, "%s", fileToAsk);	//inserisco il nome del file nel buffer da inviare
				//sprintf_s(sendbuf, "%s", argv[4]);

				printf("file richiesto al server: %s\n", sendbuf);

				encryption_algorithm(sendbuf);
				iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf) + 1, 0);	//richiedo il file al server
				if (iResult == SOCKET_ERROR) {
					printf("send failed: %d\n", WSAGetLastError());
					closesocket(ConnectSocket);
					WSACleanup();
					return 1;
				}

				iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);

				if (iResult > 0) {
					decryption_algorithm(recvbuf);

					if (recvbuf[0] == 's') {

						FILE* fp;

						fopen_s(&fp, fileToAsk, "wb");

						char* ptrStrUtile = NULL;
						char* ptrStrComodo = NULL;
						int dimensioneTotFile;
						ptrStrUtile = strtok_s(recvbuf, ":", &ptrStrComodo);

						//printf("ptrStrComodo: %s\n", ptrStrComodo);

						if (fp != NULL && ptrStrComodo != 0) {
							dimensioneTotFile = atoi(ptrStrComodo);
							int dimensioneScritta = 0;
							//printf("atoi(ptrStrComodo) : %d\n", atoi(ptrStrComodo));

							strcpy_s(sendbuf, sizeof(sendbuf), "ok");	// mettendo come standard nella variabile sendbuf la stringa "ok"
							encryption_algorithm(sendbuf);	// encrypting della variabile sendbuf
								while (dimensioneScritta < dimensioneTotFile) {

									iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf) + 1, 0);	//invio un ok al server per informarlo che sono pronto
								
									if (iResult > 0) {
										iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
										decryption_algorithm(recvbuf);
										//recvbuf[iResult] = 0;

										//printf("recv without '0': %s", recvbuf);

										if (iResult > 0) {

											//printf("recvbuf: %s\n", recvbuf);
											dimensioneScritta = dimensioneScritta + iResult;

											//printf("dimensioneScritta: %d\n", dimensioneScritta);

											fwrite(recvbuf, sizeof(char), iResult, fp);
										}
									}
								}

							fclose(fp);
						}
						else {
							printf("File non aperto per motivi ignoti! \n");

							strcpy_s(sendbuf, sizeof(sendbuf), "erroreInteronAlClient");

							encryption_algorithm(sendbuf);

							send(ConnectSocket, sendbuf, strlen(sendbuf) + 1, 0);
						}
					}
					else {
						printf("File non esistente nel server!\n");
					}
				}
			}
			else {
				printf("Server errato!\n");
				return -1;
			}
		}
		else {
			printf("Server errato!\n");
			return -1;
		}
	}

	//printf("Inviati al server %d caratteri\n", iResult);

	printf("Chiusura connessione con il server\n");

	// cleanup
	closesocket(ConnectSocket);
	WSACleanup();

	return 0;
}
