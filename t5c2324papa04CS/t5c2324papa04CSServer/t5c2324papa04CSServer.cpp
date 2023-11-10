/*
nome file		:t5c2324papa04CSserver.c
nome progetto	:t5c2324papa04CSserver.vcxproj
nome soluzione	:t5c2324papa04CS.sln
autore			:papa cesare
data			:10/11/2023
scopo			:primi passi con libreria winsock (lato server)
note tecniche	:

analisi problema/strategie risolutive:

pseudocodice	:

				Inizializzare Winsock.
				Creare un socket.
				Associare il socket.
				Ascoltare il socket per un client.
				Accettare una connessione da un client.
				Ricevere e inviare dati.
				Eseguire la disconnessione.
*/

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <process.h>
#include <Windows.h>
#include <time.h>
#include "../t5c2324papa04CS.h"

#pragma comment(lib, "Ws2_32.lib")

typedef struct clientData {
	//Creare un oggetto SOCKET temporaneo denominato ClientSocket per accettare connessioni dai client
	SOCKET ClientSocket;	//socket per la comunicazione con il client che contatterà questo server
	SOCKADDR_IN addr;
	int addrsize;
}client_data;

FILE* logfp; //creo il puntatore al file di log come una variabile globale così che pure i thread lo possano vedere

void connection_handling(void* newConnectionData);

