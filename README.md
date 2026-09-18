# tiny-web-server-c

AI 없이 C로 웹서버를 만들어보자.


윈도우에서 gcc를 어케까는지를 모르니까 걍 wsl 을 쓰자

wsl --install 을 걸어두고 우선 흐름은 먼저 정리해봄.

1. 소켓을 만든다. (일단 무언가로 통신을 해야하니까)
2. 방금 만든 소켓에다가 ip주소랑 포트번호 (주소) 를 bind 한다.
3. bind 한 소켓을 listen() 한다. 
4. listen 하고있다가 클라이언트가 요청 시도를 하면 accept() 한다. (여기가 루프 돌면 될거같다.)

우선 소켓 생성, bind, listen을 반복하고 accept로 연결된 소켓을 만드는것 까지를 첫 목표로 하자.


# 참고한 블로그
https://devocean.sk.com/blog/techBoardDetail.do?ID=165560

https://tjcplpllog.blogspot.com/2015/12/c-http.html

https://seongmok.com/57

https://just-live.tistory.com/entry/CS%EA%B8%B0%EC%B4%88-%EC%86%8C%EC%BC%93-%ED%94%84%EB%A1%9C%EA%B7%B8%EB%9E%98%EB%B0%8D-%ED%95%B5%EC%8B%AC-%EA%B0%9C%EB%85%90-%EC%A0%95%EB%A6%AC-socket-connect-bind-listen-accept

https://man7.org