#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <string.h>
#include <errno.h> 
#include "common_ipc.h"
#include "display_manager.h" 
#include "log.h"

void manager_loop(int msgid) {
    IpcMessage rcv_msg;
    
    // 하드웨어 초기화 (Cairo Image Surface 환경 설정)
    initialize_display_hardware(); 

    while (1) {
        if (msgrcv(msgid, &rcv_msg, sizeof(IpcMessage) - sizeof(long), TYPE_DISPLAY_MANAGER, 0) == -1) {
            if (errno == EINTR) continue; 
            LOG_ERROR("msgrcv failed: %s. Exiting loop.", strerror(errno));
            break;
        }
        
        if (rcv_msg.command == CMD_SHUTDOWN) {
            LOG_INFO("Received CMD_SHUTDOWN. Shutting down.");
            break;
        }

        switch(rcv_msg.command) {
            case CMD_BOOT_SEQUENCE: 
                // 메인 매니저로부터 부팅 완료 신호를 받으면, "MENU" 화면을 출력합니다.
                LOG_INFO("Received CMD_BOOT_SEQUENCE. Drawing initial menu screen.");
                draw_menu_screen();
                break;
                
            case CMD_SET_MODE:
                LOG_INFO("Received CMD_SET_MODE. Redrawing screen based on mode: %s", rcv_msg.payload);
                draw_menu_screen(); 
                break;
                
            case CMD_REQUEST_PING: 
                send_ipc_message(msgid, TYPE_MAIN_MANAGER, CMD_SEND_PONG, "Send Pong");
                break;
                
            default:
                LOG_WARN("Received unknown command: %d", rcv_msg.command);
                break;
        }
    }
}

int main() {
    key_t key; 
    int msgid;
    
    // IPC 키 생성 및 메시지 큐 획득
    if ((key = ftok("./ipc.key", 'A')) == -1) exit(1);
    if ((msgid = msgget(key, 0666)) == -1) exit(1);
    
    // 로그 초기화는 manager_loop 내부 initialize_display_hardware()에서 수행됨
    
    manager_loop(msgid);
    
    // 프로세스 종료 시 리소스 해제
    cleanup_display_resources(); // Cairo 리소스 해제, 메모리 릭 방지
    log_close(); 
    return 0;
}