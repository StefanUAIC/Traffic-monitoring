#include <time.h>
#include <signal.h>
#include "creareTabel.h"

#define PORT 2908
#define SIZE 1000

struct sockaddr_in server;
struct sockaddr_in from;
int sd, sdaccident;
int nrClienti = 0;
int day;
int descriptori[100];
int pid[100];
int port2;

void getSpeed(char comanda[], char raspuns[], struct user &u)
{
	char aux[20];
	strcpy(aux, comanda + strlen("Send speed : "));
	int viteza = atoi(aux);
	sprintf(raspuns, "Viteza este: %d\n", viteza);
	u.speed = viteza;
	//getZone(u, aux);
}

void sendAccident(char raspuns[], struct user &u)
{
    char aux[SIZE], msg[SIZE], idz[SIZE];
	bzero(msg, SIZE);
	bzero(aux, SIZE);
	strcpy(raspuns, "S-a produs un accident pe ");
	getZone(u, aux);
	strcpy(msg, strchr(aux, '|')+1);
	strcpy(idz, strchr(aux, ':') + 2);
	idz[strlen(idz) - (strlen(msg)) - 1] = '\0';
	int id_zona = atoi(idz);
	updateTableAccidente(id_zona, msg);
	strcat(raspuns, msg);
	strcat(raspuns, "\n");
	// sprintf(raspuns, "%s\n", msg);
	getZone(u, aux);
	// id-ul zonei este : 10 | numele zonei
    // snprintf(raspuns, SIZE, "Id-ul zonei este : %d |", id_zona);
}

void getAutomaticSpeed(char comanda[], char raspuns[], struct user u)
{
	char aux[100];
	strcpy(aux, comanda + strlen("Automatic sendspeed : "));
	int viteza = atoi(aux);
	sprintf(raspuns, "Clientul a trimis în mod automatic viteza: %d\n", viteza);
	u.speed = viteza;
}
void sendStructure(int sd, struct user &u)
{
	for(int i = 0; i < u.nrAccidente; i++)
		if(u.id_zona == u.acc[i].id_zonaAccident)
			u.limitaLegala = 30;
	getAccidente(u);
	if(write(sd, &u, sizeof(user)) < 0)
	{
		perror("[server]Eroare la write() catre client.\n");
		return;
	}
}

void getCoordonates(char comanda[], char raspuns[], struct user &u)
{
	char x[20];
	char y[20];
	strcpy(x, comanda + strlen("Send coordonates : x"));
	strcpy(y, strchr(x, 'y')+1);
	x[strlen(x) - strlen(y) - 1] = '\0';
	u.x = atoi(x);
	u.y = atoi(y);
	getZone(u, raspuns);
	sprintf(raspuns,"x este %d iar y este %d\n", u.x, u.y);
}

void getDay(int day)
{
	switch(day)
	{
		case 1: printf("Astăzi este luni\n"); break;
		case 2: printf("Astăzi este marți\n"); break;
		case 3: printf("Astăzi este miercuri\n"); break;
		case 4: printf("Astăzi este joi\n"); break;
		case 5: printf("Astăzi este vineri\n"); break;
		case 6: printf("Astăzi este sâmbătă\n"); break;
		case 7: printf("Astăzi este duminică\n"); break;
	}
}

void deleteRecord()
{
	char sqlstatement[SIZE];
	strcpy(sqlstatement, "DELETE FROM accidente WHERE nume_zona NOT LIKE ' ';");
	if(sqlite3_open("database.db", &db) != SQLITE_OK)
		perror("Eroare la deschiderea bazei de date.\n");
	char *zErrMsg = 0;
	sqlite3_exec(db, sqlstatement, callback, 0, &zErrMsg);
	// strcpy(sqlstatement, "DELETE FROM sports WHERE nume_event NOT LIKE ' ';");
	// sqlite3_exec(db, sqlstatement, callback, 0, &zErrMsg);
	sqlite3_close(db);
}

void functionNotToggled(char raspuns[])
{
	strcpy(raspuns, "Nu ați bifat această opțiune.\n");
}

