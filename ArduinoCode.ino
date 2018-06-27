
////////////////////////////////// LIBRARIES //////////////////////////////////////

#include <Arduino.h>
#include <Wire.h>
#include <servo_PCA9685.h>
#include <ESP8266WiFi.h>
#include <SocketIoClient.h>
#include <ArduinoJson.h>
#define us Serial


////////////////////////////////// GLOBAL DEFINE //////////////////////////////////////


WiFiClient client;
SocketIoClient webSocket;
servo_PCA9685 servo = servo_PCA9685();


const char* ssid     = "ArduinoNode";
const char* password = "12345678";
char path[] = "/";
char host[] = "192.168.137.1";//192.168.137.1  10.9.9.160
int port = 2208;



#define intv_1 50
unsigned long time_1 = 0;



////////////////////////////////// LOOP AND SETUP //////////////////////////////////////

void setup() {
  
    #if defined(ESP8266)
      us.begin(115200);
    #else
      us.begin(38400);
    #endif
      servo.begin();
      
    
    us.setDebugOutput(true);

    
    for(uint8_t t = 4; t > 0; t--) {
        us.printf("[SETUP] BOOT WAIT %d...\n", t);
        us.flush();
        delay(500);
    }
    
    wifi_connection();


    webSocket.on("connect", connection);
    webSocket.begin(host, port);
}

void loop() {

    /*
    if(millis() > time_1 + intv_1){
        webSocket.loop();
        delay(5);
        webSocket.on("message", message);
        delay(5);
    }*/
    
    walk_step(0);delay(1000);
    
}


////////////////////////////////////// BINDING CONNECTION ///////////////////////////////////////////

void wifi_connection(){
    us.print("\n");
    us.printf("WiFi Connecting to :: ");
    us.printf(ssid);
  
    WiFi.begin(ssid, password);
  
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        us.printf(".");
    }

    us.print("\n");
    us.printf("WiFi connected");  
    us.printf("IP address: ");
    us.print(WiFi.localIP());

    app_connection();
}





void app_connection(){
    us.print("\n");
    us.printf("App Connecting to :: ");
    us.printf(host);
    
    if (client.connect(host, port)) {
        us.printf("\nApp Server Connected\n");
    } else {
        us.printf("\nApp Server Connection failed\n");
    }
    
    us.print("");
}



////////////////////////////////////////// SOCKET HANDLER////////////////////////////////////////////////////

void connection(const char * payload, size_t length) {
    us.printf("connection message: %s\n", payload);
}

void message(const char * payload, size_t length) {
    parse_data(payload);//parse data 
}




int is_run = 0;
const char * prev_action = "idle";

void parse_data(String msg){
    StaticJsonBuffer<2048> JSONBuffer;                     //Memory pool
    JsonObject& parsed = JSONBuffer.parseObject(msg); //Parse message

    us.print("\n");
    
    if (parsed.success()) {
        int statuss = parsed["status"];

        if(statuss == 1){
            int type_con = parsed["type_con"];
            int menu_type = parsed["menu_type"];            
            
            //us.print("parse action:: ");            
            //us.print(" \n");

            is_run = 1;

            if(menu_type == 1){
                const char * action = parsed["action"];

                if(type_con == 1 && is_run == 1){
                    
                    const char * namee = parsed["name"];    
        
                    int snum = parsed["snum"];
            

                    int is_leg = parsed["is_leg"];
                    int is_left = parsed["is_left"];

                    if(is_leg==1){
                        int t1 = parsed["t1"];
                        int t2 = parsed["t2"];
                        int t3 = parsed["t3"];
                        int mintetha = parsed["ref_min"];
                        int maxtetha = parsed["ref_max"];
                        if(is_left==1){
                            leg_FK(1, t1, t2, t3, (maxtetha-mintetha) );
                        }else{
                            leg_FK(2, t1, t2, t3, (maxtetha-mintetha) );
                        }
                    }else{
                        int t1 = parsed["t1"];
                        int t2 = parsed["t2"];
                        int mintetha = parsed["ref_min"];
                        int maxtetha = parsed["ref_max"];
                        if(is_left==1){
                            hand_FK(2, t1, t2, (maxtetha-mintetha) );
                        }else{
                            hand_FK(1, t1, t2, (maxtetha-mintetha) );
                        }
                    }
                    
                    upload_servos();

                }
              
            }else if(menu_type == 2){
             //trigger params
                int snum = parsed["snum"];
            
                int gVelo = parsed["gVelo"];
                int passVal = parsed["passVal"];
                int cVal = parsed["cVal"];
            }
        }else{
            //trigger no params
            is_run = 0;
            prev_action = "idle";

            stand();
        }

        us.print("\n");
//        us.print(is_run);
        us.print("\n");
        
    }else{
        //us.print("Parsing failed \n"); 
        us.print(msg);
    }
    us.print("\n"); 
}



