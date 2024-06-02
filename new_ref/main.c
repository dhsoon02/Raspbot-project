#include <time.h>
#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#include "checkIR.h"
#include "controlMotors.h"
#include "stopMotors.h"
#include "controlServo.h"


static int turn_past;


void straight_right();
void straight_left();
void straight_both();



int main(void) {
    time_t start, current;
    double elapsed;
    int* ir_value;
    turn_past = -1;	// 0:past->straight , 1:past->left , 2:past->right
    controlMotors(1, 0, 1, 0);

	
    delay(10000);

    while(1){
	ir_value = checkIR();

// #include <time.h>
// time_t start, current;
// double elapsed;

/*
if(ir_value[0]==0 && ir_value[1]==0 && ir_value[2]==0 &&ir_value[3]==0){
    //0000 우회전
    controlMotors(1,100,0,0);
    delay(1000);
    controlMotors(0,0,0,0);

}



else if(ir_value[0]==1 && ir_value[1]==0 && ir_value[2]==1 &&ir_value[3]==0){
    //1010 우회전
    controlMotors(1,100,0,0);
    delay(1000);
    controlMotors(0,0,0,0);
}
else if(ir_value[0]==0 && ir_value[1]==1 && ir_value[2]==0 &&ir_value[3]==1){
    //0101 좌회전
    controlMotors(0,0,1,100);
    delay(1000);
    controlMotors(0,0,0,0);

}



else if(ir_value[0]==1 && ir_value[1]==0 && ir_value[2]==0 &&ir_value[3]==0){
    //1000 우회전
    controlMotors(1,100,0,0);
    delay(1000);
    controlMotors(0,0,0,0);

}
else if(ir_value[0]==0 && ir_value[1]==0 && ir_value[2]==0 &&ir_value[3]==1){
    //0001 좌회전
    controlMotors(0,0,1,100);
    delay(1000);
    controlMotors(0,0,0,0);
}
else if(ir_value[0]==0 && ir_value[1]==1 && ir_value[2]==1 &&ir_value[3]==1){
    //0111 우회전
    controlMotors(0,0,1,100);
    delay(1000);
    controlMotors(0,0,0,0);
}
else if(ir_value[0]==1 && ir_value[1]==1 && ir_value[2]==1 &&ir_value[3]==0){
    //1110 좌회전
    controlMotors(1,100,0,0);
    delay(1000);
    controlMotors(0,0,0,0);
}
else if(ir_value[0]==0 && ir_value[1]==0 && ir_value[2]==1 &&ir_value[3]==1){
    //0011 우회전 ??
    controlMotors(0,0,1,100);
    delay(1000);
    controlMotors(0,0,0,0);
}
else if(ir_value[0]==1 && ir_value[1]==1 && ir_value[2]==0 &&ir_value[3]==0){
    //1100 좌회전 ??
    controlMotors(1,100,0,0);
    delay(1000);
    controlMotors(0,0,0,0);
}




else if(ir_value[0]==1 && ir_value[1]==0 && ir_value[2]==1 &&ir_value[3]==1){
    //1011 우회전
    controlMotors(0,0,1,120);
    delay(20);
}
else if(ir_value[0]==1 && ir_value[1]==1 && ir_value[2]==0 &&ir_value[3]==1){
    //1101 좌회전
    controlMotors(1,120,0,0);
    delay(20);
}

else{
    controlMotors(1,50,1,50);
}

*/


// KIM

	/*
	if(ir_value[0] == 0){ // left grid
	    controlMotors(0, 0, 0, 0);
	    turn_past = -1;
	    delay(800);
	    controlMotors(0, 0, 1, 80);
	    turn_past = 1;
	    delay(900);
	    controlMotors(0, 0, 1, 60);
	    while(1){
		ir_value = checkIR();
		if(ir_value[3] == 0) {
		    controlMotors(0, 0, 0, 0);
		    turn_past = -1;
		    delay(800);
		    controlMotors(1, 60, 0, 0);
		    turn_past = 2;
		    break;
		}
	    }
	    while(1){
		ir_value = checkIR();
		if(ir_value[1] == 0) {
		    controlMotors(0, 0, 0, 0);
		    delay(800);
		    break;
		}
	    }
	    
	}
	
	else if(ir_value[3] == 0){ // right grid
	    controlMotors(0, 0, 0, 0);
	    turn_past = -1;
	    delay(800);
	    controlMotors(1, 80, 0, 0);
	    turn_past = 2;
	    delay(900);
	    controlMotors(1, 60, 0, 0);
	    while(1){
		ir_value = checkIR();
		if(ir_value[0] == 0) {
		    controlMotors(0, 0, 0, 0);
		    turn_past = -1;
		    delay(800);
		    controlMotors(0, 0, 1, 60);
		    turn_past = 1;
		    break;
		}
	    }
	    while(1){
		ir_value = checkIR();
		if(ir_value[2] == 0) {
		    controlMotors(0, 0, 0, 0);
		    delay(800);
		    break;
		}
	    }
	}
	
	else if(ir_value[0] == 1 && ir_value[1] == 1 && ir_value[2] == 0 && ir_value[3] == 1){ // line
	    // turn right
	    if (turn_past != 2) {
		controlMotors(1, 70, 0, 70);
		turn_past = 2;
	    }
	}
	else if(ir_value[0] == 1 && ir_value[1] == 0 && ir_value[2] == 1 && ir_value[3] == 1){ // line
	    // turn left
	    if (turn_past != 1) {
		controlMotors(0, 70, 1, 70);
		turn_past = 1;
	    }
	}
	else if(ir_value[0] == 1 && ir_value[1] == 0 && ir_value[2] == 0 && ir_value[3] == 1){ // line
	    // turn left
	    if (turn_past != 0) {
		controlMotors(1, 70, 1, 70);
		turn_past = 0;
	    }
	}
	
	else{
	    if (turn_past == 0) {
		controlMotors(0, 50, 0, 50);
	    }
	    else if (turn_past == 1) {
		controlMotors(1, 50, 0, 50);
	    }
	    else if (turn_past == 2) {
		controlMotors(0, 50, 1, 50);
	    }
	}
	*/

	straight_both();


    }

    return 0;
}










