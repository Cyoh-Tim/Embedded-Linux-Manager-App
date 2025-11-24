#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

// 디스플레이 하드웨어 초기화 (여기서는 Cairo 환경 설정)
void initialize_display_hardware();

// Cairo를 사용하여 화면에 내용을 그리는 함수
void draw_menu_screen(void);

// 리소스 해제 함수
void cleanup_display_resources(void);

#endif // DISPLAY_MANAGER_H