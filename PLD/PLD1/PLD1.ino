#include <EEPROM.h>

int stop_value = 1; //to stop sequence
boolean swap_state = false;
boolean swap_stop  = false;
int MoveSpeed=8; //step in Microseconds
int SwapSpeed_fast = 5;
int SwapSpeed_low = 32; //speed in slow stepping
int error_step = 75;
int error_speed = 4;
int roller_value;
int roller_speed=20;
unsigned int  X= 4000;
String     inputString = ""; // a string to hold incoming data
boolean stringComplete = true;     // whether the string is completet

int arr[] = {X,MoveSpeed,error_step,SwapSpeed_fast,SwapSpeed_low};


# define X_DIR_5v 12  //DIR+(+5v) axis stepper motor direction control  Brown
# define X_STP_5v 11  //PUL+(+5v) axis stepper motor step control       RED
# define X_ENB_5v 10
# define buttonPin 9
# define roller A1


void setup() {// *************************************************************     setup
for(int i=0;i <sizeof(arr)/sizeof(arr[0]);i++)
{
  byte hiByte = EEPROM.read(i*2);
  byte loByte = EEPROM.read(i*2+1);
  arr[i] = int(word(hiByte,loByte));
}
X               = arr[0];
MoveSpeed       = arr[1];
error_step      = arr[2];
SwapSpeed_fast  = arr[3];
SwapSpeed_low   = arr[4];
//pinMode (X_EN_5v ,OUTPUT); //ENA+(+5V)
pinMode (X_DIR_5v,OUTPUT); //DIR+(+5v)
pinMode (X_STP_5v,OUTPUT); //PUL+(+5v)
pinMode (X_ENB_5v,OUTPUT); //PUL+(+5v)
pinMode (buttonPin,INPUT );
pinMode (roller  ,INPUT );
//digitalWrite (X_EN_5v , LOW); //ENA+(+5V) low=enabled
digitalWrite (X_DIR_5v, LOW); //DIR+(+5v)
digitalWrite (X_STP_5v, LOW); //PUL+(+5v)

Serial.begin(9600);
}
void State()
{
  Serial.print("state=");
  Serial.print(X);
  Serial.print(',');
  Serial.print(MoveSpeed);
  Serial.print(',');
  Serial.print(SwapSpeed_low);
  Serial.print(',');
  Serial.print(SwapSpeed_fast);
  Serial.print(',');
  Serial.print(error_step);
  Serial.print(',');
  Serial.println(stop_value);
  inputString="";
}
void MoveX(float mm ,float ps,float rs){ // **************************************************************    Move
int xt;
float a;
if (mm == 0){
  inputString.setCharAt(0,' ');
  mm=inputString.toInt();
  Serial.print("move=");
  Serial.println(mm);
  inputString="";
}
if (mm>0)
{digitalWrite (X_DIR_5v,HIGH);xt=1;}
else
{digitalWrite (X_DIR_5v,LOW);xt=-1;}
for (int i=0; i<abs(mm); i++)
{    
     if(digitalRead(buttonPin) == LOW ){potential();break;}
     if(stop_value > 0){break;}
     if(X == 52000){X = 4000;}
     while (Serial.available()) 
        {
          char inChar = (char)Serial.read();   
          if (inChar > 0)     {inputString += inChar;}  // add it to the inputString:
          if (inChar == '\n') { stringComplete = true;} // if the incoming character is a newline, set a flag so the main loop can do something about it: 
        }
      if (stringComplete) 
       {  
        if (inputString=="nstate\n"){State();  }
        if (inputString=="Stop\n")  {stop_value = true; swap_stop =true;Serial.println("stoping.."); break;  }
        if (inputString=="Sstop\n") {swap_stop=true;Serial.println("swap stoping..");inputString = "";}
        if (inputString !="") {Serial.print("BAD COMMAND="+inputString);}// Serial.print("\n"); }// "\t" tab      
        inputString = ""; stringComplete = false; // clear the string:
       }
     a = ps + i*(rs-ps)/abs(mm);
     digitalWrite (X_STP_5v, HIGH);
     delay(int(a));
     digitalWrite (X_STP_5v, LOW);
     delay(int(a));
     X = X + xt;
}
inputString="";
}

