#include "./BSP/LED/fan.h"

volatile FAN_STATE_E g_fan_state = FAN_OFF;
/**
 * @brief  PA12 初始化为推挽输出
 */
void PA12_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitStruct.Pin   = GPIO_PIN_12;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;   // 推挽输出
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // 默认关闭（低电平）
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);
}

/* ===== 控制接口 ===== */
void PA12_ON(void)
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_SET);
}

void PA12_OFF(void)
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);
}

void PA12_Toggle(void)
{
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_12);
}
