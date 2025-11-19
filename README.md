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