//////////////////////////////// KINEMATIC PARAMETERS ///////////////////////////////////////////////////

double theta_leg[] = {0.0, 0.0, 0.0};
double theta_hand[] = {0.0, 0.0};

double l1 = 4.75, l2 = 1.85, l3 = 2.9;
double d2 = 0, d3 = 0;

double a1 = 2.7473, a2 = 4.6666;

double pi = 3.141592653589793;




//////////////////////////////// PWM POSITIONING ///////////////////////////////////////////////////

int ref_right_hand_min[]  = {100, 250};
int ref_right_hand[]      = {300, 415};
int ref_right_hand_max[]  = {500, 500};
int ref_right_hand_step[] = {0, 0, 0, 0, 0, 0};

int ref_right_leg_min[]    = {150, 300, 270};//thigh, knee, angle
int ref_right_leg[]        = {250, 400, 300};//thigh, knee, angle
int ref_right_leg_max[]    = {350, 600, 330};//thigh, knee, angle
int ref_right_leg_step[]   = {0, 0, 0, 0, 0, 0, 0, 0, 0}; //mn nr mx



int ref_left_hand_min[]   = {150, 250};
int ref_left_hand[]       = {300, 375};//sholder elbow
int ref_left_hand_max[]   = {500, 500};
int ref_left_hand_step[]  = {0, 0, 0, 0, 0, 0};

int ref_left_leg_min[]   = {300, 275, 345};
int ref_left_leg[]       = {400, 400, 375};
int ref_left_leg_max[]   = {500, 600, 405};
int ref_left_leg_step[]  = {0, 0, 0, 0, 0, 0, 0, 0, 0}; //mn nr mx


int tetha_left_leg[] = {0, 0, 0};//thigh, knee, angle
int tetha_right_leg[] = {0, 0, 0};

int tetha_left_hand[] = {0, 0};//sholder elbow
int tetha_right_hand[] = {0, 0};

///////////////////////////////// SERVOS PIN INIT ///////////////////////////////////////////////

int lh_j1 = 0;
int lh_j2 = 1;
int ll_j1 = 2;
int ll_j2 = 3;
int ll_j3 = 4;

int rh_j1 = 8;
int rh_j2 = 9;
int rl_j1 = 10;
int rl_j2 = 11;
int rl_j3 = 12;


////////////////////////////////// FORWARD KINAMITCS ANALYSIS //////////////////////////////////////////////////////