int main(int argc, char* argv[])
{
	int iResult;
	struct addrinfo* result = NULL;
	struct addrinfo* ptr = NULL;
	struct addrinfo hints;

	//Creare un oggetto WSADATA denominato wsaData
	WSADATA wsaData;	//istanziazine dell'ohhryyo

	//Chiamare WSAStartup e restituire il relativo valore come intero e verificare la presenza di errori
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (iResult != 0)
	{
		printf("WSAStartup failed: %d\n", iResult);
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;	//definito IPV4
	hints.ai_socktype = SOCK_STREAM;	//definisce l'uso come stram
	hints.ai_protocol = IPPROTO_TCP;	//protocollo TCP
	hints.ai_flags = AI_PASSIVE;

	// Resolve the local address and port to be used by the server
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0)
	{
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	SOCKET ListenSocket = INVALID_SOCKET;	//socket per il server

	// Create a SOCKET for the server to listen for client connections

	ListenSocket = socket(result->ai_family,
		result->ai_socktype,
		result->ai_protocol);

	//Verificare la presenza di errori per assicurarsi che il socket sia un socket valido.

	if (ListenSocket == INVALID_SOCKET)
	{
		printf("Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	// Setup the TCP listening socket
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);

	if (iResult == SOCKET_ERROR)
	{
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	//listen
	if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		printf("Listen failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	client_data newConnectionData[NMAX_THREADS];

	printf("Il server e' in ascolto!\n");

	time_t t = time(NULL);
	struct tm tm{};
	localtime_s(&tm, &t);
	char logName[100];
	strftime(logName, sizeof(logName), "logs/log_%Y-%m-%d_%H-%M-%S.log", &tm);

	fopen_s(&logfp, logName, "wb");

	if (logfp != NULL) {
		HANDLE semaphore = CreateSemaphore(NULL, 1, 1, L"logsWritingSemaphore");

		if (semaphore != NULL) {
			for (int i = 0; true; i++) {

				if (i != NMAX_THREADS) {
					newConnectionData[i].ClientSocket = INVALID_SOCKET;

					// Accept a client socket ossia effettuazione dell'handsaking
					//ClientSocket = accept(ListenSocket, NULL, NULL);	
					newConnectionData[i].addrsize = sizeof(newConnectionData[i].addr);
					//printf("Server in attesa di una connessione\n");
					newConnectionData[i].ClientSocket = accept(ListenSocket, (SOCKADDR*)&newConnectionData[i].addr, &newConnectionData[i].addrsize);	//inserisce all'interno di addr i dati del client

					_beginthread(connection_handling, 0, &newConnectionData[i]);
				}
				else
					_WAIT_CHILD();


			}

			CloseHandle(semaphore);
		}
	}
	else
		printf("file log non aperto per motivi ignoti!\n");
	
	printf("Errore interno del server!\n");
	fclose(logfp);
	freeaddrinfo(result);
	closesocket(ListenSocket);
	WSACleanup();
	return -1;
}

void connection_handling(void* newConnectionData) {
	int iResult;
	struct addrinfo* result = NULL;
	SOCKET clientSocket = static_cast<clientData*>(newConnectionData)->ClientSocket;
	SOCKADDR_IN addr = static_cast<clientData*>(newConnectionData)->addr;
	char bufferLog[100];
	HANDLE semaphore = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, L"logsWritingSemaphore");

	if (semaphore != NULL) {
		//printf("Inizio thread con clientSocket %u\n", clientSocket);
	//printf("INVALID_SOCKET %u\n", INVALID_SOCKET);

		char sIndIPClient[16];	//per memorizzare ip client

		inet_ntop(AF_INET, &addr.sin_addr, sIndIPClient, sizeof(sIndIPClient));

		//verifica se l'handshaking ha avuto successo
		if (clientSocket == INVALID_SOCKET) {
			printf("accept failed: %d\n", WSAGetLastError());
			freeaddrinfo(result);
			_endthread();
		}

		sprintf_s(bufferLog, sizeof(bufferLog), "il server e' stato contattato dal client TCP con ip: %s\n", sIndIPClient);
		printf("%s", bufferLog);	//stampo a schermo bufferLog

		WaitForSingleObject(semaphore, INFINITE);

		fwrite(bufferLog, sizeof(char), strlen(bufferLog), logfp);

		ReleaseSemaphore(semaphore, 1, NULL);

		char recvbuf[DEFAULT_BUFLEN_SMALL];
		int recvbuflen = DEFAULT_BUFLEN_SMALL - 1;

		//inizio del protocollo applicativo
		//il server inizia a ricevere e attende la chiave di connessione
		iResult = recv(clientSocket, recvbuf, recvbuflen, 0);

		//printf("chiave cryptata ricevuta: %s\n", recvbuf);	//test di verifica di cryptazione e decryptazione
		decryption_algorithm(recvbuf);
		//printf("chiave decryptata ricevuta: %s\n", recvbuf);	//test di verifica di cryptazione e decryptazione

		if (iResult > 0 && strcmp(recvbuf, CHIAVE_CONNESSIONE) == 0)
		{

			char sendbuf[DEFAULT_BUFLEN_NORMAL];
			int sendbuflen = DEFAULT_BUFLEN_NORMAL;
			strcpy_s(sendbuf, DEFAULT_BUFLEN_NORMAL, PRESENTAZIONE_SERVER);

			//printf("chiave non cryptata inviata: %s\n", sendbuf);	//test di verifica di cryptazione e decryptazione
			encryption_algorithm(sendbuf);
			//printf("chiave cryptata inviata: %s\n", sendbuf);	//test di verifica di cryptazione e decryptazione

			iResult = send(clientSocket, sendbuf, (int)strlen(sendbuf) + 1, 0);

			if (iResult != SOCKET_ERROR) {
				iResult = recv(clientSocket, recvbuf, recvbuflen, 0);

				decryption_algorithm(recvbuf);

				char* ptrStrUtile = NULL;
				char* ptrStrComodo = NULL;
				ptrStrUtile = strtok_s(recvbuf, "/", &ptrStrComodo);

				if (iResult > 0 && recvbuf[0] != '.' && recvbuf[0] != '/' && ptrStrComodo == NULL) {
					FILE* fp; //creo il puntatore del file da ricercare
					char fileToSend[DEFAULT_BUFLEN_SMALL];

					sprintf_s(fileToSend, sizeof(recvbuf), "stories/%s", recvbuf);

					fopen_s(&fp, fileToSend, "rb");

					if (fp != NULL) {
						int dimFile;
						int dimInviata = 0;

						sprintf_s(bufferLog, sizeof(bufferLog), "File %s richiesto dal client trovato!\n", recvbuf);
						printf("%s", bufferLog);	//stampo a schermo bufferLog

						WaitForSingleObject(semaphore, INFINITE);

						fwrite(bufferLog, sizeof(char), strlen(bufferLog), logfp);

						ReleaseSemaphore(semaphore, 1, NULL);

						fseek(fp, 0, SEEK_END);

						dimFile = ftell(fp);

						sprintf_s(sendbuf, "%s:%d", "si", dimFile);

						//printf("senbuf: %s", sendbuf);

						encryption_algorithm(sendbuf);	// encrypting sendbuf

						send(clientSocket, sendbuf, (int)strlen(sendbuf) + 1, 0);

						rewind(fp);	//mi riposiziono all'inizio del file;

						while (dimInviata < dimFile && recvbuf[0] != 'e') {
							fd_set readfds;
							FD_ZERO(&readfds);
							FD_SET(clientSocket, &readfds);

							struct timeval tv;
							tv.tv_sec = 5;
							tv.tv_usec = 0;

							int selectReturn = select(clientSocket + 1, &readfds, NULL, NULL, &tv);

							if (selectReturn == -1) {
								perror("Select()");

								// shutdown the send half of the connection since no more data will be sent
								iResult = shutdown(clientSocket, SD_SEND);
								if (iResult == SOCKET_ERROR) {
									printf("shutdown failed: %d\n", WSAGetLastError());
									closesocket(clientSocket);
									WSACleanup();
									_endthread();
								}

								_endthread();
							}
							else
								if (selectReturn == 0) {
									sprintf_s(bufferLog, sizeof(bufferLog), "Errore interno del client!\n");
									printf("%s", bufferLog);	//stampo a schermo bufferLog

									WaitForSingleObject(semaphore, INFINITE);

									fwrite(bufferLog, sizeof(char), strlen(bufferLog), logfp);

									ReleaseSemaphore(semaphore, 1, NULL);

									break;
								}
								else {
									//aspetto che il client mi invii un "ok"
									iResult = recv(clientSocket, recvbuf, recvbuflen, 0);
									//printf("iResult recv: %d\n", iResult);

									if (iResult > 0) {

										//printf("Dentro if\n");
										decryption_algorithm(recvbuf);

										//printf("recvbuf: %s\n", recvbuf);

										//printf("expression result: %d\n", ((dimInviata + DEFAULT_BUFLEN_NORMAL) <= dimFile) ? DEFAULT_BUFLEN_NORMAL : (dimFile - dimInviata));

										fread_s(sendbuf, sendbuflen, (int)sizeof(char), ((dimInviata + DEFAULT_BUFLEN_NORMAL) <= dimFile) ? DEFAULT_BUFLEN_NORMAL : (dimFile - dimInviata), fp);
										//printf("Dopo fread\n");

										//encrypto la strnga da mandare
										encryption_algorithm(sendbuf);
										// invio al client la stringa del file
										iResult = send(clientSocket, sendbuf, ((dimInviata + DEFAULT_BUFLEN_NORMAL) <= dimFile) ? DEFAULT_BUFLEN_NORMAL : (dimFile - dimInviata), 0);

										dimInviata = dimInviata + iResult;

										//printf("dimInviata: %d\n", dimInviata);
									}

									//printf("Fuori if\n");
								}
						}


						//printf("Fuori dal while\n");

						if (recvbuf[0] == 'e') {
							sprintf_s(bufferLog, sizeof(bufferLog), "Errore interno del client!\n");
							printf("%s", bufferLog);	//stampo a schermo bufferLog

							WaitForSingleObject(semaphore, INFINITE);

							fwrite(bufferLog, sizeof(char), strlen(bufferLog), logfp);

							ReleaseSemaphore(semaphore, 1, NULL);
						}

					}
					else {

						sprintf_s(bufferLog, sizeof(bufferLog), "File %s richiesto dal client non trovato!\n", recvbuf);
						printf("%s", bufferLog);	//stampo a schermo bufferLog

						WaitForSingleObject(semaphore, INFINITE);

						fwrite(bufferLog, sizeof(char), strlen(bufferLog), logfp);

						ReleaseSemaphore(semaphore, 1, NULL);

						strcpy_s(sendbuf, DEFAULT_BUFLEN_NORMAL, "no");

						encryption_algorithm(sendbuf);	// encrypting sendbuf

						send(clientSocket, sendbuf, (int)strlen(sendbuf) + 1, 0);
					}
				}
				else {

					sprintf_s(bufferLog, sizeof(bufferLog), "Il client con ip [%s] sta cercando di leggere dei file in posizioni anomale!\n", sIndIPClient);
					printf("%s", bufferLog);	//stampo a schermo bufferLog

					WaitForSingleObject(semaphore, INFINITE);

					fwrite(bufferLog, sizeof(char), strlen(bufferLog), logfp);

					ReleaseSemaphore(semaphore, 1, NULL);

					strcpy_s(sendbuf, DEFAULT_BUFLEN_NORMAL, "no");

					encryption_algorithm(sendbuf);	// encrypting sendbuf

					send(clientSocket, sendbuf, (int)strlen(sendbuf) + 1, 0);
				}
			}
			/*printf("Bytes received: %d\n Ricevuto: \n", iResult);
			printf("\"%s\"\n", recvbuf);	//decido di considerare il contenuto del buffer di ricezione come se fosse una stringa C*/

		}
		else {
			sprintf_s(bufferLog, sizeof(bufferLog), "Ciave di connessione errata! \nConnessione da ip: %s rifiutata!\n", sIndIPClient);
			printf("%s", bufferLog);	//stampo a schermo bufferLog

			WaitForSingleObject(semaphore, INFINITE);

			fwrite(bufferLog, sizeof(char), strlen(bufferLog), logfp);

			ReleaseSemaphore(semaphore, 1, NULL);

		}

		sprintf_s(bufferLog, sizeof(bufferLog), "Chiusura connessione con il client %s\n", sIndIPClient);
		printf("%s", bufferLog);	//stampo a schermo bufferLog

		WaitForSingleObject(semaphore, INFINITE);

		fwrite(bufferLog, sizeof(char), strlen(bufferLog), logfp);

		ReleaseSemaphore(semaphore, 1, NULL);
	}
	else {
		sprintf_s(bufferLog, sizeof(bufferLog), "Errore interno nel thread!\n");
		printf("%s", bufferLog);	//stampo a schermo bufferLog

		WaitForSingleObject(semaphore, INFINITE);

		fwrite(bufferLog, sizeof(char), strlen(bufferLog), logfp);

		ReleaseSemaphore(semaphore, 1, NULL);
	}

	// shutdown the send half of the connection since no more data will be sent
	iResult = shutdown(clientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed: %d\n", WSAGetLastError());
		closesocket(clientSocket);
		WSACleanup();
		_endthread();
	}

	// cleanup
	/*closesocket(clientSocket);
	WSACleanup();*/

	//printf("Fine thread\n");

	_endthread();
}