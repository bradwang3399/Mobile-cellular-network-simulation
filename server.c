#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
int main(int argc, char **argv){

  struct sockaddr_in my_addr;
  struct sockaddr_in remote_addr;

  socklen_t addrlen = sizeof(remote_addr);

  char buffer[50];
  char acknowledge[8];
  char reject[10];

  char ack1 = 0xff, ack2 = 0xf2, rjct1=0xff, rjct2=0xf3, length=0x38;
  char rejctseq = 0xf4, rejctlen = 0xf5, rejctend = 0xf6, rejctdup = 0xf7;
  char segno[10] = {0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x40};
  char duplicate[10];
  int double_check_duplicate[10] = {0,0,0,0,0,0,0,0,0};
  int duplicate_flag=0, sequence_flag, length_flag;

  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  printf("server socket id:%d \n", sockfd);
  if(sockfd == -1){
    perror("failed to create socket");
    exit(EXIT_FAILURE);
  }

	memset((char *)&my_addr, 0, sizeof(my_addr));
  my_addr.sin_family = AF_INET;
  my_addr.sin_port = htons(12345);
  my_addr.sin_addr.s_addr = INADDR_ANY;

  int rc = bind(sockfd, (const struct sockaddr *)&my_addr, sizeof(my_addr));
  if(rc == -1){
    perror("failed to bind");
    close(sockfd);
    exit(EXIT_FAILURE);
  }

  int i=0, recvlen;
  while(i<10){

  printf("\n");
  recvlen = recvfrom(sockfd, (char *)buffer, 50, 0, (struct sockaddr *)&remote_addr, &addrlen);

    printf("packet: %02X received from client %02X \n", buffer[5], buffer[2]);

    if(buffer[5]==duplicate[5] || buffer[5]==duplicate[6] || buffer[5]==duplicate[7]
      || buffer[5]==duplicate[8] || buffer[5]==duplicate[9]){//check duplicate
        printf("packet %02X duplicated, sending reject packet to the client...\n", buffer[5]);
        reject[0]=buffer[0], reject[1]=buffer[1], reject[2]=buffer[2], reject[3]=rjct1, reject[4]=rjct2,
        reject[5]=rjct1, reject[6]=rejctend, reject[7]=buffer[5], reject[8]=buffer[15], reject[9]=buffer[16];
        sendto(sockfd, (char *)reject, strlen(reject), 0, (struct sockaddr *)&remote_addr, sizeof(remote_addr));
        if(i>4){
          duplicate[i]=buffer[5];
          double_check_duplicate[i]=1;
          duplicate_flag=i;
        }
      }

      else if(buffer[5] != segno[i]){
        printf("packet %02X not sent in correct sequence, sending reject packet to the client...\n", buffer[5]);
        reject[0]=buffer[0], reject[1]=buffer[1], reject[2]=buffer[2], reject[3]=rjct1, reject[4]=rjct2,
        reject[5]=rjct1, reject[6]=rejctseq, reject[7]=buffer[5], reject[8]=buffer[15], reject[9]=buffer[16];
        sendto(sockfd, (char *)reject, strlen(reject), 0, (struct sockaddr *)&remote_addr, sizeof(remote_addr));
        if(i>4){
          duplicate[i]=buffer[5];
          sequence_flag=i;
        }
      }

     else if(buffer[6] != length){
       printf("packet %02X not in correct length, sending reject packet to the client...\n", buffer[5]);
       reject[0]=buffer[0], reject[1]=buffer[1], reject[2]=buffer[2], reject[3]=rjct1, reject[4]=rjct2,
       reject[5]=rjct1, reject[6]=rejctlen, reject[7]=buffer[5], reject[8]=buffer[15], reject[9]=buffer[16];
       sendto(sockfd, (char *)reject, strlen(reject), 0, (struct sockaddr *)&remote_addr, sizeof(remote_addr));
       if(i>4){
         duplicate[i]=buffer[5];
         length_flag=i;
       }
     }

     else if(buffer[16] != 0xffffffff){
       printf("packet %02X not in correct end id, sending reject packet to the client...\n", buffer[5]);
       reject[0]=buffer[0], reject[1]=buffer[1], reject[2]=buffer[2], reject[3]=rjct1, reject[4]=rjct2,
       reject[5]=rjct1, reject[6]=rejctend, reject[7]=buffer[5], reject[8]=buffer[15], reject[9]=buffer[16];
       sendto(sockfd, (char *)reject, strlen(reject), 0, (struct sockaddr *)&remote_addr, sizeof(remote_addr));
       if(i>4)
       duplicate[i]=buffer[5];
     }

     else{
       printf("packet %02X received from client, sending acknowledge packet back to client...\n", buffer[5]);
       acknowledge[0]=buffer[0], acknowledge[1]=buffer[1], acknowledge[2]=buffer[2], acknowledge[3]=ack1,
       acknowledge[4]=ack2, acknowledge[5]=buffer[5], acknowledge[6]=buffer[15], acknowledge[7]=buffer[16];
       sendto(sockfd, (char *)acknowledge, strlen(acknowledge), 0, (struct sockaddr *)&remote_addr, sizeof(remote_addr));
       duplicate[i]=buffer[5];
      }
    i++;


  }

  close(sockfd);
  return 0;
}
