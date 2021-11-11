#include <ctime>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <raspicam/raspicam.h>
using namespace std;

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

int main ( int argc,char **argv ) {
  time_t timer_begin,timer_end;
  raspicam::RaspiCam Camera; //Cmaera object
  //Open camera 
  cout<<"Opening Camera..."<<endl;
  Camera.setFormat(raspicam::RASPICAM_FORMAT_RGB);
  /**Sets camera width. Use a multiple of 320 (640, 1280)
  */
  Camera.setWidth (320) ;
  /**Sets camera Height. Use a multiple of 240 (480, 960)
  */
  Camera.setHeight (240);
  if ( !Camera.open()) {cerr<<"Error opening camera"<<endl;return -1;}
  //wait a while until camera stabilizes
  cout<<"Sleeping for 1 secs"<<endl;
  sleep(1);
  time ( &timer_begin );
  //capture
  Camera.grab();
  //allocate memory
  unsigned char *data=new unsigned char[  Camera.getImageTypeSize ( raspicam::RASPICAM_FORMAT_RGB )];
  //extract the image in rgb format
  Camera.retrieve ( data );//get camera image
  //save
  time_t rawtime;
  struct tm * timeinfo;
  char buffer[80];
  time (&rawtime);
  timeinfo = localtime(&rawtime);

  strftime(buffer,sizeof(buffer),"image_%d-%m-%Y_%H.%M.%S.ppm",timeinfo);
  std::string str(buffer);
  writePPM(str, Camera.getWidth(), Camera.getHeight(), data);

  // Hier kommt das Clustering...
  int n;
  cout<<"Start clustering..."<<endl;
  cout<<"Bild verkleinern um Faktor ?"<<endl;
  cin >> n;
  int wid =  Camera.getWidth() / n;
  int hei = Camera.getHeight() / n;
  unsigned char *clustered=new unsigned char[ wid * hei * 3 ];
  cout<<"Starte von "<<sizeof(data)<<endl;
  cout<<"Clusterwerte "<<wid<<" "<<hei<<" "<<sizeof(clustered)<<endl;
  clustered = cluster(data, Camera.getWidth(), Camera.getHeight(), wid, hei);
  strftime(buffer,sizeof(buffer),"image_c_%d-%m-%Y_%H.%M.%S.ppm",timeinfo);
  std::string sat(buffer);
  writePPM(sat, wid, hei, clustered);
  cout<<"Cluster saved at "<<sat<<endl;
  // Ende des Clusters
	
  time ( &timer_end ); /* get current time; same as: timer = time(NULL)  */
  double secondsElapsed = difftime ( timer_end,timer_begin );
  cout<< secondsElapsed<<" seconds"<<endl;
  //free resrources    
  delete data;
  delete clustered;
  Camera.release();
  return 0;
}
