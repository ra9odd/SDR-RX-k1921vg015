// Gregory Panin 
// 08.2026 SDR 1921VG015 first test only version
//-- Includes ------------------------------------------------------------------
#undef LSICLK_VAL
#include <K1921VG015.h>
#include <stdint.h>
#include <stdio.h>
#include <system_k1921vg015.h>
#include "plib015_adcsd.h"

// SDR -- Defines -------------------------------------------------------------------

#define ADCSD_CHANNEL_AMOUNT  3   //0-I, 1-Q, 2-Microphone
#define HALF_BUF_SIZE         256
#define FULL_BUF_SIZE         (HALF_BUF_SIZE * 2)

// Буферы для каждого канала
volatile int16_t g_adc_buffers[ADCSD_CHANNEL_AMOUNT][FULL_BUF_SIZE];// I Q
// --- БУФЕРЫ ВЫВОДА (ЦАП) ---
// Массив для хранения подготовленных 12-битных беззнаковых данных (0..4095)
volatile uint16_t g_dac_buffers[FULL_BUF_SIZE] __attribute__((aligned(4))); // Audio Out


// Текущий индекс записи (от 0 до 255)
volatile uint16_t g_buf_sample_idx = 0;

// Текущая активная половина для записи (0 - первая, 1 - вторая)
volatile uint8_t g_buf_active_half = 0;

// Флаги готовности половин для обработки в основном цикле
// bit 0: готова первая половина (0..255), bit 1: готова вторая половина (256..511)
volatile uint8_t g_buf_ready_flags = 0;
// Переменная-счетчик для прореживания PWM DAC (0..3)
volatile uint8_t g_decimation_cnt = 0;
volatile uint8_t ssb_mode = 1; //USB==1, LSB==0

// SSB 
#define FIR_TAP_NUM   41  // Порядок фильтра 40 (41 коэффициент)

// Коэффициенты ФНЧ (срез 3 кГц @ 62.5 кГц)
const float FIR_COEFFS[FIR_TAP_NUM] = {
    -0.001150f, -0.001550f, -0.001600f, -0.000840f,  0.001090f,  0.004120f,  0.007620f,  0.010530f,
     0.011490f,  0.009130f,  0.002440f, -0.008440f, -0.021960f, -0.035720f, -0.046770f, -0.051910f,
    -0.048380f, -0.034110f, -0.008310f,  0.028080f,  0.071740f,  0.117180f,  0.158020f,  0.188390f,
     0.203790f,  0.203790f,  0.188390f,  0.158020f,  0.117180f,  0.071740f,  0.028080f, -0.008310f,
    -0.034110f, -0.048380f, -0.051910f, -0.046770f, -0.035720f, -0.021960f, -0.008440f,  0.002440f,
     0.009130f
};

