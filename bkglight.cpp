#include <ctime>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <algorithm> //Fuer Minimumberechnung
#include <raspicam/raspicam.h>
#include <wiringPi.h>
#include <softPwm.h>
using namespace std;

string disclaimer = "Background-Pi by DrHangs, proceed with caution!";
string usage = "Usage of this Programm:\n"
" -h  shows this help\n"
" -c <file>  let's you change the config-files name, standard is 'config'\n"
" -t  enters programm in testmode, with more output and so on...\n"
" -p  just takes one picture and exits\n";

// Einstellungsmöglichkeiten
struct configuration {
  int left=-1;
  int right=-1;
  int top=-1;
  int bottom=-1;
  int pin_red=-1;
  int pin_green=-1;
  int pin_blue=-1;
  int pin_conf=-1;
  int ms_wait=-1;
  int smoothness=-1;
  int percent_red=-1;
  int percent_green=-1;
  int percent_blue=-1;
  int percent_brightness=-1;
  int base_brightness=-1;
} config;

void readConfig(string filename){
  ifstream myfile;
  myfile.open(filename);
  if(!myfile) {
    cout<<"Configfile doesn't exist, creating one with standard..."<<endl;
    ofstream outfile;
    outfile.open("config");
    outfile<<"#configfile für backgroundpi, standardconfig\nleft=0\nright=0\ntop=0\nbottom=0\npin-red=0\npin-green=0\npin-blue=0\npin-conf=0\nms-wait=10"<<endl;
    outfile.close();
    myfile.open("config");
  }
  string myline;
  if ( myfile.is_open() ) {
    while ( myfile ) { // equivalent to myfile.good()
      getline (myfile, myline);
      if(myline.length() > 0) {
        if(myline[0] != '#') {
          istringstream is_line(myline);
          string key;
          if( std::getline(is_line, key, '=') )
          {
            std::string value;
            if( std::getline(is_line, value) ) {
              if(key == "left")
                  config.left = stoi(value);
              else if(key == "right")
                  config.right = stoi(value);
              else if(key == "top")
                  config.top = stoi(value);
              else if(key == "bottom")
                  config.bottom = stoi(value);
              else if(key == "pin-red")
                  config.pin_red = stoi(value);
              else if(key == "pin-green")
                  config.pin_green = stoi(value);
              else if(key == "pin-blue")
                  config.pin_blue = stoi(value);
              else if(key == "pin-conf")
                  config.pin_conf = stoi(value);
              else if(key == "ms-wait")
                  config.ms_wait = stoi(value);
              else if(key == "smoothness")
                  config.smoothness = stoi(value);
              else if(key == "percent_red")
                  config.percent_red = stoi(value);
              else if(key == "percent_green")
                  config.percent_green = stoi(value);
              else if(key == "percent_blue")
                  config.percent_blue = stoi(value);
              else if(key == "percent_brightness")
                  config.percent_brightness = stoi(value);
              else if(key == "base_brightness")
                  config.base_brightness = stoi(value);
              else
                cout << "Nicht zugeordnet: " << key << endl;
            }
          }
        }
      }
    }
  }
  else {
    cout << "Couldn't open file\n";
  }
  myfile.close();
}

bool validateConfig(){
  if(config.left == -1 ||
     config.right == -1 ||
     config.top == -1 ||
     config.bottom == -1 ||
     config.pin_red == -1 ||
     config.pin_green == -1 ||
     config.pin_blue == -1 ||
     config.pin_conf == -1) return false;
  return true;
}

//Funktion für Clustering:

//höhe und breite angeben,
//teilen und entsprechende Anzahl zusammenfassen
struct rgbwert {
  int red;
  int green;
  int blue;
};

rgbwert kompresse(unsigned char *data, int fromwidth, int fromheight, int rowfrom, int rowto, int colfrom, int colto){
  float r = 0.0, g = 0.0, b = 0.0;
  int n = 0;
  for ( int z=rowfrom; z<rowto; z++ ){
    for ( int s=colfrom; s<colto; s++ ){
      int first = (z * fromwidth + s) * 3;
      r += data[first];
      g += data[first + 1];
      b += data[first + 2];
      n++;
    }
  }
  rgbwert result;
  result.red = r / n;
  result.green = g / n;
  result.blue = b / n;
  return result;
}

