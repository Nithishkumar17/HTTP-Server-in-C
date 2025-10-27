#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include<arpa/inet.h>
#include<unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>
#include <signal.h>

#define LISTEN_BACKLOG 100
#define ROOT "/mnt/d/PROJECT/VS"
#define index "/index.html"

#define RESPONSE_200 "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"


char* fetch_content(char *path){
    char* dir = malloc(strlen(ROOT)+strlen(path)+strlen(index)+1);
    int i=0;
    strcpy(dir, ROOT);

    strcat(dir, path);
    strcat(dir, index);

    printf("Fetching file from %s\n", dir);
    FILE *f = fopen(dir, "r");
    if(!f){
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    long fsize =ftell(f);
    fseek(f, 0, SEEK_SET);

    char *string = malloc(fsize + 2);
    fread(string, 1, fsize, f);
    fclose(f);
    string[fsize] = '\n';
    string[fsize+1] = '\0';
    free(dir);
    return string;
}

void handle_write(int cfd, const char *buffer, size_t length) {
    size_t total_written = 0;
    while (total_written < length) {
        ssize_t written = write(cfd, buffer + total_written, length - total_written);
        if (written < 0) {
            perror("write failed");
            return;
        }
        total_written += written;
    }
}

void* handle_client(int* p_cfd){
    int cfd= *p_cfd;
    free(p_cfd);
    char buffer[1000];
    ssize_t rd;

    rd= read(cfd, buffer, sizeof(buffer)-1);
    if(rd<0){
        perror("read failed");
    }
    buffer[rd]='\0';
    
    int idx;
    idx=0;
    //http action
    while (buffer[idx]!=' ')
    {
        idx++;
    }
    char action[idx+2];
    strncpy(action,buffer,idx);
    action[idx]='\0';
    idx+=2;

    //http path
    int curr;
    curr=idx;
    while (buffer[idx]!=' ')
    {
        idx++;
    }
    char path[idx+2];
    strncpy(path,buffer+curr-1,idx-curr+1);
    path[idx-curr+1]='\0';

    char* message = fetch_content(path);

    if(message == NULL){
        handle_write(cfd, "HTTP/1.1 404 Not Found\r\n", 24);
        handle_write(cfd, "Content-Type: text/html\r\n", 25);
        handle_write(cfd, "\r\n", 2);
        handle_write(cfd, "<html><body><h1>404 Not Found</h1></body></html>\n", 49);
        close(cfd);
        return NULL;
    }

    char content_length[100];
    snprintf(content_length, sizeof(content_length), "Content-Length: %zu\r\n", strlen(message));
    handle_write(cfd, "HTTP/1.1 200 OK\r\n", 17);
    handle_write(cfd, "Content-Type: text/html\r\n", 25);
    handle_write(cfd, content_length, strlen(content_length));
    handle_write(cfd, "Connection: close\r\n", 19);
    handle_write(cfd, "\r\n", 2);
    handle_write(cfd, message, strlen(message));
    free(message);
    printf("%d client served \n", cfd);
    close(cfd);

    return NULL;

}

int main(void){

    int                 sfd, cfd, bnd, lst;
    struct sockaddr_in  addr_bind, peer_addr;
    socklen_t           peer_addr_size;
    ssize_t             rd;

    signal(SIGPIPE, SIG_IGN);
    memset(&addr_bind, 0, sizeof(addr_bind));
    addr_bind.sin_family = AF_INET;
    addr_bind.sin_port   = htons(9000);
    addr_bind.sin_addr.s_addr   = INADDR_ANY;

    //create socket
    sfd= socket(AF_INET, SOCK_STREAM, 0);
    if(sfd<0){
        perror("socket failed");
    }

    int opt = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    //bind socket
    bnd = bind(sfd, (struct sockaddr*) &addr_bind, sizeof(addr_bind));
    if(bnd<0){
    perror("bind failed");
    }

    //start listening
    lst = listen(sfd,LISTEN_BACKLOG);
    if(lst<0){
        perror("listen failed");
    }

    peer_addr_size = sizeof(peer_addr);

    //start client fd  
    while(1){
        cfd = accept(sfd, (struct sockaddr*) &peer_addr, &peer_addr_size);
        if(cfd == -1){
            perror("accept");
        }
        int *p_cfd = malloc(sizeof(int));
        *p_cfd = cfd;

        pthread_t thread_id;

        if(pthread_create(&thread_id, NULL, (void *)handle_client, p_cfd) != 0){
            perror("Failed to create thread");
            free(p_cfd);
            continue;
        }
        else{
        pthread_detach(thread_id);
        }
    }
    printf("escaped \n");
    close(sfd);
    printf("success \n");
    return 0;
}