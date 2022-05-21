#include "g_local.h"

#ifndef LINUX
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h >
#else
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>   
#endif

FILE	*StdLogFile;		//GSLogMod
static fd_set global_fds;
extern int errno;

struct sockaddr_in net_name_to_address(char *name)
{
    char               *buf;
    char               *port;
    struct hostent     *host;
    struct sockaddr_in  addr;
	
    memset(&addr, 0, sizeof(struct sockaddr_in));
	
    buf = strdup(name);
    strtok(buf, ":");
    if ((port = strtok(NULL, "")) != NULL) {
		addr.sin_port = atoi(port);
		if (addr.sin_port <= 0 || addr.sin_port >= (1<<16)) {
			fprintf(stderr, "net_name_to_address: %s: invalid port number\n",
				port);
			exit(1);
		}
    }
    else {
		addr.sin_port = 0;
    }
	
	if (inet_addr(buf) == INADDR_NONE) {
		if ((host = gethostbyname(buf)) != NULL) {
			addr.sin_addr   = *(struct in_addr *)host->h_addr;
		}
		else {
#ifndef LINUX
			fprintf(stderr, "%s: %d", buf, WSAGetLastError());
#else
			fprintf(stderr, "%s: %s", buf, "net_name_to_addr");
#endif
			exit(1);
		}
    }
	
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(addr.sin_port);
	
    free(buf);
	
    return addr;
}

void net_send(int s, void *p, int len)
{
    if (send(s, p, len, 0) != len) {
		perror("send");
		if (errno != 0)
			exit(1);
    }
}

