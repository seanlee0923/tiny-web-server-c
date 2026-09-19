#include <sys/socket.h> 
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    // 1. 소켓을 만들기
    // 리눅스 매뉴얼에서 socket(2) 를 보니까 
    // int socket(int domain, int type, int protocol); 임
    // domain은 난 ipv4 쓸거니 AF_INET 을 넘기고,
    // http 서버 만들거니까 tcp 랑 주로 매핑된단느 SOCK_STREAM 
    // 마지막 protocol 은 0을 입력하면 운영체제에 맞는 기본 프로토콜을 넘김
    int base_socket = socket(AF_INET, SOCK_STREAM, 0);

    // socket(2) 의 반환이 정수형이네;;
    // 리눅스 매뉴얼을 볼일이 없어서 몰랐는데 socket(2) 은 소켓을 리턴하는게 아니라 
    // 파일 디스크립터 번호를 리턴을 해주는거였음
    if (base_socket == -1) {
        // 소켓 만드는데 실패시 -1 을 리턴함
        // 일단 리턴으로 종료
        return 0;
    } 

    // 우선 주소는 내 로컬로 걍 박아서 쓰기.
    // htons랑 htonl 이 내 주소를 바이트로 바꾸는거
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = 12345;
    server_addr.sin_addr.s_addr = INADDR_ANY;


    // 2. 바인드 하자
    // bind(2) 도 리눅스 매뉴얼에 친절화게 설명이 되어있음
    // int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
    // 아까 만든 소켓 응답값(디스크립터 번호), 쓸 주소(아이피랑 포트), 주소 정보의 크기
    // server_addr 은 sockaddr_in 인데 bind(2)에서는 sockaddr 이 필요하다.
    // 그래서 이렇게 캐스팅? 을 해서 넘겨버림. 
    int bind_result = bind(base_socket, (struct sockaddr *)&server_addr , sizeof(server_addr));
    if (bind_result == -1) {
        return 0;
    }

    // 3. 이제 listen
    // int listen(int sockfd, int backlog);
    // baklog 는 accept(2)로 처리가 안된 요청의 큐 크기를 지정하는거임 
    int listen_result = listen(base_socket, 10);
    if (listen_result == -1) {
        perror("listen failed");
        exit(listen_result);
        return 0;
    }
    


    // 4. accept 하기
    // 여기가 이제 가장 중요한 부분이 될거같다.
    // 일단 읽기위한 리더 버퍼
    char reader_buffer[1024];
    // 클라이언트 소켓
    struct sockaddr_in client_addr;
    // accept 에서는 포인터로 받아가지고 변수를 만든담에 &로 넘기기
    int client_addr_len = sizeof(client_addr);

    printf("run server ! \n");
    while(1) {
        // int accept(int sockfd, struct sockaddr *_Nullable restrict addr, socklen_t *_Nullable restrict addrlen);
        // 내 소켓(서버)fd랑, accept할 주소, 주소 사이즈를 넘기는거
        // Nullable 저거만 빼면 bind 랑 스펙이 거의 같아보인다.
        int client_socket = accept(base_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_socket == -1) {
            // 수락 실패하면 다음 요청을 받자
            // 실패해도 큐에 안쌓이고 알아서 지워지나? 이거도 검색해봐야함
            continue;
        }

        printf("connected ! \n");

        // 읽기
        int n = read(client_socket, reader_buffer, 1024);
        if (n == -1) {
            // 읽기에 실패한거
            continue;
        }
        if ( n == 0) {
            // EOF 면 0 을 리턴
            continue;
        }

    }


}