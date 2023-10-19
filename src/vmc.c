#include <signal.h>
#include <stdio.h>      //printf()
#include <stdlib.h>     //exit()
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include "DEV_Config.h"
#include "MQTTAsync.h"
#include <time.h>

#define UNUSED(x) (void)(x)
#define QOS 1
#define PATTERN_LENGTH 5
#define UPDATETIME 2000 	// ms update
#define debug 0

int t=67000;
int sendValue=0;
MQTTAsync client;
int connected=0;
const char *topicIn="vmc/speed/set";
int finito=0;
int waitToExit=0;
int subscribed=0;
char connString[80];
void connLost(void *context,char *cause);
void doConnect();
const int32_t unsetValue=0xffffffff;

double ms()
{
	struct timeval tp;
	gettimeofday(&tp, NULL);
	return tp.tv_sec*1000.0+tp.tv_usec/1000.0;
}

struct tvars
{
	const char *topic;
	int address;
	int l;
	int exp;
	int32_t v;
	int32_t on_value;
	double ms;
	uint8_t to_send;
}

vars[]=
{
	{"vmc/speed\0",32,1,0,unsetValue,unsetValue,0,0},
	{"vmc/NTC11\0",10,2,1,unsetValue,unsetValue,0,0},
	{"vmc/NTC12\0",12,2,1,unsetValue,unsetValue,0,0},
	{"vmc/NTC21\0",14,2,1,unsetValue,unsetValue,0,0},
	{"vmc/NTC22\0",16,2,1,unsetValue,unsetValue,0,0},
	{"vmc/SLA\0  ",18,2,1,unsetValue,unsetValue,0,0},
	{"vmc/S4\0   ",20,2,1,unsetValue,unsetValue,0,0},
	{"vmc/BP\0   ",22,2,0,unsetValue,9,0,0},
	{"vmc/V1\0   ",26,2,0,unsetValue,unsetValue,0,0},
	{"vmc/V2\0   ",29,2,0,unsetValue,unsetValue,0,0}
};
#define N sizeof vars / sizeof vars[0]

void segfault_sigaction(int signal)
{
	UNUSED(signal);
    fprintf(stderr,"Caught segfault\n");
	exit(1);
}


void onSend(void *context,MQTTAsync_successData *response)
{
	UNUSED(context);
	UNUSED(response);
//	fprintf(stderr,"Message with token %d delivered\n",response->token);
}

int sendMessage(MQTTAsync client,const char *topic,char *payload)
{
	MQTTAsync_responseOptions opts=MQTTAsync_responseOptions_initializer;
	MQTTAsync_message pubmsg=MQTTAsync_message_initializer;

	opts.onSuccess=onSend;
	opts.onFailure=NULL;
	opts.context=client;
	pubmsg.payload=payload;
	pubmsg.payloadlen=strlen(payload);
	pubmsg.qos=QOS;
	pubmsg.retained=1;
	int rc=MQTTAsync_sendMessage(client,topic,&pubmsg,&opts);
//	printf("sending message esito %d\n",rc);
	return rc;
}

void parseBuffer(unsigned char *pData)
{
	if(pData[5]==2)		//vmc spenta
		pData[32]=0;

	for(uint32_t i=0;i<N;i++)
	{
		int32_t nv=0;
		uint8_t nbits=8*vars[i].l;
		for(int j=0;j<vars[i].l;j++)
			nv+=(pData[vars[i].address+j]<<(j*8));
		//sistemo nv per numeri negativi

		if((1<<(nbits-1))&nv)	//bit segno (negativo)
		    nv=nv-(1<<nbits);

		if(vars[i].on_value!=unsetValue)
			nv=(nv==vars[i].on_value?1:0);
		double now=ms();

		if((nv!=vars[i].v)&&(now-vars[i].ms>UPDATETIME))
		{
			vars[i].v=nv;
			vars[i].ms=now;
			vars[i].to_send=1;
		}
	}
}


int msgarrvd(void *context,char *topicName,int topicLen,MQTTAsync_message *message)
{
	UNUSED(context);
	if(message&&message->payloadlen)
	{
		char *c=message->payload;
		fprintf(stderr,"received %s = %s\n",topicName,c);
		if(strncmp(topicName,topicIn,topicLen)==0)
		{
			int a=atoi(c);
			int32_t cs=vars[0].v;
			if((a>=0)&&(a<=3)&&(cs!=unsetValue))
				sendValue=(cs==0?2:(a>0?a+2:1));
		}

		MQTTAsync_freeMessage(&message);
		MQTTAsync_free(topicName);
	}
	return 1;
}