int net_open_socket(void)
{
    int s;
	
    if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
#ifndef LINUX
		printf("WSA %d\n", WSAGetLastError());
#else
		perror("socket"); 
#endif

		exit(1);
    }
	
    FD_SET(s, &global_fds);
	
    return s;
}
void net_close_socket(int s)
{
#ifndef LINUX
    if (s && closesocket(s) < 0) {
#else
    if (s && close(s) < 0) {  
#endif
		perror("close");
		exit(1);
    }
	
    FD_CLR(s, &global_fds);
}

void net_connect_socket(int s, struct sockaddr_in *a, short p)
{
    a->sin_port = htons(p);
    if (connect(s, (struct sockaddr *)a, sizeof(struct sockaddr_in)) < 0) {
		perror("connect");
		exit(1);
    }
}


void GSSendLine(char *buf)
{
    struct sockaddr_in server_addr;
	int server_port;
    int server_socket;
    
	server_addr = net_name_to_address(netlog->string);
	server_port = ntohs(server_addr.sin_port);
	server_socket = net_open_socket();
    net_connect_socket(server_socket, &server_addr, server_port);
	net_send(server_socket, buf, strlen(buf)+1);
	net_close_socket(server_socket);
}

void GSOpenLog(void)
{
	char mpath[80];
	cvar_t *gamedir;
	cvar_t *logname;
	 
	gamedir = gi.cvar("game", ".", CVAR_LATCH);
	logname = gi.cvar("logname", "stdlog.log", 0);
    strcpy(mpath, gamedir->string);
// NOTE: Change this if compiling for non-DOS!
#ifdef LINUX
        strcat(mpath, "/");
#else
		strcat(mpath, "\\");
#endif
	strcat(mpath, logname->string);

	StdLogFile = fopen (mpath, "a+t"); //GSLogMod Start, open file
}

void GSCloseLog(void)
{
	fclose(StdLogFile);
}

void GSLogShutdown(void)
{
	if (logfile->value == 2)											//GSLogMod
	{
		GSOpenLog();
		fprintf (StdLogFile, "\t\tGameEnd\t\t\t%d\n",(int)level.time);//GSLogMod
		GSCloseLog();
	}

}

void GSLogStartup(void)
{
	if (logfile->value == 2)
	{
		GSOpenLog();
		fprintf (StdLogFile, "\t\tStdLog\t1.22\n"); //GSLogMod StdLog + VersNr
		fprintf (StdLogFile,"\t\tPatchName\tRocket Arena 2 %s\n",GAMEVERSION); //GSLogMod PatchName is here = ""
		GSCloseLog();
	}
}

void GSLogNewmap(void)
{
	if (logfile->value == 2)
	{
		GSOpenLog();
		fprintf (StdLogFile, "\t\tMAP\t%s\n",level.level_name); // log name of map
		fprintf (StdLogFile, "\t\tGameStart\t\t\t%d\n",(int)level.time); // start counting frags
		GSCloseLog();
	}
}
void GSdodeathlog(char *s)
{
	fprintf(StdLogFile,s);
	if (netlog->string[0]!='\0')
		GSSendLine(s);
}
// GSLogMod Start Log Frags, Suicides, Deaths
void GSLogDeath(edict_t *self, edict_t *inflictor, edict_t *attacker)
{

	char *weaponname;
	char buf[1000];
	
	if (logfile->value==2)
	{

		GSOpenLog();
		// A Suicide occured
		if (attacker == self)
		{
			if (attacker->client->pers.weapon)
			{
				if (!strcmp(self->client->pers.weapon->classname,"weapon_grenadelauncher") || !strcmp(self->client->pers.weapon->classname,"weapon_rocketlauncher") || !strcmp(self->client->pers.weapon->classname,"weapon_bfg"))
				{
					Com_sprintf (buf,sizeof(buf), "%s\t\tSuicide\t%s\t-1\t%d\t%d\n",self->client->pers.netname, self->client->pers.weapon->pickup_name, (int)level.time, self->client->ping );
					GSdodeathlog(buf);
				}
				else
				{
					Com_sprintf (buf,sizeof(buf), "%s\t\tSuicide\t\t-1\t%d\t%d\n", self->client->pers.netname, (int)level.time, self->client->ping);	
					GSdodeathlog(buf);
				}
			} else
			{
				Com_sprintf (buf,sizeof(buf), "%s\t\tSuicide\t\t-1\t%d\t%d\n", self->client->pers.netname, (int)level.time, self->client->ping);				
				GSdodeathlog(buf);
			}
			GSCloseLog();
			return;
		}

		// A frag player killed player occured	
		if (attacker && attacker->client)
		{
			if (attacker->client->pers.weapon)
				weaponname=attacker->client->pers.weapon->pickup_name;
			else
				weaponname="BFG10K";
			Com_sprintf (buf,sizeof(buf), "%s\t%s\tKill\t%s\t1\t%d\t%d\n",attacker->client->pers.netname,self->client->pers.netname,weaponname,(int)level.time, attacker->client->ping);
			GSdodeathlog(buf);
			GSCloseLog();
			return;

		}
		
		// Print other deaths - all suicides without a weapon
		Com_sprintf (buf,sizeof(buf),"%s\t\tSuicide\t\t-1\t%d\t%d\n", self->client->pers.netname, (int)level.time, self->client->ping);
		GSdodeathlog(buf);
		GSCloseLog();	
	}
}



// GSLogMod START Player Name Logging
void GSLogEnter(edict_t *ent)
{
	if (logfile->value == 2)
	{
		GSOpenLog();
		fprintf (StdLogFile, "\t\tPlayerConnect\t%s\t\t%d\n",ent->client->pers.netname,(int)level.time);
		GSCloseLog();
	}
	
}

void GSLogExit(edict_t *ent)
{	
	if (logfile->value==2)
	{
		
		GSOpenLog();
		fprintf (StdLogFile, "\t\tPlayerLeft\t%s\t\t%d\n",ent->client->pers.netname,(int)level.time);
		GSCloseLog();
	}
}
#ifndef LINUX
int wsock32state(int mode)
{
    if ( mode == 1 ) 
	{
		WORD wVersionRequested;
		WSADATA wsaData;
		int err; 
		
		wVersionRequested = MAKEWORD( 1, 1 ); 
		err = WSAStartup( wVersionRequested, &wsaData );
		if 	( err != 0 ) 
		{
			gi.dprintf("WS Error: %d\n", WSAGetLastError());
			return 0;
		}
		
	}
    else if ( mode == 0 ) 
	{
		WSACleanup();
		return 1;
	}
    return 1;
}
#endif
