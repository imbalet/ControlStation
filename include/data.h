// #ifndef DATA
// #define DATA


// #include "main.h"


// typedef struct gamepad {
// 	int axes[4];
// 	bool buttons[20];
// } gamepad;

// gamepad pads[2];

// double coords[5];

// int size_data = 19;

// //1%019/$0/0/0/0/0/229

// void convertData(char *data) { //0a001001001001/0b0000000000000000000/a001001001001/0b000000000000000000/
//   bool now; // 1 - axes, 0 - buttons
// 	uint8_t pad;
//   if (data[0] == '$'){
//     int count = 0;
//     uint8_t ptr = 0, old_ptr = 0;

//     for (int i = 0; i <= size_data; i++){
//       if(data[i] == '\r') return;
//       if (data[i] == '/'){

//         if (count != 0){
//           char* temp = (char*) malloc(count + 1); 
//           strncpy(temp, &data[old_ptr + 1], count - 1);
//           coords[ptr] = atof(temp);
//           old_ptr = i;
//           ptr++;
//           free(temp);
//         }
//         count = 0;
//       }
//       count++;
//     }
//   }

// 	for (int i = 1; data[i] != '\r'; i++) {
// 		if (data[i] == '\r')
// 			break;
// 		if (data[i] == 'a' || data[i] == 'b') {
// 			now = data[i] == 'a' ? 1 : 0;
// 			pad = data[i - 1] - 48;
// 			continue;
// 		}
// 		for (int j = 0; data[i - 1] != '/';) {
// 			if (data[i] == '/' || data[i - 1] == '/')
// 				break;
// 			if (now) {
// 				pads[pad].axes[j] = (data[i] - 48) * 100 + (data[i + 1] - 48) * 10 + (data[i + 2] - 48);
// 				j++;
// 				i += 3;
// 			} else if (!now) {
// 				pads[pad].buttons[j] = data[i] - 48;
// 				j++;
// 				i++;
// 			}
// 		}
// 	}
// }

// #endif