void deliveryComplete(void *context, int token)
{
	UNUSED(context);
	UNUSED(token);

//	fprintf(stderr,"deliveryComplete %d\n",token);
}

void onSubscribe(void *context,MQTTAsync_successData *response)
{
	UNUSED(context);
	UNUSED(response);
	fprintf(stderr,"Subscribe success\n");
	subscribed=1;
}

void onSubscribeFailure(void *context,MQTTAsync_failureData *response)
{
	UNUSED(context);
	UNUSED(response);
	fprintf(stderr,"Subscribe fail, rc %d\n",response?response->code:0);
	subscribed=0;
}

void onConnect(void *context,MQTTAsync_successData *response)
{
	UNUSED(response);
	fprintf(stderr,"connected to broker\n");
	connected=1;
	waitToExit=0;

	MQTTAsync client=(MQTTAsync)context;
	MQTTAsync_responseOptions opts=MQTTAsync_responseOptions_initializer;

	printf("Subscribing\n");
	opts.onSuccess=onSubscribe;
	opts.onFailure=onSubscribeFailure;
	opts.context=client;
	if(MQTTAsync_subscribe(client,topicIn,QOS,&opts)!=MQTTASYNC_SUCCESS)
		printf("Failed to start subscribe\n");

}
void onConnectFailure(void *context,MQTTAsync_failureData *response)
{
	UNUSED(context);
	UNUSED(response);
	fprintf(stderr,"Unable to connect to broker\n");
	connected=0;
	usleep(500000);
	doConnect();
}

void onDisconnect(void *context,MQTTAsync_successData *response)
{
	UNUSED(context);
	UNUSED(response);
	fprintf(stderr,"disconnected from broker\n");
	connected=0;
	waitToExit=0;
}

void doConnect()
{
	int rc=0;
	fprintf(stderr,"Connecting to %s\n",connString);
	MQTTAsync_connectOptions conn_opts=MQTTAsync_connectOptions_initializer;
	conn_opts.keepAliveInterval=20;
	conn_opts.cleansession=1;
	conn_opts.username="envysoft";
	conn_opts.password="Minair_17";
	conn_opts.onSuccess=onConnect;
	conn_opts.onFailure=onConnectFailure;
	conn_opts.context=client;
	while((rc=MQTTAsync_connect(client,&conn_opts))!=0)
	{
		fprintf(stderr,"MQTTClient_connect failed, return code %d\n",rc);
		sleep(2);
	}
}

void createCient()
{
	const char *clientId="VMC";
	if(MQTTAsync_create(&client,connString,clientId,MQTTCLIENT_PERSISTENCE_NONE,NULL)!=MQTTASYNC_SUCCESS)
	{
		fprintf(stderr,"error creating instance of client\n");
		exit(1);
	}
	MQTTAsync_setCallbacks(client,client,connLost,msgarrvd,deliveryComplete);
}


void connLost(void* context,char *cause)
{
	UNUSED(context);
	fprintf(stderr,"\nConnection lost\n");
	if(cause)
		fprintf(stderr,"     cause: %s\n", cause);
	fprintf(stderr,"Reconnecting\n");
	doConnect();
}
void  Handler(int signo)
{
	if(signo==2)
		finito=1;
}


