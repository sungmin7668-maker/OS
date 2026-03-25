# MLFQ Simulator

운영체제 과제로서 C언어로 구현한 **Multi-Level Feedback Queue (MLFQ)** CPU 스케줄링 시뮬레이터입니다.

## 개요

MLFQ는 여러 개의 우선순위 큐를 사용하여, 프로세스의 CPU 사용 패턴에 따라 동적으로 우선순위를 조정하는 스케줄링 알고리즘입니다. 이 시뮬레이터는 trace1.txt 파일에서 프로세스 정보를 읽어 MLFQ 스케줄링을 수행하고, 각 프로세스의 turnaround time과 response time을 출력합니다.

## 시뮬레이터 설정

| 항목 | 값 |
|------|-----|
| 큐 개수 | 3개 (q1, q2, q3) |
| q1 time slice | 10 |
| q2 time slice | 10 |
| q3 time slice | 20 |
| boost time | 50 (q2, q3 → q1) |
| 시간 최소 단위 | 1 |
| 최대 프로세스 수 | 100 |

## 입력 형식

`trace1.txt` 파일에서 프로세스 정보를 읽어옵니다.

```
PID  arrival_time  run_time  IO_start_time  IO_run_time
```

- **IO_start_time**: CPU 실행 시작 후 I/O가 발생하기까지의 시간
- **IO_run_time**: I/O 작업 수행 시간 (0이면 I/O 없음)

예시:
```
1 0 30 0 0      → PID 1, 0초 도착, 30초 실행, I/O 없음
3 1 45 5 10     → PID 3, 1초 도착, 45초 실행, 5초 후 I/O 시작하여 10초간 수행
```

## 구현 방식

### 자료구조

- **Process**: 연결 리스트 노드 형태. PID, 도착 시간, 실행 시간, I/O 정보 등을 포함
- **Queue**: head/tail 포인터 + time quantum. FIFO 방식의 enqueue/dequeue
- **IO**: I/O 작업 중인 프로세스를 관리. 잔여 I/O 시간과 복귀할 큐 정보를 저장

### 스케줄링 로직

1. 프로세스를 arrival_time 기준 오름차순으로 정렬하여 q1에 enqueue
2. 매 반복마다 q1 → q2 → q3 순으로 비어있지 않은 큐에서 프로세스를 dequeue
3. 실행 시간은 잔여 실행 시간과 time quantum 중 작은 값으로 결정
4. I/O 작업이 예정된 프로세스는 IO_start_time만큼만 CPU를 사용한 뒤 I/O로 진입

### 1초 단위 시뮬레이션

실행을 한 번에 처리하지 않고, for 루프를 통해 **1초 단위로** 시뮬레이션합니다. 매 tick마다:
- I/O 중인 프로세스의 잔여 시간 감소
- I/O 완료 시 해당 프로세스를 지정된 큐로 복귀
- CPU 실행과 I/O가 병렬로 동작

### I/O 처리

- I/O는 프로세스 생애에서 **1회만** 발생
- 각 프로세스의 I/O는 **개별적으로 동시에** 진행
- I/O 완료 후 복귀 큐: 해당 프로세스의 직전 CPU 사용 시간이 time slice보다 작으면 **같은 큐**, 아니면 **한 단계 강등**

### Boost

- current_time이 50의 배수가 될 때마다 q2, q3의 모든 프로세스를 q1으로 이동
- starvation 방지 목적

### 우선순위 처리

같은 시점에 I/O 완료와 boost가 동시에 발생하면 **I/O 완료를 먼저** 처리하며, 해당 프로세스를 먼저 복귀시킴

## 실행 방법

```bash
gcc -o mlfq mlfq.c
./mlfq
```

## 실행 결과 (trace1.txt)

```
All processes completed.
-------------------------------------
PID | Turnaround time | response time
1   | 195             | 0
2   | 129             | 19
3   | 284             | 29
4   | 132             | 67
5   | 120             | 10
6   | 104             | 34
7   | 314             | 39
8   | 263             | 58
9   | 231             | 71
10  | 364             | 49
-------------------------------------
final_completion_time : 365
```

## 파일 구조

```
├── README.md
├── Homework1.pdf    # 과제 명세서
├── mlfq.c           # MLFQ 시뮬레이터 소스 코드
├── mlfq             # 실행파일
├── trace1.txt       # 테스트 입력 파일
├── MEMO.txt         # 참고용
└── 
```
## 배운점

### GIT, GITHUB 사용법

- **git init** : 현재 디렉토리를 Git 저장소로 초기화 / **처음에 꼭 해야됨**
- **git add .**: 모든 변경된 파일을 스테이징 영역에 추가
- **git commit -m "~~"**: 스테이징된 변경사항을 커밋 메시지와 함께 저장 / **커밋 메시지는 동사로 시작 (Add, Remove, Fix 등)**
- **git branch -m master main**: 현재 브랜치 이름을 master에서 main으로 변경 / **초기 브랜치가 master여서 변경필요**
- **git remote add origin "(레포지토리주소)"**: 원격 저장소를 origin이라는 이름으로 연결
- **git remote -v**: 연결된 원격 저장소 주소 확인
- **git pull origin main --rebase**: 원격의 변경사항을 가져와서 로컬 커밋 위에 재배치
![git_pull_rebase]./images/git_pull_rebase.png
- **git push origin main**: 로컬 커밋을 원격 저장소의 main 브랜치에 업로드
- **git push origin main --force**: 로컬 상태를 원격에 강제로 덮어씌우기 **(협업 시 주의)**

### C언어 로직