unsigned char* cluster(unsigned char *data, int fromwidth, int fromheight, int width, int height){
  int densewid = fromwidth / width;
  int densehei = fromheight / height;
  unsigned char *clustered=new unsigned char[ width * height * 3 ];

  for ( int z=0; z<height; z++ ){
    for ( int s=0; s<width; s++ ){
      int first = (z * width + s) * 3; // Erster Wert in Cluster-Wertereihe
      int raw1 = (z * fromwidth * densehei + s * densewid) * 3;
      rgbwert rgb = kompresse(data, fromwidth, fromheight, z*densehei, (z+1)*densehei, s*densewid, (s+1)*densewid);
      clustered[first] = rgb.red;
      clustered[first+1] = rgb.green;
      clustered[first+2] = rgb.blue;
    }
  }
  return clustered;
}



void writePPM(string name, int width, int height, unsigned char *data){
  std::ofstream outFile ( name,std::ios::binary );
  outFile<<"P6\n"<<width <<" "<<height <<" 255\n";
  outFile.write ( ( char* ) data, (width * height * 3) );
  cout<<"Image saved at "<<name<<endl;
}

bool setupCamera(raspicam::RaspiCam Camera){
  //Open camera
  cout<<"Opening Camera..."<<endl;
  Camera.setFormat(raspicam::RASPICAM_FORMAT_RGB);
  //Sets camera width. Use a multiple of 320 (640, 1280)
  Camera.setWidth (320) ;
  /**Sets camera Height. Use a multiple of 240 (480, 960)
  */
  Camera.setHeight (240);
  if ( !Camera.open()) {cerr<<"Error opening camera"<<endl;return false;}
  return true;
}

void savePic(raspicam::RaspiCam Camera, unsigned char *data){
  Camera.grab();
  //extract the image in rgb format
  Camera.retrieve ( data );//get camera image
  //save
  time_t rawtime;
  struct tm * timeinfo;
  char buffer[80];
  time (&rawtime);
  timeinfo = localtime(&rawtime);

  strftime(buffer,sizeof(buffer),"image_%d-%m-%Y_%H.%M.%S.ppm",timeinfo);
  string str(buffer);
  writePPM(str, Camera.getWidth(), Camera.getHeight(), data);
}

// -------------------------------------------------------
// Alles mit Pins...
bool btnpressed = false;

void switchInterrupt(void){
  btnpressed = true;
}

void setupPins(){
  wiringPiSetup();
  pinMode(config.pin_conf, INPUT);
  pinMode(config.pin_red, PWM_OUTPUT);
  pinMode(config.pin_green, PWM_OUTPUT);
  pinMode(config.pin_blue, PWM_OUTPUT);

  pullUpDnControl(config.pin_conf, PUD_UP);
  //wiringPiISR(config.pin_conf, INT_EDGE_FALLING, &switchInterrupt);

  softPwmCreate(config.pin_red, 0, 100);
  softPwmCreate(config.pin_green, 0, 100);
  softPwmCreate(config.pin_blue, 0, 100);
}

// --------------------------------------------------------
bool isTest = false;
bool isPic = false;

void mainloop(raspicam::RaspiCam Camera, unsigned char *data) {
  cout<<"Start Gay-Mode with button-press..."<<endl;
  //while(!btnpressed) delay(100);
  while(digitalRead(config.pin_conf) == 1) delay(100);
  while(digitalRead(config.pin_conf) == 0) delay(10);
  btnpressed = false; // Reset as reaction complete
  cout<<"Spreading Chem-Trails, interrupt with button-press..."<<endl;
  //while(!btnpressed){
  while(digitalRead(config.pin_conf) == 1){
    Camera.grab();
    Camera.retrieve(data);
    rgbwert color = kompresse(data, Camera.getWidth(), Camera.getHeight(), config.top, config.bottom, config.left, config.right);
    int red = color.red/2.55;
    int green = color.green/2.55;
    int blue = color.blue/2.55;
    if(isTest) cout<<"Werte: "<<red<<":"<<green<<":"<<blue<<endl;
    softPwmWrite(config.pin_red, red);
    softPwmWrite(config.pin_green, green);
    softPwmWrite(config.pin_blue, blue);
    delay(config.ms_wait);
  }
  while(digitalRead(config.pin_conf) == 0) delay(10);
  btnpressed = false; // Reset as reaction complete
}

