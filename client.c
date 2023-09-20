#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>

int main(){

  struct sockaddr_in my_addr;
  struct sockaddr_in remote_addr;
  char buffer[50];

  char start1 = 0xff, start2 = 0xff, client_id = 0x01, data1 = 0xff, data2 = 0xf1, length = 0x38, end1 = 0xff, end2 = 0xff;
  char segno1 = 0x31, segno2 = 0x32, segno3 = 0x33, segno4 = 0x34, segno5 = 0x35;
  char segno6 = 0x36, segno7 = 0x37, segno8 = 0x38, segno9 = 0x39, segno10 = 0x40;

  char payload1_1 = 0x70, payload1_2 = 0x61, payload1_3 = 0x63, payload1_4 = 0x6b,
  payload1_5 = 0x65, payload1_6 = 0x74, payload1_7 = 0x20, payload1_8 = 0x31; // translated as: packet 1
  char payload2_1 = 0x70, payload2_2 = 0x61, payload2_3 = 0x63, payload2_4 = 0x6b,
  payload2_5 = 0x65, payload2_6 = 0x74, payload2_7 = 0x20, payload2_8 = 0x32; // translated as: packet 2
  char payload3_1 = 0x70, payload3_2 = 0x61, payload3_3 = 0x63, payload3_4 = 0x6b,
  payload3_5 = 0x65, payload3_6 = 0x74, payload3_7 = 0x20, payload3_8 = 0x33; // translated as: packet 3
  char payload4_1 = 0x70, payload4_2 = 0x61, payload4_3 = 0x63, payload4_4 = 0x6b,
  payload4_5 = 0x65, payload4_6 = 0x74, payload4_7 = 0x20, payload4_8 = 0x34; // translated as: packet 4
  char payload5_1 = 0x70, payload5_2 = 0x61, payload5_3 = 0x63, payload5_4 = 0x6b,
  payload5_5 = 0x65, payload5_6 = 0x74, payload5_7 = 0x20, payload5_8 = 0x35; // translated as: packet 5
  char false_length = 0x40, false_end = 0xf1;

  char *data_packet[10] = {(char[]){start1, start2, client_id, data1, data2, segno1, length, payload1_1, payload1_2,
    payload1_3, payload1_4, payload1_5, payload1_6, payload1_7, payload1_8, end1, end2},
  (char[]){start1, start2, client_id, data1, data2, segno2, length, payload2_1, payload2_2,
    payload2_3, payload2_4, payload2_5, payload2_6, payload2_7, payload2_8, end1, end2},
  (char[]){start1, start2, client_id, data1, data2, segno3, length, payload3_1, payload3_2,
    payload3_3, payload3_4, payload3_5, payload3_6, payload3_7, payload3_8, end1, end2},
  (char[]){start1, start2, client_id, data1, data2, segno4, length, payload4_1, payload4_2,
    payload4_3, payload4_4, payload4_5, payload4_6, payload4_7, payload4_8, end1, end2},
  (char[]){start1, start2, client_id, data1, data2, segno5, length, payload5_1, payload5_2,
    payload5_3, payload5_4, payload5_5, payload5_6, payload5_7, payload5_8, end1, end2},
  (char[]){start1, start2, client_id, data1, data2, segno7, length, payload1_1, payload1_2,
    payload1_3, payload1_4, payload1_5, payload1_6, payload1_7, payload1_8, end1, end2},
  (char[]){start1, start2, client_id, data1, data2, segno7, length, payload1_1, payload1_2,
    payload1_3, payload1_4, payload1_5, payload1_6, payload1_7, payload1_8, end1, end2},
  (char[]){start1, start2, client_id, data1, data2, segno8, false_length, payload1_1, payload1_2,
    payload1_3, payload1_4, payload1_5, payload1_6, payload1_7, payload1_8, end1, end2},
  (char[]){start1, start2, client_id, data1, data2, segno9, length, payload1_1, payload1_2,
    payload1_3, payload1_4, payload1_5, payload1_6, payload1_7, payload1_8, end1, false_end},
  (char[]){start1, start2, client_id, data1, data2, segno10, length, payload1_1, payload1_2,
    payload1_3, payload1_4, payload1_5, payload1_6, payload1_7, payload1_8, end1, end2}};

  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

  if(sockfd == -1){
    perror("failed to create socket");
    exit(EXIT_FAILURE);
  }

	memset((char *) &remote_addr, 0, sizeof(remote_addr));
  remote_addr.sin_family = AF_INET;
  remote_addr.sin_port = htons(12345);
  remote_addr.sin_addr.s_addr = INADDR_ANY;
  socklen_t len = 0;

  struct timeval ack_clock;
  ack_clock.tv_sec = 3;
  ack_clock.tv_usec = 0;
  setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&ack_clock, sizeof ack_clock);

  int count = 0;

  for(int i=0; i<10; i++){

    int send1 = sendto(sockfd, (const char *)data_packet[i], strlen(data_packet[i]), 0, (const struct sockaddr *)&remote_addr, sizeof(remote_addr));
    if(send1 == -1){
      perror("failed to send");
    }

    printf("\npacket %0X sent\n", data_packet[i][5]);
    if(recvfrom(sockfd, (char *)buffer, 50, 0, (struct sockaddr *)&remote_addr, &len) < 0){
      printf("sent failed, retry...\n");
      while(count<2){
        int send1 = sendto(sockfd, (const char *)data_packet[i], strlen(data_packet[i]), 0, (const struct sockaddr *)&remote_addr, sizeof(remote_addr));
        if(recvfrom(sockfd, (char *)buffer, 50, 0, (struct sockaddr *)&remote_addr, &len) < 0){
            printf("sent failed, retry...\n");
            count++;
          }
      }
    }

    if(count==2){
      printf("server does not respond..\n");
      return 0;
    }

    if(buffer[4] == 0xFFFFFFF2){
      printf("sent succeed, acknowledge packet received from packet %0X\n", buffer[5]);

    }

    else if(buffer[4] == 0xFFFFFFF3){
      printf("sent failed, rejected by the server\n");


    }
  }

  close(sockfd);
  return 0;
}