void straight_right() {
    
    int* ir_value = checkIR();
    
    if(ir_value[3] == 0){ // right grid
	controlMotors(0, 0, 0, 0);
	turn_past = -1;
	delay(800);
	controlMotors(1, 80, 0, 0);
	turn_past = 2;
	delay(900);
	controlMotors(1, 60, 0, 0);
	while(1){
	    ir_value = checkIR();
	    if(ir_value[0] == 0) {
		controlMotors(0, 0, 0, 0);
		turn_past = -1;
		delay(800);
		controlMotors(0, 0, 1, 60);
		turn_past = 1;
		break;
	    }
	}
	while(1){
	    ir_value = checkIR();
	    if(ir_value[2] == 0) {
		controlMotors(0, 0, 0, 0);
		delay(800);
		break;
	    }
	}
    }
    
    else if(ir_value[0] == 1 && ir_value[1] == 1 && ir_value[2] == 0 && ir_value[3] == 1){ // line
	// turn right
	if (turn_past != 2) {
	    controlMotors(1, 70, 0, 70);
	    turn_past = 2;
	}
    }
    else if(ir_value[0] == 1 && ir_value[1] == 0 && ir_value[2] == 1 && ir_value[3] == 1){ // line
	// turn left
	if (turn_past != 1) {
	    controlMotors(0, 70, 1, 70);
	    turn_past = 1;
	}
    }
    else if(ir_value[0] == 1 && ir_value[1] == 0 && ir_value[2] == 0 && ir_value[3] == 1){ // line
	// turn left
	if (turn_past != 0) {
	    controlMotors(1, 70, 1, 70);
	    turn_past = 0;
	}
    }
    
    else{
	if (turn_past == 0) {
	    controlMotors(0, 50, 0, 50);
	}
	else if (turn_past == 1) {
	    controlMotors(1, 50, 0, 50);
	}
	else if (turn_past == 2) {
	    controlMotors(0, 50, 1, 50);
	}
    }
    
}










