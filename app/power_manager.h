#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

// 시스템 전원을 즉시 차단하는 함수
void trigger_hardware_power_off_signal();
// 전원 관리 IC 초기화 함수
void initialize_power_manager_hardware();

#endif // POWER_MANAGER_H