# Embedded Linux IPC Framework

![Language](https://img.shields.io/badge/Language-C-00599C?style=flat-square&logo=c&logoColor=white)
![Platform](https://img.shields.io/badge/Platform-Linux-FCC624?style=flat-square&logo=linux&logoColor=black)
![Architecture](https://img.shields.io/badge/Architecture-Multi--Process-brightgreen?style=flat-square)

## 📖 Overview

이 프로젝트는 리눅스 임베디드 시스템 환경에서 멀티 프로세스 아키텍처(Multi-Process Architecture)와 IPC(Inter-Process Communication)를 실험하고 고도화하기 위해 설계된 프레임워크입니다.

초기 버전은 **SysV Message Queue**와 **Fork/Exec** 모델을 기반으로 구현되었으며, 이를 바탕으로 최신 임베디드 소프트웨어 트렌드(Modern IPC, Rust, Systemd 등)를 단계적으로 적용하며 학습하는 것과 프레임워크화 하여 실제 개발 시 이 프로젝트의 아키텍처를 그대로 써도 무방할 정도의 완성도를 목표로 합니다.

## 🏗 System Architecture

시스템은 Supervisor Pattern을 따릅니다. 메인 프로세스가 시스템의 생명주기를 관리하며, 각 기능을 담당하는 독립적인 매니저 프로세스들이 비동기 메시징을 통해 통신합니다.

## ⚙️ Implementation Details

### 1. IPC 메시지 구조

모든 통신은 `IpcMessage` 구조체를 따릅니다.
* `mtype (long)`: 수신 대상을 지정하는 필드 (예: `TYPE_LED_MANAGER`).
* `command (int)`: 실제 수행할 명령어 (`CMD_ON`, `CMD_SHUTDOWN` 등).
* `payload (char[MAX_MSG_SIZE])`: 데이터 전송을 위한 가변 필드.

### 2. 모듈 분리: IPC 핸들러, 하드웨어 추상화 (HAL)

매니저 코드는 프로세스 로직과 하드웨어 제어를 명확히 분리합니다. *.c 파일은 각각 정해진 역할만 수행하도록 구현. 이를 위해 실제 하드웨어에 접근하는 코드는 `*_manager.c`, IPC 메시지를 받고 이에 대응하는 코드는 `*_manager_proc.c`에 구현됩니다.

* **Process Logic (`*_manager_proc.c`):** IPC 메시지를 받고 `switch-case`를 통해 명령을 해석하는 역할.
* **Hardware Abstraction Layer (HAL) (`*_manager.c`):** 실제 드라이버 호출(예시에서는 `printf`로 대체하나 구현한다면 주석으로 구현될 예정)을 담당. 로직과 하드웨어를 분리하여 테스트와 가독성을 높이는 목적.

## 🚀 Build & Run

이 프로젝트는 파일이 점차 증가함에 따라 **Makefile**을 사용하여 빌드 및 관리됩니다.

### 1. 빌드 환경

이 프로젝트를 빌드하고 실행하려면 아래 패키지가 설치되어야합니다.

* **GNU Compiler Collection (GCC)**
* **GNU Make** (Makefile 실행을 위해 필수)

```
sudo apt update
sudo apt install build-essential
```

### 2. 프로젝트 빌드

프로젝트 루트 디렉토리(Makefile이 있는 곳)에서 `make` 명령을 실행하여 모든 실행 파일을 빌드합니다. `make`는 `ipc.key` 파일 생성도 함께 처리합니다.
```
make
```
### 3. 클린 (Clean)

빌드된 모든 실행 파일과 IPC 설정 파일을 삭제하여 빌드 전의 상태로 되돌립니다.
```
make clean
```
### 4. 실행

빌드가 완료되면, 메인 프로세스인 `main`(main_ipc.c)를 실행합니다. `main_ipc`는 **Supervisor** 역할을 하며 `fork()/exec()`를 통해 나머지 모든 매니저 프로세스를 자동으로 구동하고 감시합니다.

```
./main
```


### 5. IPC 설정 파일 (`ipc.key`)

**`ipc.key`** 파일은 메시지 큐에서 고유 키값을 얻기 위한 `ftok()`함수가 참조하는 파일입니다.

* **생성:** `Makefile`에 해당 생성 명령어 포함되어 있어, `make` 실행 시 자동으로 `touch ipc.key` 명령을 통해 생성됩니다.
* **삭제:** `make clean` 명령 실행 시 실행 파일들과 함꼐 삭제됩니다.