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
        perror("create socket failed");
        exit(base_socket);
        return 0;
    }

    // 우선 주소는 내 로컬로 걍 박아서 쓰기.
    // htons랑 htonl 이 내 주소를 바이트로 바꾸는거
    // 이거 왜 14640으로 열리지?? sin_port 를 12345 로 하니까 14640으로 열리고 14640으로 하니까 12345로 열림 (???? 반전술식인가)
    // 반전술식이 맞았다 ㅋㅋㅋㅋ 바이트 순서가 반대였구만
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(12345); // 바이트 정렬을 빅 엔디안으로 맞추기
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 이건 어차피 0이라 상관없지만 일관성위해 맞추기


    // 2. 바인드 하자
    // bind(2) 도 리눅스 매뉴얼에 친절화게 설명이 되어있음
    // int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
    // 아까 만든 소켓 응답값(디스크립터 번호), 쓸 주소(아이피랑 포트), 주소 정보의 크기
    // server_addr 은 sockaddr_in 인데 bind(2)에서는 sockaddr 이 필요하다.
    // 그래서 이렇게 캐스팅? 을 해서 넘겨버림.
    int bind_result = bind(base_socket, (struct sockaddr *)&server_addr , sizeof(server_addr));
    if (bind_result == -1) {
        perror("bind failed");
        exit(bind_result);
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
    socklen_t client_addr_len = sizeof(client_addr);

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
        // ssize_t read(int fd, void *buf, size_t count);
        // 디스크립터 번호, 보낼 버퍼 시작주소, 버퍼 사이즈 를 순서대로 쓴다.
        // 매뉴얼 패이지에서는 site_t count; buf[count] 로 되어있는데 실제 호출부는 위에처럼 되어있다.
        int n = read(client_socket, reader_buffer, 1024);
        if (n == -1) {
            printf("failed to read ... do next loop");
            // 읽기에 실패한거
            continue;
        }
        if ( n == 0) {
            // EOF 면 0 을 리턴
            printf("EOF ... do next loop");
            continue;
        }
        printf("write message to socket\n");

        // 이거 길이 구할수 있도록 해야하는데 일단 랜더링을 먼저 해보자
        char resp[] = "HTTP/1.0 200 OK\r\n"
            "Server: myserver\r\n"
            "Content-type: text/html\r\n"
            "Content-length: 172\r\n\r\n"
            "<html><head><title>my server</title></head>\r\n"
            "<body><h1>Hello World</h1></body>\r\n"
            "</html>\r\n";
        // 이거 짜증나는게 헤더랑 바디 사이에 \r\n 이 없으니까 랜더링이 안됨
        // http 응답할때 구조 자체가 헤더 \r\n\r\n 바디 라서\r\n 하나만 하니까 헤더에 저 html 뭉탱이도 같이 들어가버렸음;;

        // 이제 쓰면 된다.
        // ssize_t write(int fd, const void *buf, size_t count);
        // 파일 디스크립터 번호, 쓸 메시지 첫 주소, 쓸 메시지 길이이다.
        // 첫 주소라고 하니 뭔가 어색해보이는데 그냥 배열 시작하는 주소를 넘기면 되는거라
        // resp 로 넘기기
        int write_len = write(client_socket, resp, strlen(resp));

        printf("close socket\n");

        close(client_socket);
    }


}
