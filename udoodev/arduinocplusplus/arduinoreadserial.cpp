

#include<iostream>
#include<fstream>
#include<cstdlib>
using namespace std;

int main()
{

  system("stty -F /dev/ttyACM0 cs8 9600 ignbrk -brkint -icrnl -imaxbel
-opost -onlcr -isig -icanon -iexten -echo -echoe -echok -echoctl -echoke
noflsh -ixon -crtscts");	//Activates the tty connection with the Arduino
  ifstream Arduino_Input("/dev/ttyACM0");
//Opens the tty connection as an ifstream
  ofstream Arduino_Output("/dev/ttyACM0");
//Opens the tty connection as an ofstream, not used in this example
  double Voltage;	//The Arduino is reading the voltage from A0
  long int Time = time(NULL);
  int i;

  while(time(NULL)-Time < 5){}	//Wait five seconds for the Arduino
to start up
  for(i = 0; i < 100;)
  {
    Time = time(NULL);
    while(time(NULL)-Time < 1){}	//wait one second to get
good numbers into the Arduino stream
    while(!Arduino_Input.eof())	//while the eof flage isn't set
    {
      Arduino_Input >> Voltage;	//will set the
error flag if not ready, will get a number from the Arduino stream if ready
      cout << Voltage << endl;	//Output it to the cout
stream
      i++;	//Increament i, it is not known how many numbers
I'll get at a time
    }
    Arduino_Input.clear();	//eof flag won't clear itself
  }

  Arduino_Input.close();	//Close the ifstream
  return(0);

} 