// Таблица синуса на 256 значений (амплитуда от -32767 до 32767)
// Индекс таблицы — это старшие 8 бит 32-битного nco_phase_acc
static const int16_t sin_lut[256] = 
{
    0, 804, 1608, 2410, 3212, 4011, 4808, 5602, 6393, 7179, 7962, 8739, 9512, 10278, 11039, 11793,
    12539, 13279, 14010, 14732, 15446, 16151, 16846, 17530, 18204, 18868, 19519, 20159, 20787, 21403, 22005, 22594,
    23170, 23731, 24279, 24811, 25329, 25832, 26319, 26790, 27245, 27683, 28105, 28510, 28898, 29269, 29621, 29956,
    30273, 30571, 30852, 31113, 31356, 31580, 31785, 31971, 32137, 32285, 32412, 32521, 32609, 32678, 32728, 32757,
    32767, 32757, 32728, 32678, 32609, 32521, 32412, 32285, 32137, 31971, 31785, 31580, 31356, 31113, 30852, 30571,
    30273, 29956, 29621, 29269, 28898, 28510, 28105, 27683, 27245, 26790, 26319, 25832, 25329, 24811, 24279, 23731,
    23170, 22594, 22005, 21403, 20787, 20159, 19519, 18868, 18204, 17530, 16846, 16151, 15446, 14732, 14010, 13279,
    12539, 11793, 11039, 10278, 9512, 8739, 7962, 7179, 6393, 5602, 4808, 4011, 3212, 2410, 1608, 804,
    0, -804, -1608, -2410, -3212, -4011, -4808, -5602, -6393, -7179, -7962, -8739, -9512, -10278, -11039, -11793,
    -12539, -13279, -14010, -14732, -15446, -16151, -16846, -17530, -18204, -18868, -19519, -20159, -20787, -21403, -22005, -22594,
    -23170, -23731, -24279, -24811, -25329, -25832, -26319, -26790, -27245, -27683, -28105, -28510, -28898, -29269, -29621, -29956,
    -30273, -30571, -30852, -31113, -31356, -31580, -31785, -31971, -32137, -32285, -32412, -32521, -32609, -32678, -32728, -32757,
    -32767, -32757, -32728, -32678, -32609, -32521, -32412, -32285, -32137, -31971, -31785, -31580, -31356, -31113, -30852, -30571,
    -30273, -29956, -29621, -29269, -28898, -28510, -28105, -27683, -27245, -26790, -26319, -25832, -25329, -24811, -24279, -23731,
    -23170, -22594, -22005, -21403, -20787, -20159, -19519, -18868, -18204, -17530, -16846, -16151, -15446, -14732, -14010, -13279,
    -12539, -11793, -11039, -10278, -9512, -8739, -7962, -7179, -6393, -5602, -4808, -4011, -3212, -2410, -1608, -804
};



// Таблица синусов и косинусов для гетеродина (Look-up Table)
// Размер равен FULL_BUF_SIZE * 2 для удобства непрерывного циклического чтения без ветвлений
//#define LUT_SIZE 512

//float sin_lut_f[256];


// Глобальная переменная шага фазы, которую читает смеситель
volatile uint32_t nco_phase_inc = 0; // Значение по умолчанию (8 кГц)
volatile uint32_t g_target_freq_hz = 8000;

// Исторические буферы («линии задержки») для КИХ-фильтров (по одному на канал I и Q)
volatile float g_fir_state_i[FIR_TAP_NUM] = {0};
volatile float g_fir_state_q[FIR_TAP_NUM] = {0};
volatile uint16_t g_fir_idx = 0;


static uint32_t nco_phase_acc = 0;          // 32-битный аккумулятор фазы

// --- ПАРАМЕТРЫ АРУ (Глобальные переменные) ---
// Текущий коэффициент усиления (в формате с фиксированной запятой Q8, то есть 256 = 1.0)
volatile int32_t g_agc_gain = 256; 
// Сглаженное значение текущей амплитуды выходного сигнала
volatile int32_t g_agc_envelope = 0; 

// END SSB

//-- Defines -------------------------------------------------------------------
#define GPIOA_ALL_Msk	0xFFFF
#define GPIOB_ALL_Msk	0xFFFF
#define UART3_BAUD 115200
#define SystemCoreClock_uart	16000000
#define MEASUREMENTS 32

/* ADCSD channel modes: */
#define ADCSD_MODE_CHx_DISABLED               0
#define ADCSD_MODE_CHx_RUN_AND_DISABLE        1
#define ADCSD_MODE_CHx_RUN_AND_ENABLED        2
#define ADCSD_MODE_CHx_FREE_RUN               3
/* ADSCD channel amplification: */
#define ADCSD_AMPL_CHx_0_dB                   0
#define ADCSD_AMPL_CHx_6_dB                   1
#define ADCSD_AMPL_CHx_12_dB                  2
#define ADCSD_AMPL_CHx_18_dB                  3
#define ADCSD_AMPL_CHx_20_dB                  4
#define ADCSD_AMPL_CHx_26_dB                  5
#define ADCSD_AMPL_CHx_32_dB                  6
#define ADCSD_AMPL_CHx_38_dB                  7

uint32_t g_led_mask_idx = 0;
const uint16_t g_led_masks[14] = { 0x0010u, 0x0020u, 0x0040u, 0x0080u, 0x1000u, 0x2000u, 0x4000u, 
                                   0x8000u, 0x4000u, 0x2000u, 0x1000u, 0x0080u, 0x0040u, 0x0020u};


