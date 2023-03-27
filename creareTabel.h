#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <cmath>
#include "sqlite3.h"

#define SIZE 1000

sqlite3* db;
sqlite3_stmt* statement;
int id_accidentCurrent = 0;

struct accident {
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

int callback(void *NotUsed, int argc, char **argv, char **azColName)
{
    for(int i = 0; i < argc; i++) {
        printf("%s: %s\n", azColName[i], argv[i]);
    }
    printf("\n");
    return 0;
}

void getAccidente(struct user &u)
{
    if(sqlite3_open("database.db", &db) != SQLITE_OK)
    {
        perror("Eroare la deschiderea bazei de date ");
    }
    sqlite3_prepare_v2(db, "SELECT * FROM accidente", -1, &statement, 0);
    int i = 0;
    while(sqlite3_step(statement) != SQLITE_DONE)
    {
        u.acc[i].id_zonaAccident = sqlite3_column_int(statement, 1);
        strcpy(u.acc[i].zona, (char*)sqlite3_column_text(statement, 2));
        i++;
    }
    u.nrAccidente = i;
}

void updateTableAccidente(int id_zona, char nume_zona[])
{
    id_accidentCurrent++;
    char sqlstatement[SIZE];
	if(sqlite3_open("database.db", &db) != SQLITE_OK)
		perror("Eroare la deschiderea bazei de date.\n");
	char *zErrMsg = 0;
    sprintf(sqlstatement, "INSERT INTO accidente VALUES ('%d', '%d', '%s')", id_accidentCurrent, id_zona, nume_zona);
    sqlite3_exec(db, sqlstatement, callback, 0, &zErrMsg);
    sqlite3_close(db);
}

void getInfoFromWeather(char raspuns[], int day)
{
    char status[100];
    if(sqlite3_open("database.db", &db) != SQLITE_OK)
    {
        perror("Eroare la deschiderea bazei de date ");
    }
    sqlite3_prepare_v2(db, "SELECT * FROM weather WHERE id_weather = ?", -1, &statement, 0);
    sqlite3_bind_int(statement, 1, day);
    while(sqlite3_step(statement) != SQLITE_DONE)
    {
        int temperatura = sqlite3_column_int(statement, 2);
        int temperatura_maxima = sqlite3_column_int(statement, 3);
        int temperatura_minima = sqlite3_column_int(statement, 4);
        int umiditate = sqlite3_column_int(statement, 5);
        strcpy(status, (char*)sqlite3_column_text(statement, 6));
        snprintf(raspuns, SIZE, "Temperatura este: %d, Maxima zilei este: %d, Minima zilei este %d, Umiditate de %d%%", temperatura, temperatura_maxima, temperatura_minima, umiditate);
        strcat(raspuns, " Status: ");
        strcat(raspuns, status);
    }
}

void getZone(struct user &u, char raspuns[])
{
    if(sqlite3_open("database.db", &db) != SQLITE_OK)
    {
        perror("Eroare la deschiderea bazei de date ");
    }
    sqlite3_prepare_v2(db, "SELECT * FROM zone WHERE x1 <= ? AND x2 >= ? AND y1 <= ? AND y2 >= ?", -1, &statement, 0);
    sqlite3_bind_int(statement, 1, u.x);
    sqlite3_bind_int(statement, 2, u.x);
    sqlite3_bind_int(statement, 3, u.y);
    sqlite3_bind_int(statement, 4, u.y);
    while(sqlite3_step(statement) != SQLITE_DONE)
    {
        int id_zona = sqlite3_column_int(statement, 0);
        printf("id_zona din creare tabel este : %d\n", id_zona);
        u.limitaLegala = sqlite3_column_int(statement, 6);
        snprintf(raspuns, SIZE, "Id-ul zonei este : %d |", id_zona);
        strcat(raspuns, (char*)sqlite3_column_text(statement, 1));
        u.id_zona = id_zona;
        strcpy(u.zona, (char*)sqlite3_column_text(statement, 1));
        printf("id zona accident este : %d", u.acc[0].id_zonaAccident);
        for(int i = 0; i < u.nrAccidente; i++)
            if(u.id_zona == u.acc[i].id_zonaAccident)
                u.limitaLegala = 30;
    }
}

void getInfoFromFuel(char raspuns[], struct user u)
{
    if(sqlite3_open("database.db", &db) != SQLITE_OK)
    {
        perror("Eroare la deschiderea bazei de date ");
    }
    sqlite3_prepare_v2(db, "SELECT * FROM fuel", -1, &statement, 0);
    bzero(raspuns, SIZE);
    while(sqlite3_step(statement) != SQLITE_DONE)
    {
        char companie[50];
        char zona[50];
        char pretb[20], pretm[20];
        bzero(companie, sizeof(companie));
        bzero(zona, sizeof(zona));
        bzero(pretb, sizeof(pretb));
        bzero(pretm, sizeof(pretm));
        strcpy(companie, (char*)sqlite3_column_text(statement, 1)); 
        strcpy(zona, (char*) sqlite3_column_text(statement, 2));
        double pret_benzina = sqlite3_column_double(statement, 3);
        snprintf(pretb, 20, "%f", pret_benzina);
        pretb[4] = '\0';
        double pret_motorina = sqlite3_column_double(statement, 4);
        snprintf(pretm, 20, "%f", pret_motorina);
        pretm[4] = '\0';
        strcat(raspuns, "Companie: ");
        strcat(raspuns, companie);
        strcat(raspuns, " zona: ");
        strcat(raspuns, zona);
        strcat(raspuns, " pretul benzinei: ");
        strcat(raspuns, pretb);
        strcat(raspuns, " pretul motorinei: ");
        strcat(raspuns, pretm);
        strcat(raspuns, ".\n");
    }
}

void getInfoFromSports(char raspuns[], struct user &u)
{
    char aux[SIZE];
    if(sqlite3_open("database.db", &db) != SQLITE_OK)
    {
        perror("Eroare la deschiderea bazei de date ");
    }
    sqlite3_prepare_v2(db, "SELECT * FROM sports WHERE id_event = ?", -1, &statement, 0);
    sqlite3_bind_int(statement, 1, u.countEvent%8+1);
    u.countEvent++;
    while(sqlite3_step(statement) != SQLITE_DONE)
    {
        strcpy(aux, (char*)sqlite3_column_text(statement, 1));
        strcat(raspuns, "Evenimentul este: ");
        strcat(raspuns, aux);
        strcpy(aux, (char*)sqlite3_column_text(statement, 2));
        strcat(raspuns, " La ora : ");
        strcat(raspuns, aux);
        strcpy(aux, (char*)sqlite3_column_text(statement, 3));
        strcat(raspuns, ":");
        strcat(raspuns, aux);
        strcpy(aux, (char*)sqlite3_column_text(statement, 4));
        strcat(raspuns, " pe data de: ");
        strcat(raspuns, aux);
        strcat(raspuns, "\n");
    }
}

void Insert(char comanda[])
{
	char sqlstatement[SIZE];
	if(sqlite3_open("database.db", &db) != SQLITE_OK)
		perror("Eroare la deschiderea bazei de date.\n");
	char *zErrMsg = 0;
	strcpy(sqlstatement, "INSERT INTO zone VALUES ('1','Bulevardul Carol I', '0', '1000', '0', '100','50');");
	sqlite3_exec(db, sqlstatement, callback, 0, &zErrMsg);
    strcpy(sqlstatement, "INSERT INTO zone VALUES ('2','Bulevardul Independenței', '0', '1000', '101', '200','90');");
    sqlite3_exec(db, sqlstatement, callback, 0, &zErrMsg);
    strcpy(sqlstatement, "INSERT INTO zone VALUES ('3','Strada Elena Doamna', '0', '1000', '201', '300','30');");
    sqlite3_exec(db, sqlstatement, callback, 0, &zErrMsg);
	strcpy(sqlstatement, "INSERT INTO zone VALUES ('4','Bulevardul Tudor Vladimirescu', '0', '1000', '301', '400','130');");
	sqlite3_exec(db, sqlstatement, callback, 0, &zErrMsg);
    strcpy(sqlstatement, "INSERT INTO zone VALUES ('5','Strada Sărărie', '0', '1000', '401', '500','70');");
    sqlite3_exec(db, sqlstatement, callback, 0, &zErrMsg);
    strcpy(sqlstatement, "INSERT INTO zone VALUES ('6','Strada Păcurari', '0', '1000', '501', '600','90');");
    sqlite3_exec(db, sqlstatement, callback, 0, &zErrMsg);
    strcpy(sqlstatement, "INSERT INTO zone VALUES ('7','Strada Titu Maiorescu', '0', '1000', '601', '700','130');");
	sqlite3_exec(db, sqlstatement, callback, 0, &zErrMsg);
    strcpy(sqlstatement, "INSERT INTO zone VALUES ('8','Bulevardul Ștefan cel Mare și Sfânt', '0', '1000', '701', '800','120');");
    sqlite3_exec(db, sqlstatement, callback, 0, &zErrMsg);
    strcpy(sqlstatement, "INSERT INTO zone VALUES ('9','Bulevardul Socola',  '0', '1000', '801', '900','100');");
    sqlite3_exec(db, sqlstatement, callback, 0, &zErrMsg);
    strcpy(sqlstatement, "INSERT INTO zone VALUES ('10','Strada Duca Vodă', '0', '1000', '901', '1000','90');");
    sqlite3_exec(db, sqlstatement, callback, 0, &zErrMsg);

	strcpy(sqlstatement, "INSERT INTO fuel VALUES ('1','Lukoil','Bulevardul Independenței','6.35','7.55');");
	sqlite3_exec(db, sqlstatement, callback, 0, &zErrMsg);
    strcpy(sqlstatement, "INSERT INTO fuel VALUES ('2','OSCAR','Strada Elena Doamna','6.25','7.45');");
	sqlite3_exec(db, sqlstatement, callback, 0, &zErrMsg);
    strcpy(sqlstatement, "INSERT INTO fuel VALUES ('3','Petrom','Strada Duca Vodă','6.30','7.40');");
	sqlite3_exec(db, sqlstatement, callback, 0, &zErrMsg);
    strcpy(sqlstatement, "INSERT INTO fuel VALUES ('4','Rompetrol','Bulevardul Tudor Vladimirescu','6.24','7.37');");
	sqlite3_exec(db, sqlstatement, callback, 0, &zErrMsg);
    strcpy(sqlstatement, "INSERT INTO fuel VALUES ('5','OMV','Bulevardul Ștefan cel Mare și Sfânt','6.31','7.41');");
	sqlite3_exec(db, sqlstatement, callback, 0, &zErrMsg);
    strcpy(sqlstatement, "INSERT INTO fuel VALUES ('6','Socar','Bulevardul Socola','6.40','7.60');");
	sqlite3_exec(db, sqlstatement, callback, 0, &zErrMsg);
    strcpy(sqlstatement, "INSERT INTO fuel VALUES ('7','MOL','Strada Sărărie','6.36','7.50');");
	sqlite3_exec(db, sqlstatement, callback, 0, &zErrMsg);
    strcpy(sqlstatement, "INSERT INTO fuel VALUES ('8','Shell','Strada Titu Maiorescu','6.66','7.77');");
	sqlite3_exec(db, sqlstatement, callback, 0, &zErrMsg);
    
	strcpy(sqlstatement, "INSERT INTO weather VALUES ('1','Luni','15','22','7','65', 'Însorit');");
	sqlite3_exec(db, sqlstatement, callback, 0, &zErrMsg);
	strcpy(sqlstatement, "INSERT INTO weather VALUES ('2','Marți','13','20','6','50', 'Puțin înnorat');");
	sqlite3_exec(db, sqlstatement, callback, 0, &zErrMsg);
	strcpy(sqlstatement, "INSERT INTO weather VALUES ('3','Miercuri','10','19','4','80', 'Înnorat');");
	sqlite3_exec(db, sqlstatement, callback, 0, &zErrMsg);
	strcpy(sqlstatement, "INSERT INTO weather VALUES ('4','Joi','8','17','2','60', 'Averse sporadice');");
	sqlite3_exec(db, sqlstatement, callback, 0, &zErrMsg);
	strcpy(sqlstatement, "INSERT INTO weather VALUES ('5','Vineri','6','15','0','95', 'Precipitații');");
	sqlite3_exec(db, sqlstatement, callback, 0, &zErrMsg);
	strcpy(sqlstatement, "INSERT INTO weather VALUES ('6','Sâmbătă','2','10','-6','100','Ploaie și zăpadă');");
	sqlite3_exec(db, sqlstatement, callback, 0, &zErrMsg);
	strcpy(sqlstatement, "INSERT INTO weather VALUES ('7','Duminică','-10','3','-25','69','Ninsoare');");
	sqlite3_exec(db, sqlstatement, callback, 0, &zErrMsg);

    strcpy(sqlstatement, "INSERT INTO sports VALUES ('1','Meci fotbal Steaua - Dinamo','20','30','11/01/23');");
	sqlite3_exec(db, sqlstatement, callback, 0, &zErrMsg);
    strcpy(sqlstatement, "INSERT INTO sports VALUES ('2','Maraton de alergat prin Copou','12','00','14/01/23');");
	sqlite3_exec(db, sqlstatement, callback, 0, &zErrMsg);    
    strcpy(sqlstatement, "INSERT INTO sports VALUES ('3','Curse de mașini Formula 1','16','30','15/01/23');");
	sqlite3_exec(db, sqlstatement, callback, 0, &zErrMsg);    
    strcpy(sqlstatement, "INSERT INTO sports VALUES ('4','Hrănit porumbei în Piața Unirii','12','30','16/01/23');");
	sqlite3_exec(db, sqlstatement, callback, 0, &zErrMsg);
    strcpy(sqlstatement, "INSERT INTO sports VALUES ('5','Concurs mâncat hotdogi de la Profi','20','00','17/01/23');");
	sqlite3_exec(db, sqlstatement, callback, 0, &zErrMsg);    
    strcpy(sqlstatement, "INSERT INTO sports VALUES ('6','Meci de tenis Simona Halep - Maria Șarapova','19','00','18/01/23');");
    sqlite3_exec(db, sqlstatement, callback, 0, &zErrMsg);
    strcpy(sqlstatement, "INSERT INTO sports VALUES ('7','Meci de tenis Simona Halep - Serena Williams','20','30','19/01/23');");
	sqlite3_exec(db, sqlstatement, callback, 0, &zErrMsg);    
    strcpy(sqlstatement, "INSERT INTO sports VALUES ('8','LAN Party ASII','10','00','04/02/23');");
	sqlite3_exec(db, sqlstatement, callback, 0, &zErrMsg);
    sqlite3_close(db);
}

void createTable()
{
	char sqlstatement[SIZE];
    if(sqlite3_open("database.db", &db) != SQLITE_OK)     
		perror("Eroare la deschiderea bazei de date.\n");
	char *zErrMsg = 0;
	// strcpy(sqlstatement, "CREATE TABLE zone ('id_zona' INT, nume_zona VARCHAR2, x1 INT, x2 INT, y1 INT, y2 INT, limita_legala INT)");
	// sqlite3_exec(db, sqlstatement, callback, 0, &zErrMsg);
    // strcpy(sqlstatement, "CREATE TABLE fuel ('id_peco' INT, companie VARCHAR2, zona VARCHAR2, pret_benzina REAL, pret_motorina REAL)");
    // sqlite3_exec(db, sqlstatement, callback, 0, &zErrMsg);
    // strcpy(sqlstatement, "CREATE TABLE weather ('id_weather' INT, nume_zi VARCHAR2, temperatura INT, temperatura_maxima INT, temperatura_minima INT, umiditate INT, status VARCHAR2)");
    // sqlite3_exec(db, sqlstatement, callback, 0, &zErrMsg);
	// strcpy(sqlstatement, "CREATE TABLE sports ('id_event' INT, nume_event VARCHAR2, ora VARCHAR2, minute VARCHAR2, data VARCHAR2)");
    // sqlite3_exec(db, sqlstatement, callback, 0, &zErrMsg);
    // strcpy(sqlstatement, "CREATE TABLE accidente ('id_accident' INT, 'id_zona' INT, 'nume_zona' VARCHAR2)");
	// sqlite3_exec(db, sqlstatement, callback, 0, &zErrMsg);
    sqlite3_close(db);
}