int main(int argc, char **argv)
{
	const char *urlDefault="127.0.0.1";
	const int portDefault=1883;


	sprintf(connString,"tcp://%s:%d",argc>1?argv[1]:urlDefault,argc>2?atoi(argv[2]):portDefault);
	// Exception handling:ctrl + c
    signal(SIGINT, Handler);
    signal(SIGQUIT, Handler);
    signal(SIGSEGV, segfault_sigaction);

	UBYTE buf1[15]={0x00,0xc9,0x00,0x4e,0x04,0x00,0x00,0x03,0x07,0x00,0x00,0x8b,0x00,0x9e,0x54};
	UBYTE buf2[15]={0x00,0xc9,0x00,0x4e,0x04,0x00,0x00,0x03,0x07,0x01,0x00,0x8b,0x00,0x9f,0xa8};
	UBYTE buf3[15]={0x00,0xc9,0x00,0x4e,0x04,0x00,0x00,0x03,0x07,0x02,0x00,0x8c,0x00,0x9d,0xdc};

	UBYTE bufoff[15]={0x00,0xc9,0x00,0x4e,0x04,0x00,0x00,0x03,0x02,0x02,0x00,0x91,0x00,0x58,0x8c};
	UBYTE bufon[15]={0x00,0xc9,0x00,0x4e,0x04,0x00,0x00,0x03,0x02,0x00,0x00,0x99,0x00,0x5e,0xf4};
	struct tcommands{const char *name;UBYTE *bytes;double setTime;} commands[5]=
	{
		{.name="OFF",.bytes=bufoff,.setTime=0},
		{.name="ON",.bytes=bufon,.setTime=0},
		{.name="SPEED 1",.bytes=buf1,.setTime=0},
		{.name="SPEED 2",.bytes=buf2,.setTime=0},
		{.name="SPEED 3",.bytes=buf3,.setTime=0}
	};

	createCient();
	doConnect();
	time_t start=time(NULL);
	while((subscribed==0)&&(time(NULL)-start<5))	//5 seconds to subscribe
		usleep(10000L);
	if(!subscribed)
	{
		if(connected)
			fprintf(stderr,"timeout Subscribing\n");
		else
			fprintf(stderr,"timeout connecting\n");
		exit(1);
	}

	if(DEV_ModuleInit()==1)
		return 1;

	DEV_UART_Init("/dev/ttySC0");
    UART_Set_Baudrate(9600);
	UBYTE pData[1000]={0};

	uint8_t pattern[5]={0x00,0xc9,0x00,0x4e,0x04};
	uint32_t n=0;
	char v[10];

	while(!finito)
	{

		if(subscribed)
		{
			for(uint32_t i=0;i<N;i++)
			{
				if(vars[i].to_send>0)
				{
					float fv=vars[i].v;
					for(int k=0;k<vars[i].exp;k++)
						fv=fv/10;

					sprintf(v,"%.*f", vars[i].exp, fv);
//					fprintf(stderr,"Letto %s, val %s\n",vars[i].topic,v);
					sendMessage(client,vars[i].topic,v);
					vars[i].to_send=0;
				}
			}
		}

		int l=UART_Read_Bytes(&pData[n],sizeof(pData)-n);
		if(l<=0)
		{
			fprintf(stderr,"\nLetto %d\n",l);
			usleep(5000);
			continue;
		}

		n+=l;
		if((n>=32)&&(pData[12]>0)&&(pData[0]==pattern[0])&&(pData[1]==pattern[1])&&(pData[2]==pattern[2])&&(pData[3]==pattern[3])&&(pData[4]==pattern[4]))
		{
			parseBuffer(pData);

			if(sendValue>0)
			{
				if(((sendValue>2)&&(pData[32]==(sendValue-2)))
					||((sendValue==1)&&(pData[5]==2))
					||((sendValue==2)&&(pData[5]==0)))
				{
					fprintf(stderr,"SUCCESSO: %s\n",commands[sendValue-1].name);
					sendValue=0;
				}
			}

			if(n==35)
			{
				double now=ms();
				if(sendValue&&(now-commands[sendValue-1].setTime>500))	//rimando lo stesso valore solo dopo 500ms
				{
					usleep(t);
					fprintf(stderr,"INVIO COMANDO: %s\n",commands[sendValue-1].name);
					UART_Write_nByte(commands[sendValue-1].bytes,15);
					commands[sendValue-1].setTime=now;
				}
			}
		}


		for(uint32_t s=1;s<=n-PATTERN_LENGTH;s++)
		{
			if((pData[s]==pattern[0])&&(pData[s+1]==pattern[1])&&(pData[s+2]==pattern[2])&&(pData[s+3]==pattern[3])&&(pData[s+4]==pattern[4]))
			{
			//s contiene indice di inizio, stampo il precedente

				if(debug)
				{
					for(uint32_t u=0;u<s;u++)
						fprintf(stderr,"%02x ",pData[u]);
					fprintf(stderr,"\n");
				}

				for(uint32_t u=s;u<n;u++)
					pData[u-s]=pData[u];
				n-=s;
				break;
			}
		}

	}

	//System Exit
	fprintf(stderr,"\r\nHandler:Program stop\r\n");


	waitToExit=1;
	MQTTAsync_disconnectOptions opts=MQTTAsync_disconnectOptions_initializer;
	opts.onSuccess=onDisconnect;
	opts.context=client;
	MQTTAsync_disconnect(client,&opts);
	while(waitToExit)
		usleep(10000L);
	MQTTAsync_destroy(&client);

	DEV_ModuleExit();
    return 0;
}