#define LEDS_MSK	0xFF00
#define LED0_MSK	(1 << 8)
#define LED1_MSK	(1 << 9)
#define LED2_MSK	(1 << 10)
#define LED3_MSK	(1 << 11)
#define LED4_MSK	(1 << 12)
#define LED5_MSK	(1 << 13)
#define LED6_MSK	(1 << 14)
#define LED7_MSK	(1 << 15)

//Значение источника опорного напряжения в Вольтах
#define BG_SDADC_CAP	1.285f
#define TX_BUF_SIZE 64

int16_t adcd[ADCSD_CHANNEL_AMOUNT];
uint8_t tx_buf[TX_BUF_SIZE];
uint32_t tx_data_count = 0;
uint32_t tx_buf_cur_pos = 0;

// сюда записываются данные измерений каналов в прерывании
volatile int g_adc_val[8];

// флаги готовности результатов
volatile uint8_t g_read_val[8];


volatile uint32_t adc_irq_count = 0;

void ADCSD_IRQHandler();
void PLIC_Handler_ADC();
void TMR32_IRQHandler();

void ADCSD_init()
{
  // АЦП
  // выбор источника тактирования
  RCU->ADCSDCLKCFG = (RCU_ADCSDCLKCFG_CLKSEL_HSE << RCU_ADCSDCLKCFG_CLKSEL_Pos) // частота АЦП = PLL_CLOCK_FREQ1
                   | RCU_ADCSDCLKCFG_CLKEN_Msk
                   | RCU_ADCSDCLKCFG_RSTDIS_Msk
                   ;
  // включение ADCSD
  RCU->CGCFGAPB_bit.ADCSDEN = 1;
  RCU->RSTDISAPB_bit.ADCSDEN = 1;


  // включаем генератор источник напряжения и выставляем биты сброса
  ADCSD->CTRL = (7 << ADCSD_CTRL_WTCYC_Pos)
              | (0 << ADCSD_CTRL_MDC_Pos)   // MDC = 0 (Делитель частоты модуляции = 2)
              | (3 << ADCSD_CTRL_DR_Pos)    // DR = 3  (Коэффициент децимации OSR = 256), DR = 4  (Коэффициент децимации OSR = 128)
              | ADCSD_CTRL_SFRST_Msk
              | ADCSD_CTRL_RST_Msk
              | ADCSD_CTRL_PUREF_Msk
              ;
  // биты сброса убираем
  ADCSD->CTRL = (7 << ADCSD_CTRL_WTCYC_Pos)
              | (0 << ADCSD_CTRL_MDC_Pos)   // MDC = 0
              | (3 << ADCSD_CTRL_DR_Pos)    // DR = 3  (OSR = 256), DR = 4  (Коэффициент децимации OSR = 128)
              | ADCSD_CTRL_PUREF_Msk
              | ADCSD_CTRL_ENB_Msk
              ;


  // все каналы отключены
  ADCSD->ENB = 0;
  // режим для всех каналов - циклический запуск преобразования
  ADCSD->MODE = (ADCSD_MODE_CHx_FREE_RUN << ADCSD_MODE_CH7_Pos)
              | (ADCSD_MODE_CHx_FREE_RUN << ADCSD_MODE_CH6_Pos)
              | (ADCSD_MODE_CHx_FREE_RUN << ADCSD_MODE_CH5_Pos)
              | (ADCSD_MODE_CHx_FREE_RUN << ADCSD_MODE_CH4_Pos)
              | (ADCSD_MODE_CHx_FREE_RUN << ADCSD_MODE_CH3_Pos)
              | (ADCSD_MODE_CHx_FREE_RUN << ADCSD_MODE_CH2_Pos)
              | (ADCSD_MODE_CHx_FREE_RUN << ADCSD_MODE_CH1_Pos)
              | (ADCSD_MODE_CHx_FREE_RUN << ADCSD_MODE_CH0_Pos)
              ;
  // усиление по всем каналам = х1
  ADCSD->AMPL = (ADCSD_AMPL_CHx_0_dB << ADCSD_AMPL_CH0_Pos)
              | (ADCSD_AMPL_CHx_0_dB << ADCSD_AMPL_CH1_Pos)
              | (ADCSD_AMPL_CHx_0_dB << ADCSD_AMPL_CH2_Pos)
              | (ADCSD_AMPL_CHx_0_dB << ADCSD_AMPL_CH3_Pos)
              | (ADCSD_AMPL_CHx_0_dB << ADCSD_AMPL_CH4_Pos)
              | (ADCSD_AMPL_CHx_0_dB << ADCSD_AMPL_CH5_Pos)
              | (ADCSD_AMPL_CHx_0_dB << ADCSD_AMPL_CH6_Pos)
              | (ADCSD_AMPL_CHx_0_dB << ADCSD_AMPL_CH7_Pos)
              ;
  // разрешаем работу ADCSD
  ADCSD->CTRL_bit.ENB = 1;
  // выбираем все каналы для работы
  ADCSD->ENB = ADCSD_ENB_CH0_Msk
             | ADCSD_ENB_CH1_Msk
             | ADCSD_ENB_CH2_Msk
             | ADCSD_ENB_CH3_Msk
             | ADCSD_ENB_CH4_Msk
             | ADCSD_ENB_CH5_Msk
             | ADCSD_ENB_CH6_Msk
             | ADCSD_ENB_CH7_Msk
             ;
  // прерывание ото всех каналов
  ADCSD->IM = ADCSD_IM_CH0_Msk
            | ADCSD_IM_CH1_Msk
            | ADCSD_IM_CH2_Msk
            | ADCSD_IM_CH3_Msk
            | ADCSD_IM_CH4_Msk
            | ADCSD_IM_CH5_Msk
            | ADCSD_IM_CH6_Msk
            | ADCSD_IM_CH7_Msk
            ;
  // разрешим одработку прерывания от АЦП (линия в PLIC к сожалению общая для ADCSAR и ADCSD)

    // Настраиваем обработчик прерывания для ADC
    PLIC_SetIrqHandler (Plic_Mach_Target, IsrVect_IRQ_ADC, PLIC_Handler_ADC);
    PLIC_SetPriority   (IsrVect_IRQ_ADC, 0x1);
    PLIC_IntEnable     (Plic_Mach_Target, IsrVect_IRQ_ADC);
 


}

