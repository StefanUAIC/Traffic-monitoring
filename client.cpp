#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <sys/wait.h>
#define SIZE 1000
int cmd;
const int timer_length = 30;
time_t start_time = time(NULL);

struct accident{
    char zona[SIZE];
    int id_zonaAccident = 0;
};

struct user {
	int speed = 0;
	int x = 0;
	int y = 0;
	int id_zona = 1;
	int countEvent = 0;
	bool weather = 0;
	bool sports = 0;
	bool fuel = 0;
	accident acc[30];
	int nrAccidente = 0;
	int limitaLegala = 150;
	char zona[SIZE];
}u;


int sd, sdaccident;
int port, port2;
struct sockaddr_in server;
void receiveStructure()
{
	if(read(sd, &u, sizeof(user)) < 0)
	{
		perror("[server]Eroare la read() de la server.\n");
		return;
	}
}

void printStructure()
{
	for(int i = 0; i < u.nrAccidente; i++)
		if(u.id_zona == u.acc[i].id_zonaAccident)
			u.limitaLegala = 30;
	printf("Viteza: %d\n", u.speed);
	printf("Coordonatele: x=%d, y=%d\n", u.x, u.y);
	printf("Id-ul zonei: %d\n", u.id_zona);
	printf("CountEvent: %d\n", u.countEvent);
	printf("Fuel: %d\n", u.fuel);
	printf("Weather: %d\n", u.weather);
	printf("Sports: %d\n", u.sports);
	printf("Limita legala: %d\n", u.limitaLegala);
	printf("Nr accidente: %d\n", u.nrAccidente);
	printf("Zona: %s\n", u.zona);
}

void sendComanda(char comanda[], char raspuns[])
{
	int nr = strlen(comanda);
	if(write(sd, &nr, sizeof(int)) < 0)
	{
		perror("[client]Eroare la write() spre server.\n");
		exit(0);
	}
	if(write(sd, comanda, nr) < 0)
	{
		perror("[client]Eroare la write() spre server.\n");
		exit(0);
	}
	if(read(sd, &nr, sizeof(int)) < 0)
	{
		perror ("[client]Eroare la read() de la server.\n");
		exit(0);
	}
	if(read(sd, raspuns, nr) < 0)
	{
		perror ("[client]Eroare la read() de la server.\n");
		exit(0);
	}
	raspuns[nr] = '\0';
	receiveStructure();
	if(u.speed > u.limitaLegala)
		strcat(raspuns, "\nAți depășit limita legală de viteză.\n");
	printStructure();
}
void introducereDateInceput()
{
	char comanda[SIZE];
	char raspuns[SIZE];
	printf("Introduceți coordonatele sub formatul x<value>y<value>: ");
	fflush(stdout);
	char aux[SIZE];
	int lung = read(0, aux, sizeof(aux));
	aux[lung] = '\0';
	strcpy(comanda, "Send coordonates : ");
	strcat(comanda, aux);
	sendComanda(comanda, raspuns);
	printf("%s\n", raspuns);
	bzero(comanda, SIZE);
	printf("Introduceți viteza cu care vă deplasați: ");
	fflush(stdout);
	bzero(aux, SIZE);
	lung = read(0, aux, sizeof(aux));
	aux[lung] = '\0';
	strcpy(comanda, "Send speed : ");
	strcat(comanda, aux);
	sendComanda(comanda, raspuns);
	printf("%s\n", raspuns);
}
int main(int argc, char *argv[])
{
	int nr, pid;
	if((pid = fork()) < 0)
	{
		perror("Eroare la fork().\n");
		exit(0);
	}
	else
		if(pid > 0) //parinte
		{
			if(argc != 3)
			{
				printf("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
				return -1;
			}
			port = atoi(argv[2]);
			port2 = 2910;
			if((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
			{
				perror("Eroare la socket().\n");
				return errno;
			}
			server.sin_family = AF_INET;
			server.sin_addr.s_addr = inet_addr(argv[1]);
			server.sin_port = htons(port);
			if(connect(sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1)
			{
				perror ("[client]Eroare la connect().\n");
				return errno;
			}
			if(write(sd, &port2, sizeof(int)) < 0)
			{
				perror("[client]Eroare la write() port2\n");
				return errno;
			}		
			if(write(sd, &pid, sizeof(int)) < 0)
			{
				perror("[client]Eroare la write() spre server.\n");
				return errno;
			}
			introducereDateInceput(); 
			char raspuns[SIZE], comanda[SIZE];
			while(1)
			{
				int pid2;
				if((pid2 = fork()) < 0)
				{
					perror("Eroare la fork().\n");
					exit(0);
				}
				else
					if(pid2 > 0) //parinte
					{
						printf("[client]: ");
						fflush(stdout);
						int lungRasp = read(0, comanda, SIZE);
						comanda[lungRasp] = '\0';
						kill(pid2, SIGKILL);
						sendComanda(comanda, raspuns);
						printf("%s", raspuns);
						int status;
					}
					else //copil
					{
						while(1)
						{
							if(time(NULL) - start_time >= timer_length)
							{
								printf("\r              \n");
								start_time = time(NULL);
								snprintf(comanda, SIZE, "Automatic sendspeed : %d\n", u.speed);
								sendComanda(comanda, raspuns);
								printf("%s", raspuns);
								printf("[client]: ");
								fflush(stdout);
							}
						}
						exit(0);
					}
			}
			waitpid(-1, NULL, WNOHANG);
			close(sd);
			return 0;
		}
		else
		{
			struct sockaddr_in from;
			bzero(&from, sizeof(from));
			if((sdaccident = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
			{
				perror("[client]Eroare la socket().\n");
				return errno;
			}
			from.sin_family = AF_INET;
			from.sin_addr.s_addr = inet_addr(argv[1]);
			from.sin_port = htons(port2);
			if(bind(sdaccident, (struct sockaddr *) &from, sizeof (struct sockaddr)) == -1)
    		{
    			perror("[client]Eroare la bind().\n");
    			return errno;
   			}
			while(1)
			{
				struct sockaddr_in cliaddr;
				socklen_t len;
				int n;
				char buffer[1024];
				int MAXLINE = 1024;
				len = sizeof(cliaddr);
				n = recvfrom(sdaccident, (char *)buffer, MAXLINE,
				MSG_WAITALL, ( struct sockaddr *) &cliaddr,&len);
				buffer[n] = '\0';
			}
			exit(0);
		}
}