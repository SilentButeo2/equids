#include "CImageServer.h"

bool CISdebug = false;

CImageServer::CImageServer(sem_t *imsem, CRawImage* im)
{
	connected = false;
	serverSocket = mySocket = -1;
	dataSem = imsem;
	messageRead = 0;
	image = im;
	CISdebug = true;
	stop=false;
	sem_init(&connectSem,0,1);	
	sem_init(&captureSem,0,1);
}

CImageServer::~CImageServer()
{
}

void* connectLoop(void *serv)
{
	struct sockaddr_in clientAddr;
	socklen_t addrLen = sizeof(clientAddr);
	CImageServer* server = (CImageServer*) serv;
	int newServer = 0;
	while (!server->stop)
	{
		newServer = accept(server->serverSocket, (struct sockaddr *)&clientAddr,&addrLen);
		if (newServer > -1){
			if (CISdebug) fprintf(stdout,"%sIncoming connection from %s.\n",server->log_prefix.c_str(), inet_ntoa(clientAddr.sin_addr));
			if (CISdebug) fprintf(stdout,"%sIncoming connection accepted on socket level %i.\n",server->log_prefix.c_str(), newServer);
			sem_wait(&server->connectSem);
			server->mySocket = newServer;
			sem_post(&server->connectSem);
			pthread_t* thread=(pthread_t*)malloc(sizeof(pthread_t));
			pthread_create(thread,NULL,&serverLoop,(void*)server);
		}else{
			if (CISdebug) fprintf(stdout,"CImageServer: Accept on listening socked failed.\n");
		}
	}
	return NULL;
}

int CImageServer::initServer(const char* port)
{
	if (CISdebug) fprintf(stdout,"%sInitialize server.\n", log_prefix.c_str());
	int used_port = atoi(port);
	stop = false;
	struct sockaddr_in mySocketAddr;
	mySocketAddr.sin_family = AF_INET;
	mySocketAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	mySocketAddr.sin_port = htons(used_port);
	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket < 0)
	{
		if (CISdebug) fprintf(stdout,"%sCannot create socket.\n", log_prefix.c_str());
		return -1;
	}
	if (bind(serverSocket,( struct sockaddr *)&mySocketAddr,sizeof(mySocketAddr)) < 0)
	{
		if (CISdebug) fprintf(stdout,"%sCannot bind socket.\n", log_prefix.c_str());
		return -2;
	}
	if (listen(serverSocket,1) < 0)
	{
		if (CISdebug) fprintf(stdout,"%sCannot make socket listen.\n", log_prefix.c_str());
	}
	pthread_t* thread=(pthread_t*)malloc(sizeof(pthread_t));
	pthread_create(thread,NULL,&connectLoop,(void*)this);
	return 0;
}

void CImageServer::stopServer(){
	this->stop = true;
}

void* serverLoop(void* serv)
{
	SClientInfoIMG info;
	CImageServer* server = (CImageServer*) serv;
	sem_wait(&server->connectSem);
	info.socket = server->mySocket;
	info.sem = server->dataSem;
	sem_post(&server->connectSem);
	int dataOk = 0;
	bool connected = true;
	while (connected && !server->stop){
		dataOk = 0;
		if (CISdebug) fprintf(stdout,"%sWait for a message.\n", server->log_prefix.c_str() );
		int msg = server->checkForMessage(info.socket);
		if (CISdebug) fprintf(stdout,"%sMessage received from %i.\n",server->log_prefix.c_str(),info.socket);
		sem_wait(info.sem);
		server->sendImage(info.socket);
		if (CISdebug) fprintf(stdout,"%sPost to capturing semaphore.\n",server->log_prefix.c_str());
		sem_post(&server->captureSem);
		//		sem_post(info.sem);
		//remove this so "the other" can send sem_post
		usleep(100000);
		if (msg == 1){
			connected = false;
			fprintf(stdout,"%sDisconnecting.\n", server->log_prefix.c_str());
		}
	}
	server->closeConnection(info.socket);
	return NULL;
}

int CImageServer::checkForMessage(int socket)
{
	int message = 0;
	int receiveResult = recv(socket,&message,1,MSG_WAITALL);
	if ( receiveResult >0)
	{
		if (CISdebug) fprintf(stdout,"%sPacket accepted length %i.\n", log_prefix.c_str(), receiveResult);
	}
	else
	{
		if (CISdebug) fprintf(stdout,"%sDisconnect detected.\n", log_prefix.c_str());
		message = 1;
	}
	return message;
}

int CImageServer::sendImage(int socket)
{
	if (send(socket,image->data,image-> getsize(),MSG_NOSIGNAL) == image->getsize()){
		if (CISdebug) fprintf(stdout,"%sImage sent.\n", log_prefix.c_str());
		return 0; 
	}
	else
	{
		if (CISdebug) fprintf(stdout,"%sNetwork error.\n", log_prefix.c_str());
		return -1;
	}
	return 0;
}

int CImageServer::closeConnection(int socket)
{
	close(socket);
	connected = false;
	return 0;
}