void TX_PWM_init(uint16_t period)
{
  RCU->CGCFGAPB_bit.TMR2EN = 1;
  RCU->RSTDISAPB_bit.TMR2EN = 1;

  // Выбираем льтернативную функцию №2 для GPIOA.2 и GPIOA.3
  GPIOA->ALTFUNCNUM_bit.PIN2 = 2;
  GPIOA->ALTFUNCNUM_bit.PIN3 = 2;
  GPIOA->ALTFUNCSET = (1 << 2) | (1 << 3);

  // Записываем значение периода в CAPCOM[0]
  // Таймер будет считать от 0 до 1023, формируя базу для 10 бит
  TMR2->CAPCOM[0].VAL = period - 1;

  // Настраиваем режим сравнения для CAPCOM[2] - управление выводом GPIOA.8
  TMR2->CAPCOM[2].CTRL_bit.CAP = 0; 
  TMR2->CAPCOM[2].CTRL_bit.OUTMODE = 7; 
  
  // Настраиваем режим сравнения для CAPCOM[3] - управление выводом GPIOA.9
  TMR2->CAPCOM[3].CTRL_bit.CAP = 0; 
  TMR2->CAPCOM[3].CTRL_bit.OUTMODE = 3; 

  // Убираем делитель частоты! 
  // Значение 0 означает Div1 (нет предделителя, таймер тактируется на полных 50 МГц)
  TMR2->CTRL_bit.DIV = 0; 

  // Режим счета от 0 до значения CAPCOM[0]
  TMR2->CTRL_bit.MODE = TMR_CTRL_MODE_Up;

  // Задаем начальное заполнение ШИМ (50% = виртуальный ноль для звука SDR)
  // Значение 512 идеально центрирует ШИМ в диапазоне 0..1023
  TMR2->CAPCOM[2].VAL = 512;
  TMR2->CAPCOM[3].VAL = 512;


}

