#include <string>

using namespace std;

class Song
{
private:
string title;
unsigned short year;

public:
  void setTitle(string newtitle);
  int setYear(unsigned short newyear);
  string getTitle();
  unsigned short getYear();
}; // Song{}

