#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include "common_ipc.h"

// 실행 파일 이름들을 저장하는 배열 (common_ipc.h의 NUM_MANAGERS 크기)
const char* manager_exec_names[NUM_MANAGERS] = {
    LED_MANAGER_EXEC,
    MOTOR_MANAGER_EXEC,
    STATE_MANAGER_EXEC,
    POWER_MANAGER_EXEC 
};

/**
 * @brief 인덱스를 사용하여 특정 매니저 프로세스를 생성하고 실행합니다.
 * @param manager_index 실행할 매니저의 배열 인덱스
 */
void start_manager(int manager_index) {
    if (fork() == 0) {
        // 자식 프로세스 영역
        const char* exec_name = manager_exec_names[manager_index];
        printf("[Main Proc] Starting manager: %s\n", exec_name);
        
        // execl 호출: 현재 프로세스를 새 실행 파일로 대체
        execl(exec_name, exec_name, (char *)NULL);
        
        // execl이 성공하면 이 아래 코드는 실행되지 않습니다. 실패했을 경우에만 실행
        perror("execl failed"); 
        exit(1);
    }
}

/**
 * @brief OS 종료 시그널(Ctrl+C 등)을 처리하는 핸들러
 */
void shutdown_handler(int signum) {
    printf("\n[Main Proc] Received OS signal %d. Terminating application.\n", signum);
    
    // 실제 제품에서는 여기서 STATE 매니저에게 CMD_SHUTDOWN 메시지를 보내야 함
    // (예제 단순화를 위해 exit(0) 호출)
    exit(0); 
}


int main() {
    // OS 시그널(SIGINT: Ctrl+C, SIGTERM: 시스템 종료)이 오면 shutdown_handler 함수 실행
    signal(SIGINT, shutdown_handler);
    signal(SIGTERM, shutdown_handler);

    key_t key;
    int msgid;
    
    // 1. ipc.key 파일을 기준으로 키 생성
    if ((key = ftok("./ipc.key", 'A')) == -1) {
        perror("ftok failed"); exit(1);
    }

    // 2. 메시지 큐 생성
    if ((msgid = msgget(key, 0666 | IPC_CREAT)) == -1) {
        perror("msgget failed"); exit(1);
    }
    printf("[Main Proc] Message Queue ID: %d created.\n", msgid);

    // 3. 모든 매니저 프로세스 시작 (for 루프 사용)
    for(int i = 0; i < NUM_MANAGERS; i++)
    {
        start_manager(i);
    }
    
    sleep(1); // 자식 프로세스들이 완전히 시작될 시간을 줌

    // 4. 부팅 완료 신호 전송 (STATE 매니저에게 총괄 시작을 알림)
    // common_ipc.c의 헬퍼 함수 사용
    send_ipc_message(msgid, TYPE_STATE_MANAGER, 0, "boot_done");
    
    printf("[Main Proc] Sent 'boot_done'. System operational.\n");
    printf("[Main Proc] System running. Press Ctrl+C to stop.\n");

    // 5. 전원 ON 동안 메인 프로세스는 계속 대기 (return 하지 않음, pause 사용)
    while(1) {
        pause(); // OS 시그널이 올 때까지 CPU를 사용하지 않고 효율적으로 대기
    }
    
    // 이 코드는 pause()에 의해 실행되지 않습니다.
    return 0; 
}