void PWM_init(uint16_t period) 
{

  RCU->CGCFGAPB_bit.TMR1EN = 1;
  RCU->RSTDISAPB_bit.TMR1EN = 1;

  //Настраиваем альтернативную функцию для GPIOA.8 и GPIOA.9
  RCU->CGCFGAHB_bit.GPIOAEN = 1;
  //Включаем  GPIOA
  RCU->RSTDISAHB_bit.GPIOAEN = 1;

  // Выбираем льтернативную функцию №2 для GPIOA.8 и GPIOA.9
  GPIOA->ALTFUNCNUM_bit.PIN8 = 2;
  GPIOA->ALTFUNCNUM_bit.PIN9 = 2;
  GPIOA->ALTFUNCSET = (1 << 8) | (1 << 9);

  // Записываем значение периода в CAPCOM[0]
  // Таймер будет считать от 0 до 1023, формируя базу для 10 бит
  TMR1->CAPCOM[0].VAL = period - 1;

  // Настраиваем режим сравнения для CAPCOM[2] - управление выводом GPIOA.8
  TMR1->CAPCOM[2].CTRL_bit.CAP = 0; 
  TMR1->CAPCOM[2].CTRL_bit.OUTMODE = 7; 
  
  // Настраиваем режим сравнения для CAPCOM[3] - управление выводом GPIOA.9
  TMR1->CAPCOM[3].CTRL_bit.CAP = 0; 
  TMR1->CAPCOM[3].CTRL_bit.OUTMODE = 3; 

  // Убираем делитель частоты! 
  // Значение 0 означает Div1 (нет предделителя, таймер тактируется на полных 50 МГц)
  TMR1->CTRL_bit.DIV = 0; 

  // Режим счета от 0 до значения CAPCOM[0]
  TMR1->CTRL_bit.MODE = TMR_CTRL_MODE_Up;

  // Задаем начальное заполнение ШИМ (50% = виртуальный ноль для звука SDR)
  // Значение 512 идеально центрирует ШИМ в диапазоне 0..1023
  TMR1->CAPCOM[2].VAL = 512;
  TMR1->CAPCOM[3].VAL = 512;


  //Разрешаем прерывание по совпадению значения счетчика и CAPCOM[0]
//  TMR1->IM = 2;

  // Настраиваем обработчик прерывания для TMR1
 // PLIC_SetIrqHandler (Plic_Mach_Target, IsrVect_IRQ_TMR32, TMR32_IRQHandler);
 // PLIC_SetPriority   (IsrVect_IRQ_TMR32, 0x1);
 // PLIC_IntEnable     (Plic_Mach_Target, IsrVect_IRQ_TMR32);
}

void BSP_led_init()
{
	//Разрешаем тактирование GPIOC
	RCU->CGCFGAHB_bit.GPIOCEN = 1;
	//Включаем  GPIOC
	RCU->RSTDISAHB_bit.GPIOCEN = 1;
	GPIOC->OUTENSET = 1;
	GPIOC->DATAOUTSET = 1;
}

void TMR32_init(uint32_t period)
{
  RCU->CGCFGAPB_bit.TMR32EN = 1;
  RCU->RSTDISAPB_bit.TMR32EN = 1;

  //Записываем значение периода в CAPCOM[0]
  TMR32->CAPCOM[0].VAL = period-1;
  //Выбираем режим счета от 0 до значения CAPCOM[0]
  TMR32->CTRL_bit.MODE = 1;

  //Разрешаем прерывание по совпадению значения счетчика и CAPCOM[0]
  TMR32->IM = 2;

  // Настраиваем обработчик прерывания для TMR32
  PLIC_SetIrqHandler (Plic_Mach_Target, IsrVect_IRQ_TMR32, TMR32_IRQHandler);
  PLIC_SetPriority   (IsrVect_IRQ_TMR32, 0x1);
  PLIC_IntEnable     (Plic_Mach_Target, IsrVect_IRQ_TMR32);
}