void leg_FK(int tp, double cpwm1, double cpwm2, double cpwm3, double range){
    double q1; double q2; double q3;

    if(tp == 1){
        if( (cpwm1 < ref_right_leg[0]) && (cpwm1 >= ref_right_leg_min[0]) ){
            q1 = (ref_right_leg[0] - cpwm1) * range / (ref_right_leg[0]-ref_right_leg_min[0]);
            ref_right_leg_step[0] = 1; ref_right_leg_step[1] = 0; ref_right_leg_step[2] = 0;
        }
        if( cpwm1 <= ref_right_leg_min[0] ){
            q1 = (ref_right_leg[0] - ref_right_leg_min[0]) * range / (ref_right_leg[0]-ref_right_leg_min[0]);
            ref_right_leg_step[0] = 1; ref_right_leg_step[1] = 0; ref_right_leg_step[2] = 0;
        }
        if( (cpwm1 > ref_right_leg[0]) && (cpwm1 <= ref_right_leg_max[0]) ){
            q1 = (cpwm1 - ref_right_leg[0]) * range / (ref_right_leg_max[0]-ref_right_leg[0]);
            ref_right_leg_step[0] = 0; ref_right_leg_step[1] = 0; ref_right_leg_step[2] = 1;
        }
        if( cpwm1 >= ref_right_leg_max[0] ){
            q1 = (ref_right_leg_max[0] - ref_right_leg[0]) * range / (ref_right_leg_max[0]-ref_right_leg[0]);
            ref_right_leg_step[0] = 0; ref_right_leg_step[1] = 0; ref_right_leg_step[2] = 1;
        }
        if(cpwm1 == ref_right_leg[0]){
            q1 = cpwm1 - ref_right_leg[0];
            ref_right_leg_step[0] = 0; ref_right_leg_step[1] = 1; ref_right_leg_step[2] = 0;
        }



        if( (cpwm2 < ref_right_leg[1]) && (cpwm2 >= ref_right_leg_min[1]) ){
            q2 = (ref_right_leg[1] - cpwm2) * range / (ref_right_leg[1]-ref_right_leg_min[1]);
            ref_right_leg_step[3] = 1; ref_right_leg_step[4] = 0; ref_right_leg_step[5] = 0;
        }
        if( cpwm2 <= ref_right_leg_min[1] ){
            q1 = (ref_right_leg[1] - ref_right_leg_min[1]) * range / (ref_right_leg[1]-ref_right_leg_min[1]);
            ref_right_leg_step[3] = 1; ref_right_leg_step[4] = 0; ref_right_leg_step[5] = 0;
        }
        if( (cpwm2 > ref_right_leg[1]) && (cpwm2 <= ref_right_leg_max[1]) ){
            q2 = (cpwm2 - ref_right_leg[1]) * range / (ref_right_leg_max[1]-ref_right_leg[1]);
            ref_right_leg_step[3] = 0; ref_right_leg_step[4] = 0; ref_right_leg_step[5] = 1;
        }
        if( cpwm2 >= ref_right_leg_max[1] ){
            q1 = (ref_right_leg_max[1] - ref_right_leg[1]) * range / (ref_right_leg_max[1]-ref_right_leg[1]);
            ref_right_leg_step[3] = 0; ref_right_leg_step[4] = 0; ref_right_leg_step[5] = 1;
        }
        if(cpwm2 == ref_right_leg[1]){
            q2 = cpwm2 - ref_right_leg[1];
            ref_right_leg_step[3] = 0; ref_right_leg_step[4] = 1; ref_right_leg_step[5] = 0;
        }

        

        if( (cpwm3 < ref_right_leg[2]) && (cpwm3 >= ref_right_leg_min[2]) ){
            q3 = (ref_right_leg[2] - cpwm3) * range / (ref_right_leg[2]-ref_right_leg_min[2]);
            ref_right_leg_step[6] = 1; ref_right_leg_step[7] = 0; ref_right_leg_step[8] = 0;
        }
        if( cpwm3 <= ref_right_leg_min[2] ){
            q3 = (ref_right_leg[2] - ref_right_leg_min[2]) * range / (ref_right_leg[2]-ref_right_leg_min[2]);
            ref_right_leg_step[6] = 1; ref_right_leg_step[7] = 0; ref_right_leg_step[8] = 0;
        }
        if( (cpwm3 > ref_right_leg[2]) && (cpwm3 <= ref_right_leg_max[2]) ){
            q3 = (cpwm3 - ref_right_leg[2]) * range / (ref_right_leg_max[2]-ref_right_leg[2]);
            ref_right_leg_step[6] = 0; ref_right_leg_step[7] = 0; ref_right_leg_step[8] = 1;
        }
        if( cpwm3 >= ref_right_leg_max[2] ){
            q3 = (ref_right_leg_max[2] - ref_right_leg[2]) * range / (ref_right_leg_max[2]-ref_right_leg[2]);
            ref_right_leg_step[6] = 0; ref_right_leg_step[7] = 0; ref_right_leg_step[8] = 1;
        }
        if(cpwm3 == ref_right_leg[2]){
            q3 = cpwm3 - ref_right_leg[2];
            ref_right_leg_step[6] = 0; ref_right_leg_step[7] = 1; ref_right_leg_step[8] = 0;
        }
    }else{
        if( (cpwm1 < ref_left_leg[0]) && (cpwm1 >= ref_left_leg_min[0]) ){
            q1 = (ref_left_leg[0] - cpwm1) * range / (ref_left_leg[0]-ref_left_leg_min[0]);
            ref_left_leg_step[0] = 1; ref_left_leg_step[1] = 0; ref_left_leg_step[2] = 0;
        }
        if( cpwm1 <= ref_left_leg_min[0] ){
            q1 = (ref_left_leg[0] - ref_left_leg_min[0]) * range / (ref_left_leg[0]-ref_left_leg_min[0]);
            ref_left_leg_step[0] = 1; ref_left_leg_step[1] = 0; ref_left_leg_step[2] = 0;
        }
        if( (cpwm1 > ref_left_leg[0]) && (cpwm1 <= ref_left_leg_max[0]) ){
            q1 = (cpwm1 - ref_left_leg[0]) * range / (ref_left_leg_max[0]-ref_left_leg[0]);
            ref_left_leg_step[0] = 0; ref_left_leg_step[1] = 0; ref_left_leg_step[2] = 1;
        }
        if( cpwm1 >= ref_left_leg_max[0] ){
            q1 = (ref_left_leg_max[0] - ref_left_leg[0]) * range / (ref_left_leg_max[0]-ref_left_leg[0]);
            ref_left_leg_step[0] = 0; ref_left_leg_step[1] = 0; ref_left_leg_step[2] = 1;
        }
        if(cpwm1 == ref_left_leg[0]){
            q1 = cpwm1 - ref_left_leg[0];
            ref_left_leg_step[0] = 0; ref_left_leg_step[1] = 1; ref_left_leg_step[2] = 0;
        }



        if( (cpwm2 < ref_left_leg[1]) && (cpwm2 >= ref_left_leg_min[1]) ){
            q2 = (ref_left_leg[1] - cpwm2) * range / (ref_left_leg[1]-ref_left_leg_min[1]);
            ref_left_leg_step[3] = 1; ref_left_leg_step[4] = 0; ref_left_leg_step[5] = 0;
        }
        if( cpwm2 <= ref_left_leg_min[1] ){
            q2 = (ref_left_leg[1] - ref_left_leg_min[1]) * range / (ref_left_leg[1]-ref_left_leg_min[1]);
            ref_left_leg_step[3] = 1; ref_left_leg_step[4] = 0; ref_left_leg_step[5] = 0;
        }
        if( (cpwm2 > ref_left_leg[1]) && (cpwm2 <= ref_left_leg_max[1]) ){
            q2 = (cpwm2 - ref_left_leg[1]) * range / (ref_left_leg_max[1]-ref_left_leg[1]);
            ref_left_leg_step[3] = 0; ref_left_leg_step[4] = 0; ref_left_leg_step[5] = 1;
        }
        if( cpwm2 >= ref_left_leg_max[1] ){
            q2 = (ref_left_leg_max[1] - ref_left_leg[1]) * range / (ref_left_leg_max[1]-ref_left_leg[1]);
            ref_left_leg_step[3] = 0; ref_left_leg_step[4] = 0; ref_left_leg_step[5] = 1;
        }
        if(cpwm2 == ref_left_leg[1]){
            q2 = cpwm2 - ref_left_leg[1];
            ref_left_leg_step[3] = 0; ref_left_leg_step[4] = 1; ref_left_leg_step[5] = 0;
        }

        

        if( (cpwm3 < ref_left_leg[2]) && (cpwm3 >= ref_left_leg_min[2]) ){
            q3 = (ref_left_leg[2] - cpwm3) * range / (ref_left_leg[2]-ref_left_leg_min[2]);
            ref_left_leg_step[6] = 1; ref_left_leg_step[7] = 0; ref_left_leg_step[8] = 0;
        }
        if( cpwm3 <= ref_left_leg_min[2] ){
            q3 = (ref_left_leg[2] - ref_left_leg_min[2]) * range / (ref_left_leg[2]-ref_left_leg_min[2]);
            ref_left_leg_step[6] = 1; ref_left_leg_step[7] = 0; ref_left_leg_step[8] = 0;
        }
        if( (cpwm3 > ref_left_leg[2]) && (cpwm3 <= ref_left_leg_max[2]) ){
            q3 = (cpwm3 - ref_left_leg[2]) * range / (ref_left_leg_max[2]-ref_left_leg[2]);
            ref_left_leg_step[6] = 0; ref_left_leg_step[7] = 0; ref_left_leg_step[8] = 1;
        }
        if( cpwm3 >= ref_left_leg_max[2] ){
            q3 = (ref_left_leg_max[2] - ref_left_leg[2]) * range / (ref_left_leg_max[2]-ref_left_leg[2]);
            ref_left_leg_step[6] = 0; ref_left_leg_step[7] = 0; ref_left_leg_step[8] = 1;
        }
        if(cpwm3 == ref_left_leg[2]){
            q3 = cpwm3 - ref_left_leg[2];
            ref_left_leg_step[6] = 0; ref_left_leg_step[7] = 1; ref_left_leg_step[8] = 0;
        }
    }

    float c12 = ( cos(deg(q1))*cos(deg(q2)) ) - ( sin(deg(q1))*sin(deg(q2)) );
    float s12 = ( sin(deg(q1))*cos(deg(q2)) ) + ( cos(deg(q1))*sin(deg(q2)) );
    
    double px = ( ( l3 * cos(deg(q3)) * c12 ) + (l2*c12) + ( l1 * cos(deg(q1)) ) );
    double py = ( ( l3 * cos(deg(q3)) * s12 ) + (l2*s12) + ( l1 * sin(deg(q1)) ) );
    double pz = - ( l3 * sin(deg(q3)) ) ;//sin(-q1*-180/pi);

    


    /*
    Serial.println(" ");
    Serial.print("leg_FK => ");
    Serial.print("Px:: ");Serial.print(px);Serial.print(", ");
    Serial.print("Py:: ");Serial.print(py);Serial.print(", ");
    Serial.print("Pz:: ");Serial.print(pz);
    Serial.println(" ");
    
    Serial.print("leg_FK => ");
    Serial.print("Q1");Serial.print(" ( ");Serial.print(cpwm1);Serial.print(" ) :: ");Serial.print(q1);Serial.print(", ");
    Serial.print("Q2");Serial.print(" ( ");Serial.print(cpwm2);Serial.print(" ) :: ");Serial.print(q2);Serial.print(", ");
    Serial.print("Q3");Serial.print(" ( ");Serial.print(cpwm3);Serial.print(" ) :: ");Serial.print(q3);
    Serial.println(" ");
    */

    

    leg_IK(px, py, pz);

    if(tp == 1){
        looking_for_tetha(3);
    }else{
        looking_for_tetha(4);
    }
}






