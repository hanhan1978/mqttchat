#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <getopt.h>
#include <MQTTClient.h>
#include <jansson.h>
#include <pthread.h>

#include "app.h"

struct userinfo{
  char * username;
  char * appid;
} uinfo;
void show_usage();
int set_cmd_option(int argc, char **argv) ;
struct  userinfo * ui; 
int my_readline(MQTTClient* client);
void myconnect(MQTTClient* client);
void print_message(char * text);
void * mqtt_receive(void *ptr);

int main(int argc, char *argv[]){
  ui = (struct userinfo *)malloc(sizeof(struct userinfo));
  set_cmd_option(argc, argv);
  MQTTClient client;


  pthread_t thread1;
  int  iret1;
  iret1 = pthread_create( &thread1, NULL, mqtt_receive, (void *)client);
  my_readline(client);
  pthread_join(thread1 , NULL); 
  return 1;
}

void * mqtt_receive(void *ptr){
  MQTTClient client;
  client = (MQTTClient *) ptr;
  int rc = 0;
  char url[100];
  char mess[1024];
  sprintf(url , "%s.mlkcca.com:1883", ui->appid);
  rc = MQTTClient_create(&client, url, "mchat client" , MQTTCLIENT_PERSISTENCE_NONE, NULL);
  myconnect(&client);
  rc = MQTTClient_subscribe(client, "teaidk6e6vv/message/push", 0);

  while(1){
    char * topicName = NULL;
    int topicLen;
    MQTTClient_message* message = NULL;
    rc = MQTTClient_receive(client, &topicName, &topicLen, &message, 1000);
    if (message){
      sprintf(mess, "%.*s", message->payloadlen, (char*)message->payload);
      print_message(mess);
    }
    if (rc != 0) myconnect(&client);
  }
}


int my_readline(MQTTClient * client) {
  char prompt[200];
  sprintf(prompt, "%s > ", ui->username);
  char * line;

  while(1){
    if ((line = readline(prompt)) == NULL) {
      return 0;
    }
  }
  return 1;
}


void print_message(char * text){
    json_t *root;
    json_error_t error;
    root = json_loads(text, 0, &error);
    json_t *data, *message, *user;
    data = json_object_get(root, "params");
    message = json_object_get(data, "content");
    user = json_object_get(data, "user");
    printf("\n\x1b[33m%s > %s\n\x1b[0m", json_string_value(user), json_string_value(message));
}



int set_cmd_option(int argc, char **argv) {
  int c;
  while (1){
  struct option long_options[] =
    {
      {"appid",    required_argument, 0, 'a'},
      {"user",     required_argument, 0, 'u'},
      {0, 0, 0, 0}
    };
    int option_index = 0;
    c = getopt_long (argc, argv, "a:u:", long_options, &option_index);
    if (c == -1) break;
    switch (c){
      case 'u':
        ui->username = optarg;
        break;
      case 'a':
        ui->appid = optarg;
        break;
      default:
        show_usage();
        exit(1);
        break;
    }
  }
  if(ui->username == NULL || ui->appid == NULL){
    show_usage();
    exit(0);
  }
  return 1;
}

void show_usage(){
    printf("Usage: mchat -u {username} -a {appid}\n");
    printf("required arguments:\n");
    printf("    -u     username\n");
    printf("    -a     appid\n");
    exit(1);
}

void myconnect(MQTTClient* client)
{
  MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
	conn_opts.keepAliveInterval = 10;
	conn_opts.reliable = 0;
	conn_opts.cleansession = 0;
	conn_opts.username = "j{JSON Web Token}";
	conn_opts.password = ui->appid;
  //printf("appid -> %s \n", ui->appid);
	int rc = 0;
	if ((rc = MQTTClient_connect(*client, &conn_opts)) != 0)
	{
		printf("Failed to connect, return code %d\n", rc);
		exit(-1);	
	}
}