//-- Peripheral init functions -------------------------------------------------
void periph_init()
{
	SystemInit();
	SystemCoreClockUpdate();
        PWM_init(1024);
        TX_PWM_init(1024);
      	ADCSD_init();
	BSP_led_init();
}

void delay (uint32_t val) {
	for(;val > 0; val--)
		asm("NOP");
}


//-- IRQ INTERRUPT HANDLERS ---------------------------------------------------------------
void TMR32_IRQHandler()
{
//	GPIOC->DATAOUTTGL = 1;
        //  printf("\nMeasure results:\n");
        //  printf("CH0\tCH1\tCH2\tCH3\tCH4\tCH5\tCH6\tCH7\n");
    //Сбрасываем флаг прерывания таймера
          
	TMR32->IC = 3;
}

void ADCSD_IRQHandler()
{
	uint32_t irq_channel = 0;
	for(uint32_t channel = 0; channel < ADCSD_CHANNEL_AMOUNT; channel++) 
        {
	  if ((uint32_t)ADCSD_ITMaskedStatus(channel)) 
          {
			irq_channel = channel;
			break;
	    }
	}
	ADCSD_ITStatusClear((ADCSD_CH_Num_TypeDef)irq_channel);
    adc_irq_count++;
}

void PLIC_Handler_ADC(void) 
{

    uint16_t pwm_base_idx = g_buf_active_half ? HALF_BUF_SIZE : 0;// рабочая половина буфера приемника

    // === ОБНОВЛЕНИЕ ШИМ С ДЕЦИМАЦИЕЙ В 4 РАЗА ===
  
    if (g_decimation_cnt == 0) // вывод звука в ШИМ порт А пин 8, синхронно с АЦП 
    {
        //TMR0->CAPCOM[1].VAL = g_dac_buffers[pwm_base_idx + g_buf_sample_idx];
        TMR1->CAPCOM[2].VAL = g_dac_buffers[pwm_base_idx + g_buf_sample_idx];
       
    }
    g_decimation_cnt = (g_decimation_cnt + 1) & 0x03;

    // === СБОР ДАННЫХ АЦП ===
    uint32_t v_dataupd = ADCSD->DATAUPD;
    uint32_t v_channel_mask = 1u;
    uint16_t adc_base_idx = g_buf_active_half ? HALF_BUF_SIZE : 0;
    
    for (int i = 0; i < ADCSD_CHANNEL_AMOUNT; ++i) 
    {
        if (v_dataupd & v_channel_mask) 
        {
            g_adc_buffers[i][adc_base_idx + g_buf_sample_idx] = (int16_t)(ADCSD->DATA[i].DATA & 0xFFFF);
        }
        v_channel_mask <<= 1;
    }

    g_buf_sample_idx++;
    if (g_buf_sample_idx >= HALF_BUF_SIZE) 
    {
        g_buf_sample_idx = 0;
        g_buf_ready_flags |= (1u << g_buf_active_half);
        g_buf_active_half ^= 1;
    }

    ADCSD->DATAUPD = v_dataupd;
    ADCSD->IC = v_dataupd;
}


