# Computer Networks 과제 구현

이 리포지토리에는 네트워크 프로토콜 실습을 위한 두 개의 Machine Problem(MP)이 포함되어 있다. 각 MP는 C++로 작성되었으며, 핵심 구현 내용 위주로 README를 구성하였다.

---

## 디렉토리 구조

```
├── MP1_CRC/
│   ├── crc_encoder_<학번>.cc    # CRC 인코더 소스
│   ├── crc_decoder_<학번>.cc    # CRC 디코더 소스
│   └── linksim                  # 채널 시뮬레이터 바이너리
│
├── MP2_Routing/
│   ├── distvec_<학번>.cc        # Distance Vector 알고리즘 구현
│   ├── linkstate_<학번>.cc      # Link State 알고리즘 구현
│   ├── topology.txt             # 네트워크 토폴로지 입력 예시
│   ├── messages.txt             # 전송 메시지 입력 예시
│   └── changes.txt              # 링크 비용 변경 입력 예시
```

---

## MP1: CRC (Cyclic Redundancy Check)

### 구현 내용

1. **CRC 인코더 (crc\_encoder)**

   * 입력 파일로부터 비트스트림을 읽고, 지정한 생성 다항식으로 프레임 분할 및 CRC 부호 계산
   * `dataword_size` 단위(4비트 또는 8비트)로 처리
   * 계산된 CRC를 데이터워드에 부착하여 출력 스트림 생성

2. **CRC 디코더 (crc\_decoder)**

   * 수신된 코드워드 스트림을 `dataword_size` 단위로 분할
   * 각 코드워드에 대해 CRC 검증 수행
   * 오류 검출 시 해당 프레임을 카운트하고, 오류 없는 프레임은 원래 데이터워드 복원
   * 결과 파일에 `<총 프레임 수> <오류 프레임 수>` 기록

### 사용 방법

```bash
cd MP1_CRC

# 컴파일
g++ -o crc_encoder_<학번> crc_encoder_<학번>.cc
g++ -o crc_decoder_<학번> crc_decoder_<학번>.cc

# 인코딩
./crc_encoder_<학번> input.dat coded.dat <generator> <dataword_size>

# 채널 시뮬레이션
./linksim coded.dat received.dat <error_ratio> <random_seed>

# 디코딩
./crc_decoder_<학번> received.dat output.dat result.txt <generator> <dataword_size>
```

* `<generator>`: 예) `1101`
* `<dataword_size>`: `4` 또는 `8`
* `<error_ratio>`: `0.0` \~ `1.0`
* `result.txt`: 첫 줄에 `총_프레임_수 오류_프레임_수` 출력

---

## MP2: Routing Protocols

### 구현 내용

1. **Distance Vector (distvec)**

   * 벨만-포드(Bellman-Ford) 기반 라우팅 테이블 계산
   * 네트워크 토폴로지 파일 파싱
   * 초기 테이블 구성 후, 주어진 변경 내역(`changes.txt`)마다 업데이트 수행
   * 수렴 시점마다 라우팅 테이블 출력
   * 메시지 전송 시 최단 경로 탐색 및 결과 출력
   * 비용 동률 시 다음 홉 ID가 작은 노드 선택

2. **Link State (linkstate)**

   * Dijkstra 알고리즘으로 전체 토폴로지 최단 경로 계산
   * 변경 내역 적용 후 재계산 및 테이블 갱신
   * 메시지 전송 결과 출력 방식은 DV와 동일
   * 경로 동률 시 노드 ID 우선순위 적용

### 사용 방법

```bash
cd MP2_Routing

# 컴파일
g++ -o distvec_<학번> distvec_<학번>.cc -Wall
g++ -o linkstate_<학번> linkstate_<학번>.cc -Wall

# 실행 (DV)
./distvec_<학번> topology.txt messages.txt changes.txt

# 실행 (LS)
./linkstate_<학번> topology.txt messages.txt changes.txt
```

* **입력 파일**

  * `topology.txt`: 노드 수 및 링크 정보 (노드A 노드B 비용)
  * `messages.txt`: `발신지 목적지 메시지`
  * `changes.txt`: `노드A 노드B 새로운비용`

* **출력 파일**

  * DV → `output_dv.txt`
  * LS → `output_ls.txt`

* **출력 형식**

  1. 수렴된 라우팅 테이블 (변경 전후 스텝별)

     ```
     <노드ID> <목적지ID> <최단거리> <다음홉>
     ```
  2. 메시지 전송 로그

     ```
     from <x> to <y> cost <path_cost> hops <경로 리스트> message <내용>
     ```

---

## 주요 알고리즘/구조

* CRC 연산: 비트 배열 순환 이진 나눗셈
* DV: 벨만-포드 반복 업데이트, 수렴 확인
* LS: 다익스트라 + 우선순위 큐
* 파일 입출력: 스트림 기반 파싱 및 예외 처리

---

