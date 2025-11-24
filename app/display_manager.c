#include "display_manager.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 실제 Cairo 라이브러리 헤더
#include <cairo/cairo.h>

// --- 전역 변수: Cairo 리소스 관리 ---
static cairo_surface_t *g_surface = NULL;
static cairo_t *g_cr = NULL;
static unsigned char *g_data = NULL;

// 해상도
#define WIDTH 100
#define HEIGHT 50
#define STRIDE (cairo_format_stride_for_width(CAIRO_FORMAT_RGB24, WIDTH))

// 시뮬레이션 매크로, 선언 시 시뮬레이션이 실행됩니다.
//#define SIM_DISPLAY

void initialize_display_hardware() {
    log_init("DISPLAY", NULL);
    LOG_INFO("--- [1/2] Initializing Cairo Image Surface ---");
    
    // 1. 메모리에 픽셀 데이터 버퍼 할당 (RGB24 포맷)
    g_data = (unsigned char *)malloc(STRIDE * HEIGHT);
    if (!g_data) {
        LOG_FATAL("Failed to allocate memory for image buffer.");
        return;
    }
    
    // 2. 버퍼를 기반으로 Cairo Image Surface 생성
    g_surface = cairo_image_surface_create_for_data(
        g_data, 
        CAIRO_FORMAT_RGB24, 
        WIDTH, 
        HEIGHT, 
        STRIDE
    );
    
    if (cairo_surface_status(g_surface) != CAIRO_STATUS_SUCCESS) {
        LOG_FATAL("Failed to create Cairo surface: %s", 
                  cairo_status_to_string(cairo_surface_status(g_surface)));
        free(g_data);
        g_data = NULL;
        return;
    }

    // 3. Cairo Context 생성
    g_cr = cairo_create(g_surface);

    LOG_INFO("--- [2/2] Cairo Surface (%dx%d) and Context created. ---", WIDTH, HEIGHT);
}

void cleanup_display_resources(void) {
    if (g_cr) {
        cairo_destroy(g_cr);
        g_cr = NULL;
        LOG_DEBUG("Cairo Context destroyed.");
    }
    if (g_surface) {
        cairo_surface_destroy(g_surface);
        g_surface = NULL;
        LOG_DEBUG("Cairo Surface destroyed.");
    }
    if (g_data) {
        free(g_data);
        g_data = NULL;
        LOG_DEBUG("Image Buffer freed.");
    }
}


/**
 * @brief Cairo를 사용하여 버퍼에 "MENU" 텍스트를 그립니다.
 */
void draw_menu_screen(void) {
    if (!g_cr) {
        LOG_ERROR("Cairo context not initialized. Cannot draw.");
        return;
    }
    
    LOG_INFO("--- [1/4] Drawing 'MENU' to the buffer ---");

    // 1. 배경 흰색
    cairo_set_source_rgb(g_cr, 1.0, 1.0, 1.0); 
    cairo_paint(g_cr);
    LOG_DEBUG("-> Background set to White.");

    // 2. 텍스트 검은색
    cairo_set_source_rgb(g_cr, 0.0, 0.0, 0.0); // 검은색
    cairo_select_font_face(g_cr, "sans-serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);    // 폰트 설정, 폰트 파일이면 파일로 지정하면됨.
    cairo_set_font_size(g_cr, 20.0);    // 폰트 크기
    
    // 3. "MENU" 텍스트 위치 설정 및 그리기
    const char *text = "MENU";
    cairo_text_extents_t extents;
    cairo_text_extents(g_cr, text, &extents);

    // 중앙 정렬: (WIDTH - 텍스트 폭) / 2
    double x = (WIDTH - extents.width) / 2.0;
    // 중앙 정렬: (HEIGHT + 폰트 높이) / 2 
    double y = (HEIGHT + extents.height) / 2.0;
    
    cairo_move_to(g_cr, x, y);
    cairo_show_text(g_cr, text);
    LOG_INFO("-> Drew text '%s' centered in the buffer.", text);

    // 4. 드로잉 결과 반영, 이 함수를 안쓰면 가끔 깨지는 경우가 있었음.
    // 복잡한 DISPLAY 출력 시 프레임이 몇십배까지 떨어지는 경우가 있으니 더블 버퍼링 기법을 사용하는 것을 추천합니다.
    cairo_surface_flush(g_surface);
    LOG_INFO("--- [2/4] Drawing flushed to buffer. ---");

#ifdef SIM_DISPLAY
    // 5. 시뮬레이션: 텍스트가 포함된 중앙 영역의 버퍼 내용을 출력
    // 텍스트가 Y=35 근처에 그려지므로, Row 20부터 20개 행을 출력하여 텍스트 포함 여부를 확인합니다.
    const int START_ROW = 20;
    const int NUM_ROWS_TO_PRINT = 20;
    const int BYTES_TO_PRINT = STRIDE * NUM_ROWS_TO_PRINT;
    
    // 출력 시작 주소 계산: g_data + (STRIDE * START_ROW)
    unsigned char *print_start = g_data + (STRIDE * START_ROW);

    LOG_INFO("--- [3/4] Simulating LCD Output (Rows %d to %d, Total %d bytes) ---", 
             START_ROW, START_ROW + NUM_ROWS_TO_PRINT - 1, BYTES_TO_PRINT);

    // 각 행(Row)을 16진수 문자열로 변환하고 출력합니다.
    for (int row = 0; row < NUM_ROWS_TO_PRINT; ++row) {
        char hex_buffer[STRIDE * 2 + 1]; 
        unsigned char *row_start = print_start + (row * STRIDE);

        // 한 행(STRIDE 바이트)을 16진수 문자열로 변환
        for (int i = 0; i < STRIDE; ++i) {
            sprintf(&hex_buffer[i * 2], "%02X", row_start[i]);
        }
        hex_buffer[STRIDE * 2] = '\0'; // 문자열 종료

        // 출력 (가독성을 위해 64바이트/16픽셀마다 줄바꿈)
        LOG_INFO("-> Row %d:", START_ROW + row);
        // 400바이트 (800개의 16진수 문자)를 128개씩 나누어 출력
        for (int i = 0; i < STRIDE * 2; i += 128) {
            char line_buffer[129] = {0}; 
            strncpy(line_buffer, &hex_buffer[i], 128);
            LOG_INFO("    %s", line_buffer);
        }
    }
#endif
    LOG_INFO("--- [4/4] Screen update completed (Simulated LCD output). ---");
}