void SDR_Set_Carrier_Freq(int32_t target_freq_hz)
{
    if (target_freq_hz > 25000)  target_freq_hz = 25000;
    if (target_freq_hz < -25000) target_freq_hz = -25000;

    // Сохраняем исходную частоту со знаком для NCO
    g_target_freq_hz = target_freq_hz; 

    uint32_t abs_freq = (target_freq_hz < 0) ? -target_freq_hz : target_freq_hz;
    uint64_t calculated_inc = ((uint64_t)abs_freq << 32) / 62500u;

    PLIC_IntDisable(Plic_Mach_Target, IsrVect_IRQ_ADC);
    nco_phase_inc = (uint32_t)calculated_inc;
    PLIC_IntEnable(Plic_Mach_Target, IsrVect_IRQ_ADC);

}
static inline int16_t Process_SDR_Sample(int16_t sample_i, int16_t sample_q) 
{
    // 0. NCO ПЕРЕНОС ЧАСТОТЫ НА ПЧ 8 кГц
    uint32_t nco_phase_cos = nco_phase_acc + 1073741824; 
    uint8_t lut_idx_sin = (uint8_t)(nco_phase_acc >> 24);
    uint8_t lut_idx_cos = (uint8_t)(nco_phase_cos >> 24); 

    int16_t sin_val = sin_lut[lut_idx_sin];
    int16_t cos_val = sin_lut[lut_idx_cos];

    // ЗАЩИТА 1: Комплексное умножение делаем в int32_t, чтобы избежать промежуточных переполнений
    int32_t mixed_i = ((int32_t)sample_i * cos_val - (int32_t)sample_q * sin_val) >> 15;
    int32_t mixed_q = ((int32_t)sample_i * sin_val + (int32_t)sample_q * cos_val) >> 15;

  //  nco_phase_acc += nco_phase_inc;

    if (g_target_freq_hz >= 0) {
        nco_phase_acc += nco_phase_inc; // Положительный сдвиг
    } else {
        nco_phase_acc -= nco_phase_inc; // Отрицательный сдвиг (инверсия вращения)
    }

    // 1. КОЛЬЦЕВОЙ БУФЕР И ДИФФЕРЕНЦИАЛЬНАЯ ФИЛЬТРАЦИЯ
    static int16_t delay_buf_i[16] = {0}; 
    static int16_t delay_buf_q[16] = {0};
    static uint8_t u_idx = 0;

    // Ограничиваем значения перед записью в буфер задержки, гарантируя диапазон int16_t
    if (mixed_i > 16383)  mixed_i = 16383;
    if (mixed_i < -16384) mixed_i = -16384;
    if (mixed_q > 16383)  mixed_q = 16383;
    if (mixed_q < -16384) mixed_q = -16384;

    delay_buf_i[u_idx] = (int16_t)mixed_i;
    delay_buf_q[u_idx] = (int16_t)mixed_q;

    // Задержка 2 сэмпла (сдвиг 90 градусов)
    uint8_t idx_delayed = (u_idx - 2) & 15;
    int32_t i_90 = delay_buf_i[idx_delayed];
    int32_t q_90 = delay_buf_q[idx_delayed];

    // ЗАЩИТА 2: Вычитание в Comb-фильтре делаем строго в 32-битных переменных.
    // Это на 100% предотвратит "переломы" синусоиды при сложении!
    uint8_t idx_comb = (u_idx - 4) & 15;
    int32_t comb_i = (int32_t)delay_buf_i[u_idx] - delay_buf_i[idx_comb];
    int32_t comb_q = (int32_t)delay_buf_q[u_idx] - delay_buf_q[idx_comb];
    
    uint8_t idx_comb_90 = (idx_delayed - 4) & 15;
    int32_t comb_i_90 = i_90 - delay_buf_i[idx_comb_90];
    int32_t comb_q_90 = q_90 - delay_buf_q[idx_comb_90];

    u_idx = (u_idx + 1) & 15; 

    // 2. ФАЗОВАЯ ДЕМОДУЛЯЦИЯ (Выделение одной боковой)
    int32_t audio_out = 0;
    if (ssb_mode) {
        audio_out = comb_i - comb_q_90; // Фазовое подавление зеркалки
    } else {
        audio_out = comb_i + comb_q_90; 
    }

    // 3. ФИЛЬТР НЧ ЗВУКА (3 кГц)
    static int32_t lowpass_filter = 0;
    lowpass_filter = lowpass_filter - (lowpass_filter >> 3) + (audio_out >> 3);
    audio_out = lowpass_filter;

    // =========================================================================
    // БЛОК АРУ
    // =========================================================================
    // модуль текущего отсчета звука
    int32_t abs_audio = (audio_out < 0) ? -audio_out : audio_out;

    // Детектор огибающей с быстрым зарядом и медленным разрядом
    if (abs_audio > g_agc_envelope) {
        // Сигнал растет: быстрый подъем огибающей
        g_agc_envelope = g_agc_envelope + ((abs_audio - g_agc_envelope) >> 2); 
    } else {
        // Сигнал падает: очень медленный спад огибающей (разряд)
        g_agc_envelope = g_agc_envelope - (g_agc_envelope >> 9); 
    }

    // На основе огибающей пересчитываем целевой коэффициент усиления
    // Желаемая базовая амплитуда на выходе фильтра — около 4000 единиц
    if (g_agc_envelope > 10) 
    {
        int32_t target_gain = (4000 << 8) / g_agc_envelope; // Формат Q8
        
        // Ограничиваем максимальное и минимальное усиление АРУ
        if (target_gain > 8192) target_gain = 8192; // Макс усиление (x32)
        if (target_gain < 32)   target_gain = 32;   // Мин усиление (x0.125)

        // Плавная подстройка текущего Gain к целевому (сглаживание АРУ)
        g_agc_gain = g_agc_gain + ((target_gain - g_agc_gain) >> 4);
    } 
    else {
        // Если сигнала совсем нет, плавно выводим усиление на максимум
        g_agc_gain = g_agc_gain + ((4096 - g_agc_gain) >> 6);
    }

    // Применяем коэффициент АРУ к сигналу и возвращаем масштаб обратно (>> 8)
    audio_out = (audio_out * g_agc_gain) >> 8;
    // =========================================================================

    // 4. МАСШТАБИРОВАНИЕ ПОД 10-БИТНЫЙ ШИМ (-512...511)
    // Компрессируем итоговый сигнал так, чтобы он укладывался в рамки ШИМ
    audio_out = audio_out >> 4; 

    // финальное ограничение для безопасной записи в регистр
    if (audio_out > 511)  audio_out = 511;
    if (audio_out < -512) audio_out = -512;

    return (int16_t)audio_out;
}