void hand_FK(int tp, double cpwm1, double cpwm2, double range){
    double q1; double q2;
    if(tp == 1){
        if( (cpwm1 < ref_right_hand[0]) && (cpwm1 >= ref_right_hand_min[0]) ){
            q1 = (ref_right_hand[0] - cpwm1) * range / (ref_right_hand[0]-ref_right_hand_min[0]);
            ref_right_hand_step[0] = 1; ref_right_hand_step[1] = 0; ref_right_hand_step[2] = 0;
        }
        if( cpwm1 <= ref_right_hand_min[0] ){
            q1 = (ref_right_hand[0] - ref_right_hand_min[0]) * range / (ref_right_hand[0]-ref_right_hand_min[0]);
            ref_right_hand_step[0] = 1; ref_right_hand_step[1] = 0; ref_right_hand_step[2] = 0;
        }
        if( (cpwm1 > ref_right_hand[0]) && (cpwm1 <= ref_right_hand_max[0]) ){
            q1 = (cpwm1 - ref_right_hand[0]) * range / (ref_right_hand_max[0]-ref_right_hand[0]);
            ref_right_hand_step[0] = 0; ref_right_hand_step[1] = 0; ref_right_hand_step[2] = 1;
        }
        if( cpwm1 >= ref_right_hand_max[0] ){
            q1 = (ref_right_hand_max[0] - ref_right_hand[0]) * range / (ref_right_hand_max[0]-ref_right_hand[0]);
            ref_right_hand_step[0] = 0; ref_right_hand_step[1] = 0; ref_right_hand_step[2] = 1;
        }
        if(cpwm1 == ref_right_hand[0]){
            q1 = cpwm1 - ref_right_hand[0];
            ref_right_hand_step[0] = 0; ref_right_hand_step[1] = 1; ref_right_hand_step[2] = 0;
        }



        if( (cpwm2 < ref_right_hand[1]) && (cpwm2 >= ref_right_hand_min[1]) ){
            q2 = (ref_right_hand[1] - cpwm2) * range / (ref_right_hand[1]-ref_right_hand_min[1]);
            ref_right_hand_step[3] = 1; ref_right_hand_step[4] = 0; ref_right_hand_step[5] = 0;
        }
        if( cpwm2 <= ref_right_hand_min[1] ){
            q2 = (ref_right_hand[1] - ref_right_hand_min[1]) * range / (ref_right_hand[1]-ref_right_hand_min[1]);
            ref_right_hand_step[3] = 1; ref_right_hand_step[4] = 0; ref_right_hand_step[5] = 0;
        }
        if( cpwm2 >= ref_right_hand_max[1] ){
            q2 = (ref_right_hand_max[1] - ref_right_hand[1]) * range / (ref_right_hand_max[1]-ref_right_hand[1]);
            ref_right_hand_step[3] = 0; ref_right_hand_step[4] = 0; ref_right_hand_step[5] = 1;
        }
        if( (cpwm2 > ref_right_hand[1]) && (cpwm2 <= ref_right_hand_max[1]) ){
            q2 = (cpwm2 - ref_right_hand[1]) * range / (ref_right_hand_max[1]-ref_right_hand[1]);
            ref_right_hand_step[3] = 0; ref_right_hand_step[4] = 0; ref_right_hand_step[5] = 1;
        }
        if(cpwm2 == ref_right_hand[1]){
            q2 = cpwm2 - ref_right_hand[1];
            ref_right_hand_step[3] = 0; ref_right_hand_step[4] = 1; ref_right_hand_step[5] = 0;
        }
    }else{
        if( (cpwm1 < ref_left_hand[0]) && (cpwm1 >= ref_left_hand_min[0]) ){
            q1 = (ref_left_hand[0] - cpwm1) * range / (ref_left_hand[0]-ref_left_hand_min[0]);
            ref_left_hand_step[0] = 1; ref_left_hand_step[1] = 0; ref_left_hand_step[2] = 0;
        }
        if( (cpwm1 > ref_left_hand[0]) && (cpwm1 <= ref_left_hand_max[0]) ){
            q1 = (cpwm1 - ref_left_hand[0]) * range / (ref_left_hand_max[0]-ref_left_hand[0]);
            ref_left_hand_step[0] = 0; ref_left_hand_step[1] = 0; ref_left_hand_step[2] = 1;
        }
        if( cpwm1 <= ref_left_hand_min[0] ){
            q1 = (ref_left_hand[0] - ref_left_hand_min[0]) * range / (ref_left_hand[0]-ref_left_hand_min[0]);
            ref_left_hand_step[0] = 1; ref_left_hand_step[1] = 0; ref_left_hand_step[2] = 0;
        }
        if( cpwm1 >= ref_left_hand_max[0] ){
            q1 = (ref_left_hand_max[0] - ref_left_hand[0]) * range / (ref_left_hand_max[0]-ref_left_hand[0]);
            ref_left_hand_step[0] = 0; ref_left_hand_step[1] = 0; ref_left_hand_step[2] = 1;
        }
        if(cpwm1 == ref_left_hand[0]){
            q1 = cpwm1 - ref_left_hand[0];
            ref_left_hand_step[0] = 0; ref_left_hand_step[1] = 1; ref_left_hand_step[2] = 0;
        }

        if( (cpwm2 < ref_left_hand[1]) && (cpwm2 >= ref_left_hand_min[1]) ){
            q2 = (ref_left_hand[1] - cpwm2) * range / (ref_left_hand[1]-ref_left_hand_min[1]);
            ref_left_hand_step[3] = 1; ref_left_hand_step[4] = 0; ref_left_hand_step[5] = 0;
        }
        if( (cpwm2 > ref_left_hand[1]) && (cpwm2 <= ref_left_hand_max[1]) ){
            q2 = (cpwm2 - ref_left_hand[1]) * range / (ref_left_hand_max[1]-ref_left_hand[1]);
            ref_left_hand_step[3] = 0; ref_left_hand_step[4] = 0; ref_left_hand_step[5] = 1;
        }
        if( cpwm1 <= ref_left_hand_min[0] ){
            q1 = (ref_left_hand[0] - ref_left_hand_min[0]) * range / (ref_left_hand[0]-ref_left_hand_min[0]);
            ref_left_hand_step[3] = 1; ref_left_hand_step[4] = 0; ref_left_hand_step[5] = 0;
        }
        if( cpwm1 >= ref_left_hand_max[0] ){
            q1 = (ref_left_hand_max[0] - ref_left_hand[0]) * range / (ref_left_hand_max[0]-ref_left_hand[0]);
            ref_left_hand_step[3] = 0; ref_left_hand_step[4] = 0; ref_left_hand_step[5] = 1;
        }
        if(cpwm2 == ref_left_hand[1]){
            q2 = cpwm2 - ref_left_hand[1];
            ref_left_hand_step[3] = 0; ref_left_hand_step[4] = 1; ref_left_hand_step[5] = 0;
        }
    }

    double px = ( a2 * cos(deg(q1)) * cos(deg(q2)) ) + ( a1 *  cos(deg(q1)) );
    double py = ( a2 * sin(deg(q1)) * cos(deg(q2)) ) + ( a1 *  sin(deg(q1)) );
    double pz = ( a2 *  sin(deg(q2)) );




    /*
    Serial.println(" ");
    Serial.print("hand_FK => ");
    Serial.print("Px:: ");Serial.print(px);Serial.print(", ");
    Serial.print("Py:: ");Serial.print(py);Serial.print(", ");
    Serial.print("Pz:: ");Serial.print(pz);
    
    Serial.println(" ");
    Serial.print("hand_FK => ");
    Serial.print("Q1:: ");Serial.print(" ( ");Serial.print(cpwm1);Serial.print(" ) :: ");Serial.print(q1);Serial.print(", ");
    Serial.print("Q2:: ");Serial.print(" ( ");Serial.print(cpwm2);Serial.print(" ) :: ");Serial.print(q2);
    Serial.println(" ");
    */

    
    
    hand_IK(px, py, pz);
    
    if(tp == 1){
        looking_for_tetha(1);
    }else{
        looking_for_tetha(2);
    }
    
}