void Swap()
{
  inputString.setCharAt(0,' ');
  int mm=inputString.toInt();
  Serial.print("swap=");
  Serial.println(mm);
  inputString="";
  MoveX(mm,SwapSpeed_fast,SwapSpeed_low);
  while(true){
    MoveX(-error_step,error_speed,error_speed);
    MoveX(-mm,SwapSpeed_low,SwapSpeed_fast);
    MoveX(-mm,MoveSpeed,SwapSpeed_low);
    if(swap_stop == true){
      MoveX(mm+error_step,MoveSpeed,MoveSpeed);
      swap_stop = false;
      swap_state = false;
      break;
    }
    MoveX(error_step,error_speed,error_speed);
    MoveX(mm,SwapSpeed_low,SwapSpeed_fast);
    MoveX(mm,SwapSpeed_fast,SwapSpeed_low);
  }
}
void SetX()
{
  inputString.setCharAt(0,' ');
  X = inputString.toInt();
  Serial.print("X=");
  Serial.println(X);
  inputString="";
}
void MSpeed(){  // ************************************************************   MoveSpeed
 inputString.setCharAt(0,' ');
 MoveSpeed=inputString.toInt();
 if(MoveSpeed > 0)
 {
   Serial.print("Speed=");
   Serial.println(MoveSpeed);
 }else{
   Serial.println("speed value errror!!");
 }
 inputString="";
}
void ErrorStep()
{
  inputString.setCharAt(0,' ');
  int mm=inputString.toInt();
  error_step = mm;
  Serial.print("ErrorStep=");
  Serial.println(error_step);
  inputString="";
}
void ErrorSpeed()
{
  inputString.setCharAt(0,' ');
  int mm=inputString.toInt();
  error_speed = mm;
  Serial.print("ErrorSpeed=");
  Serial.println(error_speed);
  inputString="";
}
void Save(boolean mode)
{
  if (mode == true){
    Serial.println("Saved...");
  }
  int arr[] = {X,MoveSpeed,error_step,SwapSpeed_fast,SwapSpeed_low};
  for(int i=0;i <sizeof(arr)/sizeof(arr[0]);i++)
  {
    byte hiByte = highByte(arr[i]);
    byte loByte = lowByte(arr[i]);
    EEPROM.write(i*2,hiByte);
    EEPROM.write(i*2+1,loByte);
  }
  inputString="";
}
void Ratio_l(){
  inputString.setCharAt(0,' ');
  int a = inputString.toInt();
  SwapSpeed_low = a;
  Serial.print("Low=");
  Serial.println(SwapSpeed_low);
  inputString="";
}
void Ratio_f(){
  inputString.setCharAt(0,' ');
  int a = inputString.toInt();
  SwapSpeed_fast = a;
  Serial.print("Fast=");
  Serial.println(SwapSpeed_fast);
  inputString="";
}
void potential(){
  stop_value = 2;
  float a;
  int s;
  int xt;
  while(true){
    roller_value = analogRead(A0);
    if(digitalRead(buttonPin) == HIGH){break;}
    if(X == 52000){X = 4000;}
    while (Serial.available()) 
    {
      char inChar = (char)Serial.read();   
      if (inChar > 0)     {inputString += inChar;}  // add it to the inputString:
      if (inChar == '\n') { stringComplete = true;} // if the incoming character is a newline, set a flag so the main loop can do something about it: 
    }
    if (stringComplete) 
    {  
    if (inputString=="nstate\n"){State();  }
    if (inputString !="") {Serial.print("BAD COMMAND="+inputString);}    
    inputString = ""; stringComplete = false; // clear the string:
    }
    a = (float)roller_value/1024-0.5;
    s = roller_speed*(1-0.8*abs(a));
    if(a<-0.05) {digitalWrite (X_DIR_5v,LOW );xt=-1;}else{
    if(a>0.05 ) {digitalWrite (X_DIR_5v,HIGH);xt= 1;}else{continue;}}
    digitalWrite (X_STP_5v, HIGH);
    delay(s);
    digitalWrite (X_STP_5v, LOW );
    delay(s);
    X = X + xt;
  }
  stop_value = 1;
}
void loop()  // **************************************************************     loop
{
  if(digitalRead(buttonPin) == LOW){potential();}
  while (Serial.available()) 
  {
    char inChar = (char)Serial.read();   
    if (inChar > 0)     {inputString += inChar;}  // add it to the inputString:
    if (inChar == '\n') { stringComplete = true;} // if the incoming character is a newline, set a flag so the main loop can do something about it: 
  }
  
 if (stringComplete) 
 {  
  if (inputString.charAt(0)=='s') {MSpeed(); }
  if (inputString.charAt(0)=='k') {Ratio_l(); }
  if (inputString.charAt(0)=='l') {Ratio_f(); }
  if (inputString.charAt(0)=='e') {ErrorStep(); }
  if (inputString.charAt(0)=='p') {ErrorSpeed(); } 
  if (inputString.charAt(0)=='x') {SetX(); }
  if (inputString.charAt(0)=='m') {stop_value = false; MoveX(0,MoveSpeed,MoveSpeed);stop_value = true;Save(false); } 
  if (inputString.charAt(0)=='r') {stop_value = false; swap_stop  = false; Swap();stop_value = true;Save(false);   }
  if (inputString=="Save\n")      {Save(true);}
  if (inputString=="nstate\n")    {State();  }
  if (inputString !="") {Serial.print("BAD COMMAND="+inputString);}// Serial.print("\n"); }// "\t" tab      
  inputString = ""; stringComplete = false; // clear the string:
 }
}