void straight_left() {
    
    int* ir_value = checkIR();
    
    if(ir_value[0] == 0){ // left grid
	controlMotors(0, 0, 0, 0);
	turn_past = -1;
	delay(800);
	controlMotors(0, 0, 1, 80);
	turn_past = 1;
	delay(900);
	controlMotors(0, 0, 1, 60);
	while(1){
	    ir_value = checkIR();
	    if(ir_value[3] == 0) {
		controlMotors(0, 0, 0, 0);
		turn_past = -1;
		delay(800);
		controlMotors(1, 60, 0, 0);
		turn_past = 2;
		break;
	    }
	}
	while(1){
	    ir_value = checkIR();
	    if(ir_value[1] == 0) {
		controlMotors(0, 0, 0, 0);
		delay(800);
		break;
	    }
	}
	
    }
    
    else if(ir_value[0] == 1 && ir_value[1] == 1 && ir_value[2] == 0 && ir_value[3] == 1){ // line
	// turn right
	if (turn_past != 2) {
	    controlMotors(1, 70, 0, 70);
	    turn_past = 2;
	}
    }
    else if(ir_value[0] == 1 && ir_value[1] == 0 && ir_value[2] == 1 && ir_value[3] == 1){ // line
	// turn left
	if (turn_past != 1) {
	    controlMotors(0, 70, 1, 70);
	    turn_past = 1;
	}
    }
    else if(ir_value[0] == 1 && ir_value[1] == 0 && ir_value[2] == 0 && ir_value[3] == 1){ // line
	// turn left
	if (turn_past != 0) {
	    controlMotors(1, 70, 1, 70);
	    turn_past = 0;
	}
    }
    
    else{
	if (turn_past == 0) {
	    controlMotors(0, 50, 0, 50);
	}
	else if (turn_past == 1) {
	    controlMotors(1, 50, 0, 50);
	}
	else if (turn_past == 2) {
	    controlMotors(0, 50, 1, 50);
	}
    }
    
}










void straight_both() {
    
    int* ir_value = checkIR();


    if(ir_value[3] == 0){ // right grid
	controlMotors(0, 0, 0, 0);
	turn_past = -1;
	delay(80);

	controlMotors(1, 100, 0, 0);
	turn_past = 2;
	delay(700);

	controlMotors(1, 80, 0, 0);
	turn_past = 2;
	while(1){
	    ir_value = checkIR();
	    if(ir_value[0] == 0) {
		controlMotors(0, 0, 0, 0);
		turn_past = -1;
		delay(80);
		break;
	    }
	}

	controlMotors(0, 0, 1, 60);
	turn_past = 1;
	while(1){
	    ir_value = checkIR();
	    if(ir_value[2] == 0) {
		controlMotors(0, 0, 0, 0);
		turn_past = -1;
		delay(80);
		break;
	    }
	}
    }


    else if(ir_value[0] == 0){ // left grid
	controlMotors(0, 0, 0, 0);
	turn_past = -1;
	delay(80);

	controlMotors(0, 0, 1, 100);
	turn_past = 1;
	delay(700);

	controlMotors(0, 0, 1, 80);
	turn_past = 1;
	while(1){
	    ir_value = checkIR();
	    if(ir_value[3] == 0) {
		controlMotors(0, 0, 0, 0);
		turn_past = -1;
		delay(80);
		break;
	    }
	}

	controlMotors(1, 60, 0, 0);
	turn_past = 2;
	while(1){
	    ir_value = checkIR();
	    if(ir_value[1] == 0) {
		controlMotors(0, 0, 0, 0);
		turn_past = -1;
		delay(80);
		break;
	    }
	}
    }

    

    else if(ir_value[0] == 1 && ir_value[1] == 1 && ir_value[2] == 0 && ir_value[3] == 1){ // line
	// turn right
	//if (turn_past != 2) {
	controlMotors(1, 100, 0, 0);
	turn_past = 2;
	//}
    }
    else if(ir_value[0] == 1 && ir_value[1] == 0 && ir_value[2] == 1 && ir_value[3] == 1){ // line
	// turn left
	//if (turn_past != 1) {
	controlMotors(0, 0, 1, 100);
	turn_past = 1;
	//}
    }
    else if(ir_value[0] == 1 && ir_value[1] == 0 && ir_value[2] == 0 && ir_value[3] == 1){ // line
	// turn left
	//if (turn_past != 0) {
	controlMotors(1, 60, 1, 60);
	turn_past = 0;
	//}
    }

    else{	// 1111
	
	stopMotors();
	delay(1000);
	

	controlMotors(1, 100, 0, 0);
	while (1) {
	    ir_value = checkIR();
	    if (ir_value[0] == 0) {
		controlMotors(0, 0, 1, 100);
		while (1) {
		    ir_value = checkIR();
		    if (ir_value[2] == 0) {
			controlMotors(0, 0, 0, 0);
			delay(2000);
			break;
		    }
		}

		break;
	    }

	    else if (ir_value[1] == 0) {
		controlMotors(0, 0, 0, 0);
		delay(2000);
		break;
	    }

	} // end of while (1)



    } // end of else (1111)

}