//////////////////////////////INVERSE KINAMITCS ANALYSIS /////////////////////////////////////////////

void leg_IK(double px, double py, double pz){
    float s3 = -pz/l3; 
    float c3 = sqrt(abs(1 - (s3 * s3)));
    double t3 = atan2(s3, c3) / 2 / pi * 360 ;
    if(t3 > range){
        theta_leg[2] = 180 - t3;
    }else{
        theta_leg[2] = t3 ; 
    }


    float mn = (l3*l3) + (l2*l2) + (2 * l2* l3 * c3); 
    float Q = ( (px*px) + (py*py) + (l1*l1) - mn ) / ( 2*l1 );
    float txy2 = sqrt((px*px) + (py*py) - (Q*Q));
    double t1 = ( atan2(py, px)  -  atan2(txy2, Q) ) / 2 / pi * 360; //if th+ then -
    if(t1 > range){
        theta_leg[0] = 180 - t1;
    }else{
        theta_leg[0] = t1 ;
    }


    float k = (px * cos(deg(theta_leg[0]))) + (py * sin(deg(theta_leg[0]))) - l1; 
    float n = (py * cos(deg(theta_leg[0]))) - (px * sin(deg(theta_leg[0]))); 
    float l = l3 * cos(deg(theta_leg[2])) + l2; 

    float d = abs(1 / ( l*l ));
    float c2 = (k * l )/ d;
    float s2 = (n * l) / d;
    double t2 = atan2(s2, c2) / 2 / pi * 360;
    if(t2 > range){
        theta_leg[1] = 180 - t2;
    }else{
        theta_leg[1] = t2 ;
    }

    /*
    Serial.print("leg_IK => ");
    Serial.print("Q1:: ");Serial.print(theta_leg[0]);Serial.print(", ");
    Serial.print("Q2:: ");Serial.print(theta_leg[1]);Serial.print(", ");
    Serial.print("Q3:: ");Serial.print(theta_leg[2]);
    Serial.println(" ");
    */
}