int Procesare(char comanda[], char raspuns[], struct user &u, int client)
{
    int lungRasp = 0;
	int cmd = 0;
	if(strstr(comanda, "toggle weather") != NULL)
		cmd = 1;
	if(strstr(comanda, "toggle fuel") != NULL)
		cmd = 2;
	if(strstr(comanda, "toggle sports") != NULL)
		cmd = 3;
	if(strstr(comanda, "Show weather") != NULL)
		cmd = 4;
	if(strstr(comanda, "Show fuel") != NULL)
		cmd = 5;
	if(strstr(comanda, "Show sports") != NULL)
		cmd = 6;
	if(strstr(comanda, "Send speed") != NULL)
		cmd = 7;
	if(strstr(comanda, "Send coordonates") != NULL)
		cmd = 8;
	if(strstr(comanda, "Get zone") != NULL)
		cmd = 9;
	if(strstr(comanda, "Send accident") != NULL)
		cmd = 10;
	if(strstr(comanda, "Automatic sendspeed") != NULL)
		cmd = 11;
	switch(cmd)
	{
		case 1:
		{
			u.weather = !u.weather;
			strcpy(raspuns, "S-a bifat opțiunea de vreme.\n");
			lungRasp = strlen(raspuns);
			break;
		}
		case 2:
		{
			u.fuel = !u.fuel;
			strcpy(raspuns, "S-a bifat opțiunea de preturi peco.\n");
			lungRasp = strlen(raspuns);
			break;
		}
		case 3:
		{
			u.sports = !u.sports;
			strcpy(raspuns, "S-a bifat opțiunea de evenimente sportive.\n");
			lungRasp = strlen(raspuns);
			break;
		}
		case 4:
		{
			if(u.weather)
				getInfoFromWeather(raspuns, day);
			else
				functionNotToggled(raspuns);
			lungRasp = strlen(raspuns);
			break;
		}
		case 5:
		{
			if(u.fuel)
				getInfoFromFuel(raspuns, u);
			else
				functionNotToggled(raspuns);
			lungRasp = strlen(raspuns);
			break;
		}
		case 6:
		{
			if(u.sports)
				getInfoFromSports(raspuns, u);
			else
				functionNotToggled(raspuns);
			lungRasp = strlen(raspuns);
			break;
		}
		case 7:
		{
			getSpeed(comanda, raspuns, u);
			lungRasp = strlen(raspuns);
			break;
		}
		case 8:
		{
			getCoordonates(comanda, raspuns, u);
			lungRasp = strlen(raspuns);
			break;
		}
		case 9:
		{
			getZone(u, raspuns);
			lungRasp = strlen(raspuns);
			break;
		}
		case 10:
		{
			bzero(raspuns, SIZE);
			sendAccident(raspuns, u);
			lungRasp = strlen(raspuns);
			break;
		}
		case 11:
		{
			getAutomaticSpeed(comanda, raspuns, u);
			lungRasp = strlen(raspuns);
			break;
		}
		default:
		{
			strcpy(raspuns, "Comandă greșită!\n");
			lungRasp = strlen(raspuns);
			break;
		}
	}
	return lungRasp;
}

void Comunicare(int client, struct user &u)
{
	int nr;
	char comanda[SIZE], raspuns[SIZE];
	bzero(comanda, sizeof(comanda));
	bzero(raspuns, sizeof(raspuns));
	printf("[server]Asteptam mesajul...\n");
	fflush(stdout);
	if(read(client, &nr, sizeof(int)) < 0)
	{
		perror("[server]Eroare la read() de la client ");
		close(client);
		exit(0);
		return;
	}
	if(read(client, comanda, nr) < 0)
	{
		perror("[server]Eroare la read() de la client ");
		close(client);
		exit(0);
		return;
	}
	comanda[strlen(comanda) - 1] = '\0';
	printf("[server]Mesajul a fost receptionat...%s\n", comanda);
	// Baza de date
	// deleteRecord();
	// createTable();
	// Insert(comanda);
	nr = Procesare(comanda, raspuns, u, client);
	printf("[server]Trimitem mesajul inapoi...%s->\n", raspuns);
	if(write(client, &nr, sizeof(int)) < 0)
	{
		perror("[server]Eroare la write() catre client1.\n");
		return;
	}
	if(write(client, raspuns, nr) < 0)
	{
		perror("[server]Eroare la write() catre client2.\n");
		return;
	}
	else
		printf("[server]Mesajul a fost trasmis cu succes.\n");
	sendStructure(client, u);
}

int main ()
{
	if((sdaccident = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	{
    	perror("[server]Eroare la socket() accident.\n");
    	return errno;
    }
	srand(time(0));
	day = rand()%7 + 1;
	getDay(day);
	deleteRecord();
    if((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
    	perror("[server]Eroare la socket().\n");
    	return errno;
    }
	int on = 1;
	setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    bzero(&server, sizeof (server));
    bzero(&from, sizeof (from));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(PORT);
    if(bind(sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
    {
    	perror("[server]Eroare la bind().\n");
    	return errno;
    }
    if(listen(sd, 5) == -1)
    {
    	perror("[server]Eroare la listen().\n");
    	return errno;
    }
    while(1)
    {
    	int client;
    	socklen_t length = sizeof(from);
    	printf("[server]Asteptam la portul %d...\n", PORT);
    	fflush(stdout);
    	client = accept(sd, (struct sockaddr *) &from, &length);
    	if(client < 0)
    	{
    		perror("[server]Eroare la accept().\n");
    		continue;
    	}
		if(read(client, &port2, sizeof(int)) < 0)
		{
    		perror("[server]Eroare la read port2.\n");
    		return errno;
		}
		from.sin_port = htons(port2);
		int auxiliar_int;
		if(read(client, &auxiliar_int, sizeof(int)) < 0)
		{
			perror("[client]Eroare la read() de la server.\n");
			return errno;
		}
		pid[nrClienti++] = auxiliar_int;
    	int pidd;
    	if((pidd = fork()) == -1)
		{
    		close(client);
    		continue;
    	}
		else
			if(pidd > 0) //parinte
			{
				close(client);
				while(waitpid(-1, NULL, WNOHANG));
				continue;
			}
			else
				if(pidd == 0) //copil
				{
					close(sd);
					struct user u;
					while(1)
						Comunicare(client, u);
					printf("Client deconectat de la server.\n");
					close(client);
					exit(0);
				}
		
    }
    return 0;
}