int main ( int argc,char *argv[] ) {
  cout<<disclaimer<<endl;
  time_t timer_begin,timer_end;
  string filename = "config";
  for(int a=0; a<argc; a++){
    string value = argv[a];
    if(value == "-h") {
      cout<<usage<<endl;
      return 0;
    } else if(value == "-t"){
      isTest = true;
    } else if(value == "-p"){
      isPic = true;
    } else if(value == "-c"){
      cout<<"Use configfile: "<<argv[a+1]<<endl;
      filename = argv[a+1];
      a++;
    }
  }
  // Config and Pin-Handling
  readConfig(filename);
  if(!validateConfig()){cout<<"No valid config, stopping program!"<<endl;return -1;}
  setupPins();

  raspicam::RaspiCam Camera; //Camera object
  //if(!setupCamera(Camera)) return -1;
  //Open camera
  cout<<"Opening Camera..."<<endl;
  Camera.setFormat(raspicam::RASPICAM_FORMAT_RGB);
  //Sets camera width. Use a multiple of 320 (640, 1280)
  Camera.setWidth (320) ;
  /**Sets camera Height. Use a multiple of 240 (480, 960)
  */
  Camera.setHeight (240);
  if ( !Camera.open()) {cerr<<"Error opening camera"<<endl;return false;}
  //wait a while until camera stabilizes
  cout<<"Sleeping for 2 secs"<<endl;
  sleep(2);
  time ( &timer_begin );
  //allocate memory
  unsigned char *data=new unsigned char[  Camera.getImageTypeSize ( raspicam::RASPICAM_FORMAT_RGB )];
  Camera.grab();
  Camera.retrieve(data);

  if(isPic) {
    savePic(Camera, data);
  } else {
   // mainloop(Camera, data);
  //cout<<"Start Gay-Mode with button-press..."<<endl;
  //while(digitalRead(config.pin_conf) == 1) delay(100);
  //while(digitalRead(config.pin_conf) == 0) delay(10); 
  //cout<<"Spreading Chem-Trails, interrupt with button-press..."<<endl;
  rgbwert vorher; // Speichert den vorherigen Wert (in Prozent statt byte!)
  rgbwert color;
  vorher.red = 0;
  vorher.green = 0;
  vorher.blue = 0;
  int red;
  int green;
  int blue;
  while(digitalRead(config.pin_conf) == 1){
    Camera.grab();
    Camera.retrieve(data);
    color = kompresse(data, Camera.getWidth(), Camera.getHeight(), config.top, config.bottom, config.left, config.right);
	
    red = (color.red/2.55 + vorher.red * config.smoothness)/(config.smoothness+1);
    green = (color.green/2.55 + vorher.green * config.smoothness)/(config.smoothness+1);
    blue = (color.blue/2.55 + vorher.blue * config.smoothness)/(config.smoothness+1);
	
	vorher.red = red;
    vorher.green = green;
    vorher.blue = blue;
	
	//Farbkorrektur
	red = min(0, max(100, red * config.percent_red/100 * config.percent_brightness/100 + config.base_brightness));
	green = min(0, max(100, green * config.percent_green/100 * config.percent_brightness/100 + config.base_brightness));
	blue = min(0, max(100, blue * config.percent_blue/100 * config.percent_brightness/100 + config.base_brightness));
	
    if(isTest) cout<<"Werte: "<<red<<":"<<green<<":"<<blue<<"--"<<color.red<<":"<<color.green<<":"<<color.blue<<endl;
    softPwmWrite(config.pin_red, red);
    softPwmWrite(config.pin_green, green);
    softPwmWrite(config.pin_blue, blue);
	
    delay(config.ms_wait);
  }
  softPwmWrite(config.pin_red, 0);
  softPwmWrite(config.pin_green, 0);
  softPwmWrite(config.pin_blue, 0);
  while(digitalRead(config.pin_conf) == 0) delay(10);
  }

  time ( &timer_end ); /* get current time; same as: timer = time(NULL)  */
  double secondsElapsed = difftime ( timer_end,timer_begin );
  cout<< secondsElapsed<<" seconds"<<endl;
  //free resrources
  delete data;
  Camera.release();
  return 0;
}