void process_adc_data(uint8_t half_to_process)
{
    // Вычисляем смещение в буферах для текущей итерации (0 или 256)
    uint16_t offset = half_to_process ? HALF_BUF_SIZE : 0;
    
    // Указатели на входные данные каналов АЦП
    volatile int16_t *src_I   = &g_adc_buffers[0][offset];
    volatile int16_t *src_Q   = &g_adc_buffers[1][offset];
    volatile int16_t *src_Mic = &g_adc_buffers[2][offset];
    
    // Указатель на выходной буфер PWM-ЦАП (куда складывать результат обработки)
    volatile uint16_t *dst_Pwm = &g_dac_buffers[offset];

    // Цикл обработки пачки из 256 элементов

	// Внутри цикла обработки в main():
	for (int i = 0; i < HALF_BUF_SIZE; i++) 
	{
       // Читаем 16-битный знаковый сэмпл микрофона
        int16_t sample_i = src_I[i];  //+
        int16_t sample_q = src_Q[i];  //+

        dst_Pwm[i] =  Process_SDR_Sample(sample_i,sample_q)+512;
	}

}



int main(void)
{
  char str[64];
  int i=0;
  ssb_mode = 1;
  periph_init();

  SDR_Set_Carrier_Freq(0);
  InterruptEnable();
  GPIOC->DATAOUT =1;

 for (i = 0; i < FULL_BUF_SIZE; i++) { g_dac_buffers[i] = 511; }

  memset((uint8_t*)adcd,0,sizeof(adcd));


  while(1)
  {
        GPIOC->DATAOUT = 0; 

       // Проверяем, готова ли первая половина (Ping)
        if (g_buf_ready_flags & 0x01) 
        {
            GPIOC->DATAOUT = 1; 
	    PLIC_IntDisable(Plic_Mach_Target, IsrVect_IRQ_ADC);
            g_buf_ready_flags &= ~0x01;
	    PLIC_IntEnable(Plic_Mach_Target, IsrVect_IRQ_ADC);
            
            process_adc_data(0); 
        }
        
        // Проверяем, готова ли вторая половина (Pong)
        if (g_buf_ready_flags & 0x02) 
        {
            GPIOC->DATAOUT = 1; 
            //__disable_irq();
	    PLIC_IntDisable(Plic_Mach_Target, IsrVect_IRQ_ADC);
            g_buf_ready_flags &= ~0x02;
	    PLIC_IntEnable(Plic_Mach_Target, IsrVect_IRQ_ADC);
            
            process_adc_data(1); 
        }

  }

  return 0;
}

