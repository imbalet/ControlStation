## Станция управления
---
### Компоненты
  - ESP32/ESP8266 (Нужно раскомментировать/закомментировать блок кода в platformio.ini для выбора)
  - [arduinoWebSockets](https://github.com/Links2004/arduinoWebSockets)
  - [ESPWebServer](https://github.com/esp8266/ESPWebServer)

*Библиотеки лежат в /lib*

---
### Структура
```
|--lib  
|  |  
|  |--ESPWebServer  
|  |--arduinoWebSockets  
|  
|--src  
|  |  
|  |- html.h (Веб-интерфейс станции управления)  
|  |- main.cpp  
|  |- stm32.txt (Код для STM32 для приема и конвертации данных)  
|  
|- platformio.ini  
```

---
### Конфигурация UART
```
Mode - Asynchronous
Enable global interrupt

Baud Rate - 15200 Bits/s
Word Length 8 Bits (including Parity)
Parity - None
Stop Bits - 1

DMA
Peripheral То Memory
Mode - circular
Data width - byte
```
---

### Код для STM32
``` C
#include <stdbool.h>
#include <string.h>
#define DEF_PAD_DATA "0a255255255255/0b0000000000000000000/a255255255255/0b000000000000000000/\r"
#define UART_TIMEOUT 300

typedef struct gamepad {
	int axes[4];
	bool buttons[20];
} gamepad;

gamepad pads[2];

//RX
uint8_t rx_data[1];
uint8_t temp_data[100];
char data[100];
uint8_t size_data = 0;
uint8_t ind_data = 0;
bool write_data = 0;
uint8_t check_sum;

//Timer
uint32_t timer = 0;

//-1 to 1
float mov_axes[4];

void convertData(char *data) { //0a001001001001/0b0000000000000000000/a001001001001/0b000000000000000000/
	bool now; // 1 - axes, 0 - buttons
	uint8_t pad;
	if (!(data[1] == 'a' || data[1] == 'b' || data[size_data - 1] == '\r')) {
		return;
	}
	for (int i = 1; data[i] != '\r'; i++) {
		if (data[i] == '\r')
			break;
		if (data[i] == 'a' || data[i] == 'b') {
			now = data[i] == 'a' ? 1 : 0;
			pad = data[i - 1] - 48;
			continue;
		}
		for (int j = 0; data[i - 1] != '/';) {
			if (data[i] == '/' || data[i - 1] == '/')
				break;
			if (now) {
				pads[pad].axes[j] = (data[i] - 48) * 100 + (data[i + 1] - 48) * 10 + (data[i + 2] - 48);
				j++;
				i += 3;
			} else if (!now) {
				pads[pad].buttons[j] = data[i] - 48;
				j++;
				i++;
			}
		}
	}
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {

	if (rx_data[0] == '%') {
		check_sum = 0;
		write_data = 1;
		ind_data = 0;
		size_data = 0;
	}

	if (write_data) {
		if (ind_data <= 76) {
			check_sum += rx_data[0];
		}

		temp_data[ind_data] = rx_data[0];
		ind_data++;
	}
	if (ind_data >= 99) {
		check_sum = 0;
		write_data = 0;
		ind_data = 0;
		size_data = 0;
	}
	if (ind_data == size_data) {
		HAL_GetTick();
	}
	if (rx_data[0] == '\r') {

		size_data = (temp_data[1] - 48) * 100 + (temp_data[2] - 48) * 10 + (temp_data[3] - 48);
		uint8_t check_sum_data = (temp_data[size_data - 3] - 48) * 100 + (temp_data[size_data - 2] - 48) * 10 + (temp_data[size_data - 1] - 48);
		write_data = 0;
		if (check_sum == check_sum_data && ind_data == size_data + 1) {
			timer = HAL_GetTick();
			memcpy(data, &temp_data[5], size_data - 10);
			data[size_data - 8] = '\r';
		}
	}
}

void convert_chushpan(void) {
	if (HAL_GetTick() - timer >= UART_TIMEOUT) {
		memcpy(data, DEF_PAD_DATA, 74);
	}

	HAL_UART_Receive_DMA(&huart1, rx_data, 1);

	convertData((char*) data);

	for (int i = 0; i < 4; i++) {
		mov_axes[i] = i % 2 == 0 ? (double)((pads[0].axes[i]) - 256.0f) / 256.0f : (double)((pads[0].axes[i]) - 256.0f) / -256.0f;
		if (mov_axes[i] >= -0.1 && mov_axes[i] <= 0.1) mov_axes[i] = 0.0f;
	}
}


int main(void){
  //....
  HAL_UART_Receive_DMA(&huart1, rx_data, 1);
  //....
  while(1){
    convert_chushpan();
  }
}

```
---

