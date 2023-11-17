//
//
//#include "CustomLibs/Servo.h"
//#include "stm32f4xx.h"
//
//
////----------------------------------------------------------
////                     class Servos
////----------------------------------------------------------
//
//
//Servos::Servos(TIM_HandleTypeDef *tim1, TIM_HandleTypeDef *tim2,
//		GPIO_PIN* pines) {
//
//	this->tim1 = tim1;
//	this->tim2 = tim2;
//	_start = 0;
//	upd_changes = true;
//	ind = 0;
//	sig_gen = false;
//	gpio_port[N_SERVOS] = {};
//	gpio_pin[N_SERVOS] = {};
//	period[N_SERVOS] = {};
//	index[N_SERVOS] = {};
//	servo_index[N_SERVOS] = {};
//	tim_period[N_SERVOS] = {};
//
//	for (int i = 0; i < N_SERVOS; i++) {
//		gpio_port[i] = pines[i].getPort();
//	}
//	for (int i = 0; i < N_SERVOS; i++) {
//		gpio_pin[i] = pines[i].getNum();
//	}
//	for (int i = 0; i < N_SERVOS; i++) {
//		period[N_SERVOS] = SERVO_MIN_PREIOD;
//	}
//
//	uint8_t aux[N_SERVOS] = {};
//	this->setAngle(aux);
//}
//
//void Servos::start() {
//	_start = 1;
//	HAL_TIM_Base_Start_IT(tim1);
//}
//
//void Servos::stop() {
//	_start = 0;
//}
//
//
//void Servos::attach(uint8_t index){
//	if(index < N_SERVOS){
//		attached_servos[index] = true;
//	}
//}
//
//void Servos::detach(uint8_t index){
//	if(index < N_SERVOS){
//		attached_servos[index] = false;
//	}
//}
//
//
//
//void Servos::setAngle(uint8_t *angles) {
//	for (int i = 0; i < N_SERVOS; i++) {
//		angles[i] = angles[i] <= 180 ? angles[i] : 180;
//		period[i] = ((uint16_t)(
//				angles[i] * ((SERVO_MAX_PERIOD - SERVO_MIN_PREIOD) / 180)) + SERVO_MIN_PREIOD);
//	}
//}
//
//
//
//void Servos::setAngle(uint8_t index, uint8_t angle){
//	if(index < N_SERVOS){
//		angle = (angle <= 180) ? angle : 180;
//		period[index] = ((uint16_t)(
//				angle * ((SERVO_MAX_PERIOD - SERVO_MIN_PREIOD) / 180)) + SERVO_MIN_PREIOD);
//	}
//}
//
//
//
//void Servos::calculate_period(){
//
//	// Arrange periods from lowest to biggest
//	for (int i = 0; i < N_SERVOS; i++) {
//		index[i] = i;
//	}
//	volatile uint16_t a_period[N_SERVOS];
//	for(int i = 0; i < N_SERVOS; i++){
//		a_period[i] = period[i];
//	}
//
//	for (int i = 0; i < N_SERVOS; i++) {
//		for (int j = 0; j < N_SERVOS - i; j++) {
//
//			if(j + 1 <= (N_SERVOS - 1)){
//				if (a_period[j] > a_period[j + 1]) {
//					int aux = a_period[j];
//					a_period[j] = a_period[j + 1];
//					a_period[j + 1] = aux;
//					int aux2 = index[j];
//					index[j] = index[j + 1];
//					index[j + 1] = aux2;
//				}
//			}
//
//		}
//
//	}
//
//	// Calculate time diferences:
//	_tim_period[0] = a_period[0];
//	for (int i = 1; i < N_SERVOS; i++) {
//		_tim_period[i] = (a_period[i] - a_period[i - 1]);
//	}
//	upd_changes = true;
//	this->update();
//}
//
//
//
//void Servos::update(){
//	if(sig_gen == false){
//
//		if (upd_changes == true) {
//			for (int i = 0; i < N_SERVOS; i++) {
//				servo_index[i] = index[i];
//			}
//			for (int i = 0; i < N_SERVOS; i++) {
//				tim_period[i] = _tim_period[i];
//			}
//			upd_changes = false;
//		}
//	}
//}
//
//
//void Servos::servo_timer_loop(TIM_HandleTypeDef *htim) {
//
//	if (_start == 1) {
//
//		// 50Hz Interrupt
//		if (htim == tim1) {
//			sig_gen = true;
//
//			// Set all servos to high:
//			for (int i = 0; i < N_SERVOS; i++) {
//				if(attached_servos[i] == true){
//					HAL_GPIO_WritePin(gpio_port[i], gpio_pin[i], SERVO_HIGH_STATE);
//				}
//			}
//			_50HZ_tick++;
//
//			// Configure timer2
//			TIM2_ARR_REG = tim_period[0];
//			HAL_TIM_Base_Start_IT(tim2);
//
//		}
//
//		// Switching low interrupt
//		else if (htim == tim2) {
//			HAL_TIM_Base_Stop_IT(tim2);
//
//			while (true) {
//				if(attached_servos[ind] == true){
//					HAL_GPIO_WritePin(gpio_port[servo_index[ind]],
//							gpio_pin[servo_index[ind]], SERVO_LOW_STATE);
//				}
//
//				// End switching all servos low
//				if (ind == (N_SERVOS - 1)) {
//					HAL_TIM_Base_Stop_IT(tim2);
//					ind = 0;
//					sig_gen = false;
//					this->calculate_period();
//					this->update();
//					break;
//				}
//
//				// if next period 0, switch now low
//				if (tim_period[ind + 1] == 0) {
//					ind++;
//					continue;
//				}
//
//				// configure next timer interrupt
//				ind++;
//				TIM2_ARR_REG = tim_period[ind];
//				HAL_TIM_Base_Start_IT(tim2);
//				break;
//			}
//
//		}
//	}
//
//}
//
//
//
//
//