void hand_IK(double px, double py, double pz){
    float s2 = pz / a2;
    float c2 = sqrt(abs(1 - (s2 * s2)));
    double t2 = atan2(s2, c2) / 2 / pi * 360 ;
    if(t2 > range){
        theta_hand[1] = 180 - t2;
    }else{
        theta_hand[1] = t2 ; 
    }


    float aa = a1 + ( a2 *  sin(deg(theta_hand[1])) );
    float c1 = px / aa;
    float s1 = py / aa;
    double t1 = atan2(s1, c1) / 2 / pi * 360 ;
    if(t1 > range){
        theta_hand[0] = 180 - t1;
    }else{
        theta_hand[0] = t1 ; 
    }

    /*
    Serial.print("hand_IK => ");
    Serial.print("Q1:: ");Serial.print(theta_hand[0]);Serial.print(", ");
    Serial.print("Q2:: ");Serial.print(theta_hand[1]);
    Serial.println(" ");
    */
}




//////////////////////////// UPDATE TETHA TO PWM/////////////////////////////////////////////

void looking_for_tetha(int i){
    
    if(i==1){//hand
        if(ref_right_hand_step[0] == 1){
            tetha_right_hand[0] = ( ref_right_hand[0] - (theta_hand[0] * (ref_right_hand[0]-ref_right_hand_min[0]) / range) );
        }else
        if(ref_right_hand_step[2] == 1){
            tetha_right_hand[0] = ( ref_right_hand[0] + (theta_hand[0] * (ref_right_hand_max[0]-ref_right_hand[0]) / range) );
        }else
        if(ref_right_hand_step[1] == 1){
            tetha_right_hand[0] = ( ref_right_hand[0] );
        }

        if(ref_right_hand_step[3] == 1){
            tetha_right_hand[1] = ( ref_right_hand[1] - (theta_hand[1] * (ref_right_hand[1]-ref_right_hand_min[1]) / range) );
        }else
        if(ref_right_hand_step[5] == 1){
            tetha_right_hand[1] = ( ref_right_hand[1] + (theta_hand[1] * (ref_right_hand_max[1]-ref_right_hand[1]) / range) );
        }else
        if(ref_right_hand_step[4] == 1){
            tetha_right_hand[1] = ( ref_right_hand[1] );
        }
    }

    if(i==2){//hand
        if(ref_left_hand_step[0] == 1){
            tetha_left_hand[0] = ( ref_left_hand[0] - (theta_hand[0] * (ref_left_hand[0]-ref_left_hand_min[0]) / range) );
        }else
        if(ref_left_hand_step[2] == 1){
            tetha_left_hand[0] = ( ref_left_hand[0] + (theta_hand[0] * (ref_left_hand_max[0]-ref_left_hand[0]) / range) );
        }else
        if(ref_left_hand_step[1] == 1){
            tetha_left_hand[0] = ( ref_left_hand[0] );
        }

        if(ref_left_hand_step[3] == 1){
            tetha_left_hand[1] = ( ref_left_hand[1] - (theta_hand[1] * (ref_left_hand[1]-ref_left_hand_min[1]) / range) );
        }else
        if(ref_left_hand_step[5] == 1){
            tetha_left_hand[1] = ( ref_left_hand[1] + (theta_hand[1] * (ref_left_hand_max[1]-ref_left_hand[1]) / range) );
        }else
        if(ref_left_hand_step[4] == 1){
            tetha_left_hand[1] = ( ref_left_hand[1] );
        }
    }
    

    if(i==3){//leg        
        if(ref_right_leg_step[0] == 1){
            tetha_right_leg[0] = ( ref_right_leg[0] - (theta_leg[0] * (ref_right_leg[0]-ref_right_leg_min[0]) / range) );
        }else
        if(ref_right_leg_step[2] == 1){
            tetha_right_leg[0] = ( ref_right_leg[0] + (theta_leg[0] * (ref_right_leg_max[0]-ref_right_leg[0]) / range) );
        }else
        if(ref_right_leg_step[1] == 1){
            tetha_right_leg[0] = ( ref_right_leg[0] );
        }

        if(ref_right_leg_step[3] == 1){
            tetha_right_leg[1] = ( ref_right_leg[1] - (theta_leg[1] * (ref_right_leg[1]-ref_right_leg_min[1]) / range) );
        }else
        if(ref_right_leg_step[5] == 1){
            tetha_right_leg[1] = ( ref_right_leg[1] + (theta_leg[1] * (ref_right_leg_max[1]-ref_right_leg[1]) / range) );
        }else
        if(ref_right_leg_step[4] == 1){
            tetha_right_leg[1] = ( ref_right_leg[1] );
        }

        if(ref_right_leg_step[6] == 1){
            tetha_right_leg[2] = ( ref_right_leg[2] - (theta_leg[2] * (ref_right_leg[2]-ref_right_leg_min[2]) / range) );
        }else
        if(ref_right_leg_step[8] == 1){
            tetha_right_leg[2] = ( ref_right_leg[2] + (theta_leg[2] * (ref_right_leg_max[2]-ref_right_leg[2]) / range) );
        }else
        if(ref_right_leg_step[7] == 1){
            tetha_right_leg[2] = ( ref_right_leg[2] );
        }
    }

    if(i==4){//leg
        if(ref_left_leg_step[0] == 1){
            tetha_left_leg[0] = ( ref_left_leg[0] - (theta_leg[0] * (ref_left_leg[0]-ref_left_leg_min[0]) / range) );
        }else
        if(ref_left_leg_step[2] == 1){
            tetha_left_leg[0] = ( ref_left_leg[0] + (theta_leg[0] * (ref_left_leg_max[0]-ref_left_leg[0]) / range) );
        }else
        if(ref_left_leg_step[1] == 1){
            tetha_left_leg[0] = ( ref_left_leg[0] );
        }

        if(ref_left_leg_step[3] == 1){
            tetha_left_leg[1] = ( ref_left_leg[1] - (theta_leg[1] * (ref_left_leg[1]-ref_left_leg_min[1]) / range) );
        }else
        if(ref_left_leg_step[5] == 1){
            tetha_left_leg[1] = ( ref_left_leg[1] + (theta_leg[1] * (ref_left_leg_max[1]-ref_left_leg[1]) / range) );
        }else
        if(ref_left_leg_step[4] == 1){
            tetha_left_leg[1] = ( ref_left_leg[1] );
        }

        if(ref_left_leg_step[6] == 1){
            tetha_left_leg[2] = ( ref_left_leg[2] - (theta_leg[2] * (ref_left_leg[2]-ref_left_leg_min[2]) / range) );
        }else
        if(ref_left_leg_step[8] == 1){
            tetha_left_leg[2] = ( ref_left_leg[2] + (theta_leg[2] * (ref_left_leg_max[2]-ref_left_leg[2]) / range) );
        }else
        if(ref_left_leg_step[7] == 1){
            tetha_left_leg[2] = ( ref_left_leg[2] );
        }
    }
}


////////////////////////////////// UPDATE SERVOS POSITIONING////////////////////////////////////////////
void upload_servos__(){
    servo.setPWM(lh_j1, 0, tetha_left_hand[0]);
    servo.setPWM(lh_j2, 0, tetha_left_hand[1]);

    servo.setPWM(ll_j1, 0, tetha_left_leg[0]);
    servo.setPWM(ll_j2, 0, tetha_left_leg[1]);
    servo.setPWM(ll_j3, 0, tetha_left_leg[2]);

    servo.setPWM(rh_j1, 0, tetha_right_hand[0]);
    servo.setPWM(rh_j2, 0, tetha_right_hand[1]);
    
    servo.setPWM(rl_j1, 0, tetha_right_leg[0]);
    servo.setPWM(rl_j2, 0, tetha_right_leg[1]);
    servo.setPWM(rl_j3, 0, tetha_right_leg[2]);
}
