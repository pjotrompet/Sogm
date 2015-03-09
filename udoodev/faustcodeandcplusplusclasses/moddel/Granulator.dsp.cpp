//-----------------------------------------------------
// name: "Granulator"
// author: "Mayank Sanganeria"
// version: "1.0"
//
// Code generated with Faust 0.9.71 (http://faust.grame.fr)
//-----------------------------------------------------
/* link with  */
#include <math.h>
/************************************************************************

	IMPORTANT NOTE : this file contains two clearly delimited sections : 
	the ARCHITECTURE section (in two parts) and the USER section. Each section 
	is governed by its own copyright and license. Please check individually 
	each section for license and copyright information.
*************************************************************************/

/*******************BEGIN ARCHITECTURE SECTION (part 1/2)****************/

/************************************************************************
    FAUST Architecture File
	Copyright (C) 2003-2011 GRAME, Centre National de Creation Musicale
    ---------------------------------------------------------------------
    This Architecture section is free software; you can redistribute it 
    and/or modify it under the terms of the GNU General Public License 
	as published by the Free Software Foundation; either version 3 of 
	the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License 
	along with this program; If not, see <http://www.gnu.org/licenses/>.

	EXCEPTION : As a special exception, you may create a larger work 
	that contains this FAUST architecture section and distribute  
	that work under terms of your choice, so long as this FAUST 
	architecture section is not modified. 


 ************************************************************************
 ************************************************************************/

#include <libgen.h>
#include <stdlib.h>
#include <iostream>
#include <list>

#ifndef FAUST_FUI_H
#define FAUST_FUI_H

#ifndef FAUST_UI_H
#define FAUST_UI_H

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif

/*******************************************************************************
 * UI : Faust User Interface
 * This abstract class contains only the method that the faust compiler can
 * generate to describe a DSP interface.
 ******************************************************************************/

class UI
{

 public:

	UI() {}

	virtual ~UI() {}

    // -- widget's layouts

    virtual void openTabBox(const char* label) = 0;
    virtual void openHorizontalBox(const char* label) = 0;
    virtual void openVerticalBox(const char* label) = 0;
    virtual void closeBox() = 0;

    // -- active widgets

    virtual void addButton(const char* label, FAUSTFLOAT* zone) = 0;
    virtual void addCheckButton(const char* label, FAUSTFLOAT* zone) = 0;
    virtual void addVerticalSlider(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT init, FAUSTFLOAT min, FAUSTFLOAT max, FAUSTFLOAT step) = 0;
    virtual void addHorizontalSlider(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT init, FAUSTFLOAT min, FAUSTFLOAT max, FAUSTFLOAT step) = 0;
    virtual void addNumEntry(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT init, FAUSTFLOAT min, FAUSTFLOAT max, FAUSTFLOAT step) = 0;

    // -- passive widgets

    virtual void addHorizontalBargraph(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT min, FAUSTFLOAT max) = 0;
    virtual void addVerticalBargraph(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT min, FAUSTFLOAT max) = 0;

	// -- metadata declarations

    virtual void declare(FAUSTFLOAT*, const char*, const char*) {}
};

#endif

#include <string>
#include <map>
#include <set>
#include <vector>
#include <stack>

#include <iostream>
#include <fstream>

//using namespace std;

#if 1

/*******************************************************************************
 * FUI : used to save and recall the state of the user interface
 * This class provides essentially two new methods saveState() and recallState()
 * used to save on file and recall from file the state of the user interface.
 * The file is human readble and editable
 ******************************************************************************/

class FUI : public UI
{
    
    std::stack<std::string>             fGroupStack;
	std::vector<std::string>            fNameList;
	std::map<std::string, FAUSTFLOAT*>	fName2Zone;

 protected:

 	// labels are normalized by replacing white spaces by underscores and by
 	// removing parenthesis
	std::string normalizeLabel(const char* label)
	{
		std::string 	s;
		char 	c;

		while ((c=*label++)) {
			if (isspace(c)) 				{ s += '_'; }
			//else if ((c == '(') | (c == ')') ) 	{ }
			else 							{ s += c; }
		}
		return s;
	}

	// add an element by relating its full name and memory zone
	virtual void addElement(const char* label, FAUSTFLOAT* zone)
	{
		std::string fullname (fGroupStack.top() + '/' + normalizeLabel(label));
		fNameList.push_back(fullname);
		fName2Zone[fullname] = zone;
	}

	// keep track of full group names in a stack
	virtual void pushGroupLabel(const char* label)
	{
		if (fGroupStack.empty()) {
			fGroupStack.push(normalizeLabel(label));
		} else {
			fGroupStack.push(fGroupStack.top() + '/' + normalizeLabel(label));
		}
	}

	virtual void popGroupLabel()
	{
		fGroupStack.pop();
	};

 public:

	FUI() 			{}
	virtual ~FUI() 	{}

	// -- Save and recall methods

	// save the zones values and full names
	virtual void saveState(const char* filename)
	{
		std::ofstream f(filename);

		for (unsigned int i=0; i<fNameList.size(); i++) {
			std::string	n = fNameList[i];
			FAUSTFLOAT*	z = fName2Zone[n];
			f << *z << ' ' << n.c_str() << std::endl;
		}

		f << std::endl;
		f.close();
	}

	// recall the zones values and full names
	virtual void recallState(const char* filename)
	{
		std::ifstream f(filename);
		FAUSTFLOAT	v;
		std::string	n;

		while (f.good()) {
			f >> v >> n;
			if (fName2Zone.count(n)>0) {
				*(fName2Zone[n]) = v;
			} else {
				std::cerr << "recallState : parameter not found : " << n.c_str() << " with value : " << v << std::endl;
			}
		}
		f.close();
	}


    // -- widget's layouts (just keep track of group labels)

    virtual void openTabBox(const char* label) 			{ pushGroupLabel(label); }
    virtual void openHorizontalBox(const char* label) 	{ pushGroupLabel(label); }
    virtual void openVerticalBox(const char* label)  	{ pushGroupLabel(label); }
    virtual void closeBox() 							{ popGroupLabel(); };

    // -- active widgets (just add an element)

    virtual void addButton(const char* label, FAUSTFLOAT* zone) 		{ addElement(label, zone); }
    virtual void addCheckButton(const char* label, FAUSTFLOAT* zone) 	{ addElement(label, zone); }
    virtual void addVerticalSlider(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT , FAUSTFLOAT , FAUSTFLOAT , FAUSTFLOAT)
    																{ addElement(label, zone); }
    virtual void addHorizontalSlider(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT , FAUSTFLOAT , FAUSTFLOAT , FAUSTFLOAT)
    																{ addElement(label, zone); }
    virtual void addNumEntry(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT , FAUSTFLOAT , FAUSTFLOAT , FAUSTFLOAT)
    																{ addElement(label, zone); }

    // -- passive widgets (are ignored)

    virtual void addHorizontalBargraph(const char*, FAUSTFLOAT*, FAUSTFLOAT, FAUSTFLOAT) {};
    virtual void addVerticalBargraph(const char*, FAUSTFLOAT*, FAUSTFLOAT, FAUSTFLOAT) {};

	// -- metadata are not used

    virtual void declare(FAUSTFLOAT*, const char*, const char*) {}
};
#endif

#endif

/************************************************************************
 ************************************************************************
    FAUST Architecture File
	Copyright (C) 2003-2011 GRAME, Centre National de Creation Musicale
    ---------------------------------------------------------------------
    This Architecture section is free software; you can redistribute it
    and/or modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 3 of
	the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
	along with this program; If not, see <http://www.gnu.org/licenses/>.

 ************************************************************************
 ************************************************************************/
 
#ifndef __misc__
#define __misc__

#include <algorithm>
#include <map>
#include <string.h>
#include <stdlib.h>

/************************************************************************
 ************************************************************************
    FAUST Architecture File
	Copyright (C) 2003-2011 GRAME, Centre National de Creation Musicale
    ---------------------------------------------------------------------
    This Architecture section is free software; you can redistribute it
    and/or modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 3 of
	the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
	along with this program; If not, see <http://www.gnu.org/licenses/>.

 ************************************************************************
 ************************************************************************/
 
#ifndef __meta__
#define __meta__

struct Meta
{
    virtual void declare(const char* key, const char* value) = 0;
};

#endif


using std::max;
using std::min;

struct XXXX_Meta : std::map<const char*, const char*>
{
    void declare(const char* key, const char* value) { (*this)[key]=value; }
};

struct MY_Meta : Meta, std::map<const char*, const char*>
{
    void declare(const char* key, const char* value) { (*this)[key]=value; }
};

inline int	lsr(int x, int n)	{ return int(((unsigned int)x) >> n); }
inline int 	int2pow2(int x)		{ int r=0; while ((1<<r)<x) r++; return r; }

long lopt(char *argv[], const char *name, long def)
{
	int	i;
	for (i = 0; argv[i]; i++) if (!strcmp(argv[i], name)) return atoi(argv[i+1]);
	return def;
}

bool isopt(char *argv[], const char *name)
{
	int	i;
	for (i = 0; argv[i]; i++) if (!strcmp(argv[i], name)) return true;
	return false;
}

const char* lopts(char *argv[], const char *name, const char* def)
{
	int	i;
	for (i = 0; argv[i]; i++) if (!strcmp(argv[i], name)) return argv[i+1];
	return def;
}
#endif

#ifndef FAUST_GUI_H
#define FAUST_GUI_H

#include <list>
#include <map>

/*******************************************************************************
 * GUI : Abstract Graphic User Interface
 * Provides additional macchanismes to synchronize widgets and zones. Widgets
 * should both reflect the value of a zone and allow to change this value.
 ******************************************************************************/

class uiItem;
typedef void (*uiCallback)(FAUSTFLOAT val, void* data);

class clist : public std::list<uiItem*>
{
    public:
    
        virtual ~clist();
        
};

class GUI : public UI
{
    
	typedef std::map<FAUSTFLOAT*, clist*> zmap;
	
 private:
 	static std::list<GUI*>	fGuiList;
	zmap                    fZoneMap;
	bool                    fStopped;
	
 public:
		
    GUI() : fStopped(false) 
    {	
		fGuiList.push_back(this);
	}
	
    virtual ~GUI() 
    {   
        // delete all 
        zmap::iterator g;
        for (g = fZoneMap.begin(); g != fZoneMap.end(); g++) {
            delete (*g).second;
        }
        // suppress 'this' in static fGuiList
        fGuiList.remove(this);
    }

	// -- registerZone(z,c) : zone management
	
	void registerZone(FAUSTFLOAT* z, uiItem* c)
	{
  		if (fZoneMap.find(z) == fZoneMap.end()) fZoneMap[z] = new clist();
		fZoneMap[z]->push_back(c);
	} 	

	void updateAllZones();
	
	void updateZone(FAUSTFLOAT* z);
	
	static void updateAllGuis()
	{
		std::list<GUI*>::iterator g;
		for (g = fGuiList.begin(); g != fGuiList.end(); g++) {
			(*g)->updateAllZones();
		}
	}
    void addCallback(FAUSTFLOAT* zone, uiCallback foo, void* data);
    virtual void show() {};	
    virtual void run() {};
	
	virtual void stop()		{ fStopped = true; }
	bool stopped() 	{ return fStopped; }

    virtual void declare(FAUSTFLOAT* , const char* , const char*) {}
};

/**
 * User Interface Item: abstract definition
 */

class uiItem
{
  protected :
		  
	GUI*            fGUI;
	FAUSTFLOAT*		fZone;
	FAUSTFLOAT		fCache;
	
	uiItem (GUI* ui, FAUSTFLOAT* zone) : fGUI(ui), fZone(zone), fCache(-123456.654321) 
	{ 
 		ui->registerZone(zone, this); 
 	}
	
  public :
  
	virtual ~uiItem() 
    {}
	
	void modifyZone(FAUSTFLOAT v) 	
	{ 
		fCache = v;
		if (*fZone != v) {
			*fZone = v;
			fGUI->updateZone(fZone);
		}
	}
		  	
	FAUSTFLOAT		cache()			{ return fCache; }
	virtual void 	reflectZone() 	= 0;	
};

/**
 * Callback Item
 */

struct uiCallbackItem : public uiItem
{
	uiCallback	fCallback;
	void*		fData;
	
	uiCallbackItem(GUI* ui, FAUSTFLOAT* zone, uiCallback foo, void* data) 
			: uiItem(ui, zone), fCallback(foo), fData(data) {}
	
	virtual void 	reflectZone() {		
		FAUSTFLOAT 	v = *fZone;
		fCache = v; 
		fCallback(v, fData);	
	}
};

// en cours d'installation de call back. a finir!!!!!

/**
 * Update all user items reflecting zone z
 */

inline void GUI::updateZone(FAUSTFLOAT* z)
{
	FAUSTFLOAT 	v = *z;
	clist* 	l = fZoneMap[z];
	for (clist::iterator c = l->begin(); c != l->end(); c++) {
		if ((*c)->cache() != v) (*c)->reflectZone();
	}
}

/**
 * Update all user items not up to date
 */

inline void GUI::updateAllZones()
{
	for (zmap::iterator m = fZoneMap.begin(); m != fZoneMap.end(); m++) {
		FAUSTFLOAT* z = m->first;
		clist*	l = m->second;
        if (z) {
            FAUSTFLOAT	v = *z;
            for (clist::iterator c = l->begin(); c != l->end(); c++) {
                if ((*c)->cache() != v) (*c)->reflectZone();
            }
        }
	}
}

inline void GUI::addCallback(FAUSTFLOAT* zone, uiCallback foo, void* data) 
{ 
	new uiCallbackItem(this, zone, foo, data); 
};

inline clist::~clist() 
{
    std::list<uiItem*>::iterator it;
    for (it = begin(); it != end(); it++) {
        delete (*it);
    }
}

#endif
/************************************************************************

	IMPORTANT NOTE : this file contains two clearly delimited sections :
	the ARCHITECTURE section (in two parts) and the USER section. Each section
	is governed by its own copyright and license. Please check individually
	each section for license and copyright information.
*************************************************************************/

/*******************BEGIN ARCHITECTURE SECTION (part 1/2)****************/

/************************************************************************
    FAUST Architecture File
	Copyright (C) 2003-2011 GRAME, Centre National de Creation Musicale
    ---------------------------------------------------------------------
    This Architecture section is free software; you can redistribute it
    and/or modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 3 of
	the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
	along with this program; If not, see <http://www.gnu.org/licenses/>.

	EXCEPTION : As a special exception, you may create a larger work
	that contains this FAUST architecture section and distribute
	that work under terms of your choice, so long as this FAUST
	architecture section is not modified.


 ************************************************************************
 ************************************************************************/
#ifndef __faustconsole__
#define __faustconsole__

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stack>
#include <string>
#include <map>
#include <vector>
#include <iostream>


//using namespace std;

/******************************************************************************
*******************************************************************************

								USER INTERFACE

*******************************************************************************
*******************************************************************************/

struct param {
	FAUSTFLOAT* fZone; FAUSTFLOAT fMin; FAUSTFLOAT fMax;
	param(FAUSTFLOAT* z, FAUSTFLOAT init, FAUSTFLOAT a, FAUSTFLOAT b) : fZone(z), fMin(a), fMax(b) { *z = init; }
};

class CMDUI : public UI
{
    int                             fArgc;
    char**                          fArgv;
    std::vector<char*>              fFiles;
    std::stack<std::string>         fPrefix;
	std::map<std::string, param>	fKeyParam;

	void openAnyBox(const char* label)
	{
		std::string prefix;

		if (label && label[0]) {
			prefix = fPrefix.top() + "-" + label;
		} else {
			prefix = fPrefix.top();
		}
		fPrefix.push(prefix);
	}

	std::string simplify(const std::string& src)
	{
		int		i=0;
		int		level=0;
		std::string	dst;

		while (src[i] ) {

			switch (level) {

				case 0 :
				case 1 :
				case 2 :
					// Skip the begin of the label "--foo-"
					// until 3 '-' have been read
					if (src[i]=='-') { level++; }
					break;

				case 3 :
					// copy the content, but skip non alphnum
					// and content in parenthesis
					switch (src[i]) {
						case '(' :
						case '[' :
							level++;
							break;

						case '-' :
							dst += '-';
							break;

						default :
							if (isalnum(src[i])) {
								dst+= tolower(src[i]);
							}

					}
					break;

				default :
					// here we are inside parenthesis and
					// we skip the content until we are back to
					// level 3
					switch (src[i]) {

						case '(' :
						case '[' :
							level++;
							break;

						case ')' :
						case ']' :
							level--;
							break;

						default :
							break;
					}

			}
			i++;
		}
		return dst;
	}

public:

	CMDUI(int argc, char *argv[]) : UI(), fArgc(argc), fArgv(argv) { fPrefix.push("-"); }
	virtual ~CMDUI() {}

	void addOption(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT init, FAUSTFLOAT min, FAUSTFLOAT max)
	{
		std::string fullname = "-" + simplify(fPrefix.top() + "-" + label);
		fKeyParam.insert(make_pair(fullname, param(zone, init, min, max)));
	}

	virtual void addButton(const char* label, FAUSTFLOAT* zone)
	{
		addOption(label,zone,0,0,1);
	}

	virtual void addCheckButton(const char* label, FAUSTFLOAT* zone)
	{
		addOption(label,zone,0,0,1);
	}

	virtual void addVerticalSlider(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT init, FAUSTFLOAT min, FAUSTFLOAT max, FAUSTFLOAT step)
	{
		addOption(label,zone,init,min,max);
	}

	virtual void addHorizontalSlider(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT init, FAUSTFLOAT min, FAUSTFLOAT max, FAUSTFLOAT step)
	{
		addOption(label,zone,init,min,max);
	}

	virtual void addNumEntry(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT init, FAUSTFLOAT min, FAUSTFLOAT max, FAUSTFLOAT step)
	{
		addOption(label,zone,init,min,max);
	}

	// -- passive widgets

	virtual void addHorizontalBargraph(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT min, FAUSTFLOAT max) 		{}
	virtual void addVerticalBargraph(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT min, FAUSTFLOAT max) 			{}

	virtual void openTabBox(const char* label)			{ openAnyBox(label); }
	virtual void openHorizontalBox(const char* label)	{ openAnyBox(label); }
	virtual void openVerticalBox(const char* label)		{ openAnyBox(label); }

	virtual void closeBox() 							{ fPrefix.pop(); }

	virtual void show() {}
	virtual void run()
	{
		char c;
		printf("Type 'q' to quit\n");
		while ((c = getchar()) != 'q') {
			sleep(1);
		}
	}

	void printhelp_command()
	{
		std::map<std::string, param>::iterator i;
		std::cout << fArgc << "\n";
		std::cout << fArgv[0] << " option list : ";
		for (i = fKeyParam.begin(); i != fKeyParam.end(); i++) {
			std::cout << "[ " << i->first << " " << i->second.fMin << ".." << i->second.fMax <<" ] ";
		}
		std::cout << " infile outfile\n";
	}
    
    void printhelp_init()
	{
		std::map<std::string, param>::iterator i;
		std::cout << fArgc << "\n";
		std::cout << fArgv[0] << " option list : ";
		for (i = fKeyParam.begin(); i != fKeyParam.end(); i++) {
			std::cout << "[ " << i->first << " " << i->second.fMin << ".." << i->second.fMax <<" ] ";
		}
		std::cout << std::endl;
	}

	void process_command()
	{
		std::map<std::string, param>::iterator p;
		for (int i = 1; i < fArgc; i++) {
			if (fArgv[i][0] == '-') {
				if ((strcmp(fArgv[i], "-help") == 0)
                    || (strcmp(fArgv[i], "-h") == 0)
                    || (strcmp(fArgv[i], "--help") == 0)) 	{
					printhelp_command();
					exit(1);
				}
				p = fKeyParam.find(fArgv[i]);
				if (p == fKeyParam.end()) {
					std::cout << fArgv[0] << " : unrecognized option " << fArgv[i] << "\n";
					printhelp_command();
					exit(1);
				}
				char* end;
				*(p->second.fZone) = FAUSTFLOAT(strtod(fArgv[i+1], &end));
				i++;
			} else {
				fFiles.push_back(fArgv[i]);
			}
		}
	}

	int 	files()         { return fFiles.size(); }
	char* 	file (int n)	{ return fFiles[n]; }

	char* input_file ()     { std::cout << "input file " << fFiles[0] << "\n"; return fFiles[0]; }
	char* output_file() 	{ std::cout << "output file " << fFiles[1] << "\n"; return fFiles[1]; }

	void process_init()
	{
		std::map<std::string, param>::iterator p;
		for (int i = 1; i < fArgc; i++) {
			if (fArgv[i][0] == '-') {
                if ((strcmp(fArgv[i], "-help") == 0)
                    || (strcmp(fArgv[i], "-h") == 0)
                    || (strcmp(fArgv[i], "--help") == 0)) 	{
					printhelp_init();
					exit(1);
				}
				p = fKeyParam.find(fArgv[i]);
				if (p == fKeyParam.end()) {
					std::cout << fArgv[0] << " : unrecognized option " << fArgv[i] << "\n";
                    printhelp_init();
					exit(1);
				}
				char* end;
				*(p->second.fZone) = FAUSTFLOAT(strtod(fArgv[i+1], &end));
				i++;
			}
		}
	}
};

#endif

/********************END ARCHITECTURE SECTION (part 2/2)****************/
/************************************************************************

	IMPORTANT NOTE : this file contains two clearly delimited sections :
	the ARCHITECTURE section (in two parts) and the USER section. Each section
	is governed by its own copyright and license. Please check individually
	each section for license and copyright information.
*************************************************************************/

/*******************BEGIN ARCHITECTURE SECTION (part 1/2)****************/

/************************************************************************
    FAUST Architecture File
	Copyright (C) 2003-2011 GRAME, Centre National de Creation Musicale
    ---------------------------------------------------------------------
    This Architecture section is free software; you can redistribute it
    and/or modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 3 of
	the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
	along with this program; If not, see <http://www.gnu.org/licenses/>.

	EXCEPTION : As a special exception, you may create a larger work
	that contains this FAUST architecture section and distribute
	that work under terms of your choice, so long as this FAUST
	architecture section is not modified.


 ************************************************************************
 ************************************************************************/

#ifndef __alsa_dsp__
#define __alsa_dsp__

#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <pwd.h>
#include <limits.h>

#include <alsa/asoundlib.h>
/************************************************************************
	IMPORTANT NOTE : this file contains two clearly delimited sections :
	the ARCHITECTURE section (in two parts) and the USER section. Each section
	is governed by its own copyright and license. Please check individually
	each section for license and copyright information.
*************************************************************************/

/*******************BEGIN ARCHITECTURE SECTION (part 1/2)****************/

/************************************************************************
    FAUST Architecture File
	Copyright (C) 2003-2011 GRAME, Centre National de Creation Musicale
    ---------------------------------------------------------------------
    This Architecture section is free software; you can redistribute it
    and/or modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 3 of
	the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
	along with this program; If not, see <http://www.gnu.org/licenses/>.

	EXCEPTION : As a special exception, you may create a larger work
	that contains this FAUST architecture section and distribute
	that work under terms of your choice, so long as this FAUST
	architecture section is not modified.


 ************************************************************************
 ************************************************************************/
 
/******************************************************************************
*******************************************************************************

						An abstraction layer over audio layer

*******************************************************************************
*******************************************************************************/

#ifndef __audio__
#define __audio__
			
class dsp;

typedef void (* shutdown_callback)(const char* message, void* arg);

typedef void (* buffer_size_callback)(int frames, void* arg);

class audio {
    
 public:
			 audio() {}
	virtual ~audio() {}
	
	virtual bool init(const char* name, dsp*)               = 0;
	virtual bool start()                                    = 0;
	virtual void stop()                                     = 0;
    virtual void shutdown(shutdown_callback cb, void* arg)  {}
    
    virtual int get_buffer_size() = 0;
    virtual int get_sample_rate() = 0;
    
};
					
#endif
/************************************************************************
	IMPORTANT NOTE : this file contains two clearly delimited sections :
	the ARCHITECTURE section (in two parts) and the USER section. Each section
	is governed by its own copyright and license. Please check individually
	each section for license and copyright information.
*************************************************************************/

/*******************BEGIN ARCHITECTURE SECTION (part 1/2)****************/

/************************************************************************
    FAUST Architecture File
	Copyright (C) 2003-2011 GRAME, Centre National de Creation Musicale
    ---------------------------------------------------------------------
    This Architecture section is free software; you can redistribute it
    and/or modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 3 of
	the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
	along with this program; If not, see <http://www.gnu.org/licenses/>.

	EXCEPTION : As a special exception, you may create a larger work
	that contains this FAUST architecture section and distribute
	that work under terms of your choice, so long as this FAUST
	architecture section is not modified.


 ************************************************************************
 ************************************************************************/
 
/******************************************************************************
*******************************************************************************

								FAUST DSP

*******************************************************************************
*******************************************************************************/

#ifndef __dsp__
#define __dsp__

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif

class UI;

//----------------------------------------------------------------
//  signal processor definition
//----------------------------------------------------------------

class dsp {
 protected:
	int fSamplingFreq;
 public:
	dsp() {}
	virtual ~dsp() {}

	virtual int getNumInputs() 										= 0;
	virtual int getNumOutputs() 									= 0;
	virtual void buildUserInterface(UI* ui_interface) 				= 0;
	virtual void init(int samplingRate) 							= 0;
 	virtual void compute(int len, FAUSTFLOAT** inputs, FAUSTFLOAT** outputs) 	= 0;
};

// On Intel set FZ (Flush to Zero) and DAZ (Denormals Are Zero)
// flags to avoid costly denormals
#ifdef __SSE__
    #include <xmmintrin.h>
    #ifdef __SSE2__
        #define AVOIDDENORMALS _mm_setcsr(_mm_getcsr() | 0x8040)
    #else
        #define AVOIDDENORMALS _mm_setcsr(_mm_getcsr() | 0x8000)
    #endif
#else
    #define AVOIDDENORMALS
#endif

#endif

/**
DEFAULT ALSA PARAMETERS CONTROLLED BY ENVIRONMENT VARIABLES

Some default parameters of Faust's ALSA applications are controlled by the following environment variables :

    FAUST2ALSA_DEVICE   = "hw:0"
    FAUST2ALSA_FREQUENCY= 44100
    FAUST2ALSA_BUFFER   = 512
    FAUST2ALSA_PERIODS  = 2

*/

//using namespace std;

// handle 32/64 bits int size issues

#ifdef __x86_64__

#define uint32	unsigned int
#define uint64	unsigned long int

#define int32	int
#define int64	long int

#else

#define uint32	unsigned int
#define uint64	unsigned long long int

#define int32	int
#define int64	long long int
#endif

// check 32/64 bits issues are correctly handled

#define check_error(err) if (err) { printf("%s:%d, alsa error %d : %s\n", __FILE__, __LINE__, err, snd_strerror(err)); exit(1); }
#define check_error_msg(err,msg) if (err) { fprintf(stderr, "%s:%d, %s : %s(%d)\n", __FILE__, __LINE__, msg, snd_strerror(err), err); exit(1); }
#define display_error_msg(err,msg) if (err) { fprintf(stderr, "%s:%d, %s : %s(%d)\n", __FILE__, __LINE__, msg, snd_strerror(err), err); }


/**
 * Used to set the priority and scheduling of the audi#include <sys/types.h>
       #include <pwd.h>
o thread
 */
static bool setRealtimePriority ()
{
    struct passwd *         pw;
    int                     err;
    uid_t                   uid;
    struct sched_param      param;

    uid = getuid ();
    pw = getpwnam ("root");
    setuid (pw->pw_uid);
    param.sched_priority = 50; /* 0 to 99  */
    err = sched_setscheduler(0, SCHED_RR, &param);
    setuid (uid);
    return (err != -1);
}

/******************************************************************************
*******************************************************************************

								AUDIO INTERFACE

*******************************************************************************
*******************************************************************************/

enum { kRead = 1, kWrite = 2, kReadWrite = 3 };

/**
 * A convenient class to pass parameters to AudioInterface
 */
class AudioParam
{
  public:

	const char*		fCardName;
	unsigned int	fFrequency;
	unsigned int	fBuffering;
	unsigned int	fPeriods;

	unsigned int	fSoftInputs;
	unsigned int	fSoftOutputs;

  public :
	AudioParam() :
		fCardName("hw:0"),
		fFrequency(44100),
		fBuffering(512),
		fPeriods(2),
		fSoftInputs(2),
		fSoftOutputs(2)
	{}

	AudioParam&	cardName(const char* n)	{ fCardName = n; 		return *this; }
	AudioParam&	frequency(int f)		{ fFrequency = f; 		return *this; }
	AudioParam&	buffering(int fpb)		{ fBuffering = fpb; 	return *this; }
	AudioParam&	periods(int p)			{ fPeriods = p; 		return *this; }
	AudioParam&	inputs(int n)			{ fSoftInputs = n; 		return *this; }
	AudioParam&	outputs(int n)			{ fSoftOutputs = n; 	return *this; }
};

/**
 * An ALSA audio interface
 */
class AudioInterface : public AudioParam
{
 public :
	snd_pcm_t*				fOutputDevice ;
	snd_pcm_t*				fInputDevice ;
	snd_pcm_hw_params_t* 	fInputParams;
	snd_pcm_hw_params_t* 	fOutputParams;

	snd_pcm_format_t 		fSampleFormat;
	snd_pcm_access_t 		fSampleAccess;

	unsigned int			fCardInputs;
	unsigned int			fCardOutputs;

	unsigned int			fChanInputs;
	unsigned int			fChanOutputs;

	bool					fDuplexMode;

	// interleaved mode audiocard buffers
	void*		fInputCardBuffer;
	void*		fOutputCardBuffer;

	// non interleaved mode audiocard buffers
	void*		fInputCardChannels[256];
	void*		fOutputCardChannels[256];

	// non interleaved mod, floating point software buffers
	float*		fInputSoftChannels[256];
	float*		fOutputSoftChannels[256];

 public :

	const char*	cardName()				{ return fCardName;  	}
 	int			frequency()				{ return fFrequency; 	}
	int			buffering()				{ return fBuffering;  	}
	int			periods()				{ return fPeriods;  	}

	float**		inputSoftChannels()		{ return fInputSoftChannels;	}
	float**		outputSoftChannels()	{ return fOutputSoftChannels;	}

	bool		duplexMode()			{ return fDuplexMode; }

	AudioInterface(const AudioParam& ap = AudioParam()) : AudioParam(ap)
	{

		fInputDevice 			= 0;
		fOutputDevice 			= 0;
		fInputParams			= 0;
		fOutputParams			= 0;
	}

	/**
	 * Open the audio interface
	 */
	void open()
	{
		int err;

		// try to open output device, quit if fail to open output device
		err = snd_pcm_open( &fOutputDevice, fCardName, SND_PCM_STREAM_PLAYBACK, 0 ); check_error(err)

		// setup output device parameters
		err = snd_pcm_hw_params_malloc	( &fOutputParams ); 		check_error(err)
		setAudioParams(fOutputDevice, fOutputParams);

		fCardOutputs = fSoftOutputs;
		snd_pcm_hw_params_set_channels_near(fOutputDevice, fOutputParams, &fCardOutputs);
		err = snd_pcm_hw_params (fOutputDevice, fOutputParams );	check_error(err);

		// allocate alsa output buffers
		if (fSampleAccess == SND_PCM_ACCESS_RW_INTERLEAVED) {
			fOutputCardBuffer = calloc(interleavedBufferSize(fOutputParams), 1);
		} else {
			for (unsigned int i = 0; i < fCardOutputs; i++) {
				fOutputCardChannels[i] = calloc(noninterleavedBufferSize(fOutputParams), 1);
			}
        }

		// check for duplex mode (if we need and have an input device)
		if (fSoftInputs == 0) {
			fDuplexMode = false;
			fCardInputs = 0;
		} else {
			// try to open input device
			err = snd_pcm_open( &fInputDevice,  fCardName, SND_PCM_STREAM_CAPTURE, 0 );
			if (err == 0) {
				fDuplexMode = true;
			} else {
				printf("Warning : no input device");
				fDuplexMode = false;
				fCardInputs = 0;
			}
		}

		if (fDuplexMode) {

			// we have and need an input device
			// set the number of physical inputs close to what we need
			err = snd_pcm_hw_params_malloc	( &fInputParams ); 	check_error(err);
			setAudioParams(fInputDevice, fInputParams);
			fCardInputs 	= fSoftInputs;
			snd_pcm_hw_params_set_channels_near(fInputDevice, fInputParams, &fCardInputs);
			err = snd_pcm_hw_params (fInputDevice,  fInputParams );	 	check_error(err);

			// allocation of alsa buffers
			if (fSampleAccess == SND_PCM_ACCESS_RW_INTERLEAVED) {
				fInputCardBuffer = calloc(interleavedBufferSize(fInputParams), 1);
			} else {
				for (unsigned int i = 0; i < fCardInputs; i++) {
					fInputCardChannels[i] = calloc(noninterleavedBufferSize(fInputParams), 1);
				}
			}
        }

		printf("inputs : %u, outputs : %u\n", fCardInputs, fCardOutputs);

		// allocation of floating point buffers needed by the dsp code

		fChanInputs = max(fSoftInputs, fCardInputs);		assert (fChanInputs < 256);
		fChanOutputs = max(fSoftOutputs, fCardOutputs);		assert (fChanOutputs < 256);

		for (unsigned int i = 0; i < fChanInputs; i++) {
			fInputSoftChannels[i] = (float*) calloc (fBuffering, sizeof(float));
			for (unsigned int j = 0; j < fBuffering; j++) {
				fInputSoftChannels[i][j] = 0.0;
			}
		}

		for (unsigned int i = 0; i < fChanOutputs; i++) {
			fOutputSoftChannels[i] = (float*) calloc (fBuffering, sizeof(float));
			for (unsigned int j = 0; j < fBuffering; j++) {
				fOutputSoftChannels[i][j] = 0.0;
			}
		}
	}

	void setAudioParams(snd_pcm_t* stream, snd_pcm_hw_params_t* params)
	{
		int	err;

		// set params record with initial values
		err = snd_pcm_hw_params_any	( stream, params );
		check_error_msg(err, "unable to init parameters")

		// set alsa access mode (and fSampleAccess field) either to non interleaved or interleaved

		err = snd_pcm_hw_params_set_access (stream, params, SND_PCM_ACCESS_RW_NONINTERLEAVED );
		if (err) {
			err = snd_pcm_hw_params_set_access (stream, params, SND_PCM_ACCESS_RW_INTERLEAVED );
			check_error_msg(err, "unable to set access mode neither to non-interleaved or to interleaved");
		}
		snd_pcm_hw_params_get_access(params, &fSampleAccess);

		// search for 32-bits or 16-bits format
		err = snd_pcm_hw_params_set_format (stream, params, SND_PCM_FORMAT_S32);
		if (err) {
			err = snd_pcm_hw_params_set_format (stream, params, SND_PCM_FORMAT_S16);
		 	check_error_msg(err, "unable to set format to either 32-bits or 16-bits");
		}
		snd_pcm_hw_params_get_format(params, &fSampleFormat);
		// set sample frequency
		snd_pcm_hw_params_set_rate_near (stream, params, &fFrequency, 0);

		// set period and period size (buffering)
		err = snd_pcm_hw_params_set_period_size	(stream, params, fBuffering, 0);
		check_error_msg(err, "period size not available");

		err = snd_pcm_hw_params_set_periods (stream, params, fPeriods, 0);
		check_error_msg(err, "number of periods not available");
	}

	ssize_t interleavedBufferSize (snd_pcm_hw_params_t* params)
	{
		_snd_pcm_format 	format;  	snd_pcm_hw_params_get_format(params, &format);
		snd_pcm_uframes_t 	psize;		snd_pcm_hw_params_get_period_size(params, &psize, NULL);
		unsigned int 		channels; 	snd_pcm_hw_params_get_channels(params, &channels);
		ssize_t bsize = snd_pcm_format_size (format, psize * channels);
		return bsize;
	}

	ssize_t noninterleavedBufferSize (snd_pcm_hw_params_t* params)
	{
		_snd_pcm_format 	format;  	snd_pcm_hw_params_get_format(params, &format);
		snd_pcm_uframes_t 	psize;		snd_pcm_hw_params_get_period_size(params, &psize, NULL);
		ssize_t bsize = snd_pcm_format_size (format, psize);
		return bsize;
	}

	void close()
	{}

	/**
	 * Read audio samples from the audio card. Convert samples to floats and take
	 * care of interleaved buffers
	 */
	void read()
	{
        if (fSampleAccess == SND_PCM_ACCESS_RW_INTERLEAVED) {

			int count = snd_pcm_readi(fInputDevice, fInputCardBuffer, fBuffering);
			if (count < 0) {
				 //display_error_msg(count, "reading samples");
				 snd_pcm_prepare(fInputDevice);
				 //check_error_msg(err, "preparing input stream");
			}

			if (fSampleFormat == SND_PCM_FORMAT_S16) {

				short* buffer16b = (short*)fInputCardBuffer;
				for (unsigned int s = 0; s < fBuffering; s++) {
					for (unsigned int c = 0; c < fCardInputs; c++) {
						fInputSoftChannels[c][s] = float(buffer16b[c + s*fCardInputs])*(1.0/float(SHRT_MAX));
					}
				}

			} else if (fSampleFormat == SND_PCM_FORMAT_S32) {

				int32* buffer32b = (int32*)fInputCardBuffer;
				for (unsigned int s = 0; s < fBuffering; s++) {
					for (unsigned int c = 0; c < fCardInputs; c++) {
						fInputSoftChannels[c][s] = float(buffer32b[c + s*fCardInputs])*(1.0/float(INT_MAX));
					}
				}
			} else {

				printf("unrecognized input sample format : %u\n", fSampleFormat);
				exit(1);
			}

		} else if (fSampleAccess == SND_PCM_ACCESS_RW_NONINTERLEAVED) {

			int count = snd_pcm_readn(fInputDevice, fInputCardChannels, fBuffering);
			if (count < 0) {
				 //display_error_msg(count, "reading samples");
				 snd_pcm_prepare(fInputDevice);
				 //check_error_msg(err, "preparing input stream");
			}

			if (fSampleFormat == SND_PCM_FORMAT_S16) {

				for (unsigned int c = 0; c < fCardInputs; c++) {
					short* chan16b = (short*)fInputCardChannels[c];
					for (unsigned int s = 0; s < fBuffering; s++) {
						fInputSoftChannels[c][s] = float(chan16b[s])*(1.0/float(SHRT_MAX));
					}
				}

			} else if (fSampleFormat == SND_PCM_FORMAT_S32) {

				for (unsigned int c = 0; c < fCardInputs; c++) {
					int32* chan32b = (int32*)fInputCardChannels[c];
					for (unsigned int s = 0; s < fBuffering; s++) {
						fInputSoftChannels[c][s] = float(chan32b[s])*(1.0/float(INT_MAX));
					}
				}
			} else {
				printf("unrecognized input sample format : %u\n", fSampleFormat);
				exit(1);
			}

		} else {
			check_error_msg(-10000, "unknow access mode");
		}
    }

	/**
	 * write the output soft channels to the audio card. Convert sample
	 * format and interleaves buffers when needed
	 */
	void write()
	{
		recovery :

		if (fSampleAccess == SND_PCM_ACCESS_RW_INTERLEAVED) {

			if (fSampleFormat == SND_PCM_FORMAT_S16) {

				short* buffer16b = (short*)fOutputCardBuffer;
				for (unsigned int f = 0; f < fBuffering; f++) {
					for (unsigned int c = 0; c < fCardOutputs; c++) {
						float x = fOutputSoftChannels[c][f];
						buffer16b[c + f*fCardOutputs] = short( max(min(x,1.0f),-1.0f) * float(SHRT_MAX) ) ;
					}
				}

			} else if (fSampleFormat == SND_PCM_FORMAT_S32)  {

				int32* buffer32b = (int32*)fOutputCardBuffer;
				for (unsigned int f = 0; f < fBuffering; f++) {
					for (unsigned int c = 0; c < fCardOutputs; c++) {
						float x = fOutputSoftChannels[c][f];
						buffer32b[c + f*fCardOutputs] = int( max(min(x,1.0f),-1.0f) * float(INT_MAX) ) ;
					}
				}
			} else {

				printf("unrecognized output sample format : %u\n", fSampleFormat);
				exit(1);
			}

			int count = snd_pcm_writei(fOutputDevice, fOutputCardBuffer, fBuffering);
			if (count<0) {
				//display_error_msg(count, "w3");
				snd_pcm_prepare(fOutputDevice);
				//check_error_msg(err, "preparing output stream");
				goto recovery;
			}


		} else if (fSampleAccess == SND_PCM_ACCESS_RW_NONINTERLEAVED) {

			if (fSampleFormat == SND_PCM_FORMAT_S16) {

				for (unsigned int c = 0; c < fCardOutputs; c++) {
					short* chan16b = (short*) fOutputCardChannels[c];
					for (unsigned int f = 0; f < fBuffering; f++) {
						float x = fOutputSoftChannels[c][f];
						chan16b[f] = short( max(min(x,1.0f),-1.0f) * float(SHRT_MAX) ) ;
					}
				}

			} else if (fSampleFormat == SND_PCM_FORMAT_S32) {

				for (unsigned int c = 0; c < fCardOutputs; c++) {
					int32* chan32b = (int32*) fOutputCardChannels[c];
					for (unsigned int f = 0; f < fBuffering; f++) {
						float x = fOutputSoftChannels[c][f];
						chan32b[f] = int( max(min(x,1.0f),-1.0f) * float(INT_MAX) ) ;
					}
				}

			} else {

				printf("unrecognized output sample format : %u\n", fSampleFormat);
				exit(1);
			}

			int count = snd_pcm_writen(fOutputDevice, fOutputCardChannels, fBuffering);
			if (count<0) {
				//display_error_msg(count, "w3");
				snd_pcm_prepare(fOutputDevice);
				//check_error_msg(err, "preparing output stream");
				goto recovery;
			}

		} else {
			check_error_msg(-10000, "unknow access mode");
		}
	}

	/**
	 *  print short information on the audio device
	 */
	void shortinfo()
	{
		int						err;
		snd_ctl_card_info_t*	card_info;
    	snd_ctl_t*				ctl_handle;
		err = snd_ctl_open (&ctl_handle, fCardName, 0);		check_error(err);
		snd_ctl_card_info_alloca (&card_info);
		err = snd_ctl_card_info(ctl_handle, card_info);		check_error(err);
		printf("%s|%d|%d|%d|%d|%s\n",
				snd_ctl_card_info_get_driver(card_info),
				fCardInputs, fCardOutputs,
				fFrequency, fBuffering,
				snd_pcm_format_name((_snd_pcm_format)fSampleFormat));
	}

	/**
	 *  print more detailled information on the audio device
	 */
	void longinfo()
	{
		int						err;
		snd_ctl_card_info_t*	card_info;
    	snd_ctl_t*				ctl_handle;

		printf("Audio Interface Description :\n");
		printf("Sampling Frequency : %d, Sample Format : %s, buffering : %d\n",
				fFrequency, snd_pcm_format_name((_snd_pcm_format)fSampleFormat), fBuffering);
		printf("Software inputs : %2d, Software outputs : %2d\n", fSoftInputs, fSoftOutputs);
		printf("Hardware inputs : %2d, Hardware outputs : %2d\n", fCardInputs, fCardOutputs);
		printf("Channel inputs  : %2d, Channel outputs  : %2d\n", fChanInputs, fChanOutputs);

		// affichage des infos de la carte
		err = snd_ctl_open (&ctl_handle, fCardName, 0);		check_error(err);
		snd_ctl_card_info_alloca (&card_info);
		err = snd_ctl_card_info(ctl_handle, card_info);		check_error(err);
		printCardInfo(card_info);

		// affichage des infos liees aux streams d'entree-sortie
		if (fSoftInputs > 0)	printHWParams(fInputParams);
		if (fSoftOutputs > 0)	printHWParams(fOutputParams);
	}

	void printCardInfo(snd_ctl_card_info_t*	ci)
	{
		printf("Card info (address : %p)\n", ci);
		printf("\tID         = %s\n", snd_ctl_card_info_get_id(ci));
		printf("\tDriver     = %s\n", snd_ctl_card_info_get_driver(ci));
		printf("\tName       = %s\n", snd_ctl_card_info_get_name(ci));
		printf("\tLongName   = %s\n", snd_ctl_card_info_get_longname(ci));
		printf("\tMixerName  = %s\n", snd_ctl_card_info_get_mixername(ci));
		printf("\tComponents = %s\n", snd_ctl_card_info_get_components(ci));
		printf("--------------\n");
	}

	void printHWParams( snd_pcm_hw_params_t* params )
	{
		printf("HW Params info (address : %p)\n", params);
#if 0
		printf("\tChannels    = %d\n", snd_pcm_hw_params_get_channels(params));
		printf("\tFormat      = %s\n", snd_pcm_format_name((_snd_pcm_format)snd_pcm_hw_params_get_format(params)));
		printf("\tAccess      = %s\n", snd_pcm_access_name((_snd_pcm_access)snd_pcm_hw_params_get_access(params)));
		printf("\tRate        = %d\n", snd_pcm_hw_params_get_rate(params, NULL));
		printf("\tPeriods     = %d\n", snd_pcm_hw_params_get_periods(params, NULL));
		printf("\tPeriod size = %d\n", (int)snd_pcm_hw_params_get_period_size(params, NULL));
		printf("\tPeriod time = %d\n", snd_pcm_hw_params_get_period_time(params, NULL));
		printf("\tBuffer size = %d\n", (int)snd_pcm_hw_params_get_buffer_size(params));
		printf("\tBuffer time = %d\n", snd_pcm_hw_params_get_buffer_time(params, NULL));
#endif
		printf("--------------\n");
	}

};

// lopt : Scan Command Line long int Arguments
long lopt(int argc, char *argv[], const char* longname, const char* shortname, long def)
{
	for (int i=2; i<argc; i++)
		if ( strcmp(argv[i-1], shortname) == 0 || strcmp(argv[i-1], longname) == 0 )
			return atoi(argv[i]);
	return def;
}

// sopt : Scan Command Line string Arguments
const char* sopt(int argc, char *argv[], const char* longname, const char* shortname, const char* def)
{
	for (int i=2; i<argc; i++)
		if ( strcmp(argv[i-1], shortname) == 0 || strcmp(argv[i-1], longname) == 0 )
			return argv[i];
	return def;
}

// fopt : Scan Command Line flag option (without argument), return true if the flag
bool fopt(int argc, char *argv[], const char* longname, const char* shortname)
{
	for (int i=1; i<argc; i++)
		if ( strcmp(argv[i], shortname) == 0 || strcmp(argv[i], longname) == 0 )
			return true;
	return false;
}

/**
 * Return the value of an environment variable or defval if undefined.
 */
static int getDefaultEnv(const char* name, int defval)
{
    const char* str = getenv(name);
    if (str) {
        return atoi(str);
    } else {
        return defval;
    }
}

/**
 * Return the value of an environment variable or defval if undefined.
 */
static const char* getDefaultEnv(const char* name, const char* defval)
{
    const char* str = getenv(name);
    if (str) {
        return str;
    } else {
        return defval;
    }
}

/******************************************************************************
*******************************************************************************

							   ALSA audio interface

*******************************************************************************
*******************************************************************************/
static void* __run(void* ptr);

class alsaaudio : public audio
{
	AudioInterface*	fAudio;
	dsp* 			fDSP;
	pthread_t 		fAudioThread;
	bool 			fRunning;

 public:

	 alsaaudio(int argc, char *argv[], dsp* DSP) : fAudio(0), fDSP(DSP), fRunning(false) {
			fAudio = new AudioInterface (
				AudioParam().cardName( sopt(argc, argv, "--device", "-d",  	getDefaultEnv("FAUST2ALSA_DEVICE", "hw:0")  ) )
				.frequency( lopt(argc, argv, "--frequency", "-f", 			getDefaultEnv("FAUST2ALSA_FREQUENCY",44100) ) )
				.buffering( lopt(argc, argv, "--buffer", "-b",    			getDefaultEnv("FAUST2ALSA_BUFFER",512)     ) )
				.periods( lopt(argc, argv, "--periods", "-p",     			getDefaultEnv("FAUST2ALSA_PERIODS",2)       ) )
				.inputs(DSP->getNumInputs())
				.outputs(DSP->getNumOutputs()));
		}

	virtual ~alsaaudio() { stop(); delete fAudio; }

	virtual bool init(const char */*name*/, dsp* DSP) {
		fAudio->open();
	    DSP->init(fAudio->frequency());
 		return true;
	}

	virtual bool start() {
		fRunning = true;
		if (pthread_create(&fAudioThread, 0, __run, this)) {
			fRunning = false;
        }
		return fRunning;
	}

	virtual void stop() {
		if (fRunning) {
			fRunning = false;
			pthread_join(fAudioThread, 0);
		}
	}
    
    virtual int get_buffer_size() { return fAudio->buffering(); }
    virtual int get_sample_rate() { return fAudio->frequency(); }

	virtual void run() {
		bool rt = setRealtimePriority();
		printf(rt ? "RT : ":"NRT: "); fAudio->shortinfo();
        AVOIDDENORMALS;
		if (fAudio->duplexMode()) {
            fAudio->write();
			fAudio->write();
			while (fRunning) {
				fAudio->read();
				fDSP->compute(fAudio->buffering(), fAudio->inputSoftChannels(), fAudio->outputSoftChannels());
				fAudio->write();
			}
        } else {
            fAudio->write();
			while (fRunning) {
				fDSP->compute(fAudio->buffering(), fAudio->inputSoftChannels(), fAudio->outputSoftChannels());
				fAudio->write();
			}
		}
	}
};

void* __run (void* ptr)
{
	alsaaudio * alsa = (alsaaudio*)ptr;
	alsa->run();
	return 0;
}

#endif

/********************END ARCHITECTURE SECTION (part 2/2)****************/


#ifdef OSCCTRL
/*
   Copyright (C) 2011 Grame - Lyon
   All rights reserved.
   Redistribution and use in source and binary forms, with or without
   modification, are permitted.
*/

#ifndef __OSCUI__
#define __OSCUI__

/*

  Faust Project

  Copyright (C) 2011 Grame

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

  Grame Research Laboratory, 9 rue du Garet, 69001 Lyon - France
  research@grame.fr

*/

#ifndef __OSCControler__
#define __OSCControler__

#include <string>
/*

  Copyright (C) 2011 Grame

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

  Grame Research Laboratory, 9 rue du Garet, 69001 Lyon - France
  research@grame.fr

*/


#ifndef __FaustFactory__
#define __FaustFactory__

#include <stack>
#include <string>
#include <sstream>

/*

  Copyright (C) 2011 Grame

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

  Grame Research Laboratory, 9 rue du Garet, 69001 Lyon - France
  research@grame.fr

*/


#ifndef __FaustNode__
#define __FaustNode__

#include <string>
#include <vector>

/*

  Copyright (C) 2011 Grame

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

  Grame Research Laboratory, 9 rue du Garet, 69001 Lyon - France
  research@grame.fr

*/


#ifndef __MessageDriven__
#define __MessageDriven__

#include <string>
#include <vector>

/*

  Copyright (C) 2010  Grame

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

  Grame Research Laboratory, 9 rue du Garet, 69001 Lyon - France
  research@grame.fr

*/

#ifndef __MessageProcessor__
#define __MessageProcessor__

namespace oscfaust
{

class Message;
//--------------------------------------------------------------------------
/*!
	\brief an abstract class for objects able to process OSC messages	
*/
class MessageProcessor
{
	public:
		virtual		~MessageProcessor() {}
		virtual void processMessage( const Message* msg ) = 0;
};

} // end namespoace

#endif
/*

  Copyright (C) 2011 Grame

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

  Grame Research Laboratory, 9 rue du Garet, 69001 Lyon - France
  research@grame.fr

*/

#ifndef __smartpointer__
#define __smartpointer__

#include <cassert>

namespace oscfaust
{

/*!
\brief the base class for smart pointers implementation

	Any object that want to support smart pointers should
	inherit from the smartable class which provides reference counting
	and automatic delete when the reference count drops to zero.
*/
class smartable {
	private:
		unsigned 	refCount;		
	public:
		//! gives the reference count of the object
		unsigned refs() const         { return refCount; }
		//! addReference increments the ref count and checks for refCount overflow
		void addReference()           { refCount++; assert(refCount != 0); }
		//! removeReference delete the object when refCount is zero		
		void removeReference()		  { if (--refCount == 0) delete this; }
		
	protected:
		smartable() : refCount(0) {}
		smartable(const smartable&): refCount(0) {}
		//! destructor checks for non-zero refCount
		virtual ~smartable()    
        { 
            /* 
                See "Static SFaustNode create (const char* name, C* zone, C init, C min, C max, const char* prefix, GUI* ui)" comment.
                assert (refCount == 0); 
            */
         }
		smartable& operator=(const smartable&) { return *this; }
};

/*!
\brief the smart pointer implementation

	A smart pointer is in charge of maintaining the objects reference count 
	by the way of pointers operators overloading. It supports class 
	inheritance and conversion whenever possible.
\n	Instances of the SMARTP class are supposed to use \e smartable types (or at least
	objects that implements the \e addReference and \e removeReference
	methods in a consistent way).
*/
template<class T> class SMARTP {
	private:
		//! the actual pointer to the class
		T* fSmartPtr;

	public:
		//! an empty constructor - points to null
		SMARTP()	: fSmartPtr(0) {}
		//! build a smart pointer from a class pointer
		SMARTP(T* rawptr) : fSmartPtr(rawptr)              { if (fSmartPtr) fSmartPtr->addReference(); }
		//! build a smart pointer from an convertible class reference
		template<class T2> 
		SMARTP(const SMARTP<T2>& ptr) : fSmartPtr((T*)ptr) { if (fSmartPtr) fSmartPtr->addReference(); }
		//! build a smart pointer from another smart pointer reference
		SMARTP(const SMARTP& ptr) : fSmartPtr((T*)ptr)     { if (fSmartPtr) fSmartPtr->addReference(); }

		//! the smart pointer destructor: simply removes one reference count
		~SMARTP()  { if (fSmartPtr) fSmartPtr->removeReference(); }
		
		//! cast operator to retrieve the actual class pointer
		operator T*() const  { return fSmartPtr;	}

		//! '*' operator to access the actual class pointer
		T& operator*() const {
			// checks for null dereference
			assert (fSmartPtr != 0);
			return *fSmartPtr;
		}

		//! operator -> overloading to access the actual class pointer
		T* operator->() const	{ 
			// checks for null dereference
			assert (fSmartPtr != 0);
			return fSmartPtr;
		}

		//! operator = that moves the actual class pointer
		template <class T2>
		SMARTP& operator=(T2 p1_)	{ *this=(T*)p1_; return *this; }

		//! operator = that moves the actual class pointer
		SMARTP& operator=(T* p_)	{
			// check first that pointers differ
			if (fSmartPtr != p_) {
				// increments the ref count of the new pointer if not null
				if (p_ != 0) p_->addReference();
				// decrements the ref count of the old pointer if not null
				if (fSmartPtr != 0) fSmartPtr->removeReference();
				// and finally stores the new actual pointer
				fSmartPtr = p_;
			}
			return *this;
		}
		//! operator < to support SMARTP map with Visual C++
		bool operator<(const SMARTP<T>& p_)	const			  { return fSmartPtr < ((T *) p_); }
		//! operator = to support inherited class reference
		SMARTP& operator=(const SMARTP<T>& p_)                { return operator=((T *) p_); }
		//! dynamic cast support
		template<class T2> SMARTP& cast(T2* p_)               { return operator=(dynamic_cast<T*>(p_)); }
		//! dynamic cast support
		template<class T2> SMARTP& cast(const SMARTP<T2>& p_) { return operator=(dynamic_cast<T*>(p_)); }
};

}

#endif

namespace oscfaust
{

class Message;
class OSCRegexp;
class MessageDriven;
typedef class SMARTP<MessageDriven>	SMessageDriven;

//--------------------------------------------------------------------------
/*!
	\brief a base class for objects accepting OSC messages
	
	Message driven objects are hierarchically organized in a tree.
	They provides the necessary to dispatch an OSC message to its destination
	node, according to the message OSC address. 
	
	The principle of the dispatch is the following:
	- first the processMessage() method should be called on the top level node
	- next processMessage call propose 
	
	
*/
class MessageDriven : public MessageProcessor, public smartable
{
	std::string						fName;			///< the node name
	std::string						fOSCPrefix;		///< the node OSC address prefix (OSCAddress = fOSCPrefix + '/' + fName)
	std::vector<SMessageDriven>		fSubNodes;		///< the subnodes of the current node

	protected:
				 MessageDriven(const char *name, const char *oscprefix) : fName (name), fOSCPrefix(oscprefix) {}
		virtual ~MessageDriven() {}

	public:
		static SMessageDriven create (const char* name, const char *oscprefix)	{ return new MessageDriven(name, oscprefix); }

		/*!
			\brief OSC message processing method.
			\param msg the osc message to be processed
			The method should be called on the top level node.
		*/
		virtual void	processMessage( const Message* msg );

		/*!
			\brief propose an OSc message at a given hierarchy level.
			\param msg the osc message currently processed
			\param regexp a regular expression based on the osc address head
			\param addrTail the osc address tail
			
			The method first tries to match the regular expression with the object name. 
			When it matches:
			- it calls \c accept when \c addrTail is empty 
			- or it \c propose the message to its subnodes when \c addrTail is not empty. 
			  In this case a new \c regexp is computed with the head of \c addrTail and a new \c addrTail as well.
		*/
		virtual void	propose( const Message* msg, const OSCRegexp* regexp, const std::string addrTail);

		/*!
			\brief accept an OSC message. 
			\param msg the osc message currently processed
			\return true when the message is processed by the node
			
			The method is called only for the destination nodes. The real message acceptance is the node 
			responsability and may depend on the message content.
		*/
		virtual bool	accept( const Message* msg );

		/*!
			\brief handler for the \c 'get' message
			\param ipdest the output message destination IP
			
			The \c 'get' message is supported by every node:
			- it is propagated to the subnodes until it reaches terminal nodes
			- a terminal node send its state on \c 'get' request to the IP address given as parameter.
			The \c get method is basically called by the accept method.
		*/
		virtual void	get (unsigned long ipdest) const;

		/*!
			\brief handler for the \c 'get' 'attribute' message
			\param ipdest the output message destination IP
			\param what the requested attribute
			
			The \c 'get' message is supported by every node:
			- it is propagated to the subnodes until it reaches terminal nodes
			- a terminal node send its state on \c 'get' request to the IP address given as parameter.
			The \c get method is basically called by the accept method.
		*/
		virtual void	get (unsigned long ipdest, const std::string & what) const {}

		void			add ( SMessageDriven node )	{ fSubNodes.push_back (node); }
		const char*		getName() const				{ return fName.c_str(); }
		std::string		getOSCAddress() const;
		int				size () const				{ return fSubNodes.size (); }
		
		const std::string&	name() const			{ return fName; }
		SMessageDriven	subnode (int i) 			{ return fSubNodes[i]; }
};

} // end namespoace

#endif
/*

  Copyright (C) 2011  Grame

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

  Grame Research Laboratory, 9 rue du Garet, 69001 Lyon - France
  research@grame.fr

*/


#ifndef __Message__
#define __Message__

#include <string>
#include <vector>

namespace oscfaust
{

class OSCStream;
template <typename T> class MsgParam;
class baseparam;
typedef SMARTP<baseparam>	Sbaseparam;

//--------------------------------------------------------------------------
/*!
	\brief base class of a message parameters
*/
class baseparam : public smartable
{
	public:
		virtual ~baseparam() {}

		/*!
		 \brief utility for parameter type checking
		*/
		template<typename X> bool isType() const { return dynamic_cast<const MsgParam<X>*> (this) != 0; }
		/*!
		 \brief utility for parameter convertion
		 \param errvalue the returned value when no conversion applies
		 \return the parameter value when the type matches
		*/
		template<typename X> X	value(X errvalue) const 
			{ const MsgParam<X>* o = dynamic_cast<const MsgParam<X>*> (this); return o ? o->getValue() : errvalue; }
		/*!
		 \brief utility for parameter comparison
		*/
		template<typename X> bool	equal(const baseparam& p) const 
			{ 
				const MsgParam<X>* a = dynamic_cast<const MsgParam<X>*> (this); 
				const MsgParam<X>* b = dynamic_cast<const MsgParam<X>*> (&p);
				return a && b && (a->getValue() == b->getValue());
			}
		/*!
		 \brief utility for parameter comparison
		*/
		bool operator==(const baseparam& p) const 
			{ 
				return equal<float>(p) || equal<int>(p) || equal<std::string>(p);
			}
		bool operator!=(const baseparam& p) const
			{ 
				return !equal<float>(p) && !equal<int>(p) && !equal<std::string>(p);
			}
			
		virtual SMARTP<baseparam> copy() const = 0;
};

//--------------------------------------------------------------------------
/*!
	\brief template for a message parameter
*/
template <typename T> class MsgParam : public baseparam
{
	T fParam;
	public:
				 MsgParam(T val) : fParam(val)	{}
		virtual ~MsgParam() {}
		
		T	getValue() const { return fParam; }
		
		virtual SMARTP<baseparam> copy() const { return new MsgParam<T>(fParam); }
};

//--------------------------------------------------------------------------
/*!
	\brief a message description
	
	A message is composed of an address (actually an OSC address),
	a message string that may be viewed as a method name
	and a list of message parameters.
*/
class Message
{
	public:
		typedef SMARTP<baseparam>		argPtr;		///< a message argument ptr type
		typedef std::vector<argPtr>		argslist;	///< args list type

	private:
		unsigned long	fSrcIP;			///< the message source IP number
		std::string	fAddress;			///< the message osc destination address
		argslist	fArguments;			///< the message arguments
	
	public:
			/*!
				\brief an empty message constructor
			*/
			 Message() {}
			/*!
				\brief a message constructor
				\param address the message destination address
			*/
			 Message(const std::string& address) : fAddress(address) {}
			/*!
				\brief a message constructor
				\param address the message destination address
				\param args the message parameters
			*/
			 Message(const std::string& address, const argslist& args) 
				: fAddress(address), fArguments(args) {}
			/*!
				\brief a message constructor
				\param msg a message
			*/
			 Message(const Message& msg);
	virtual ~Message() {} //{ freed++; std::cout << "running messages: " << (allocated - freed) << std::endl; }

	/*!
		\brief adds a parameter to the message
		\param val the parameter
	*/
	template <typename T> void add(T val)	{ fArguments.push_back(new MsgParam<T>(val)); }
	/*!
		\brief adds a float parameter to the message
		\param val the parameter value
	*/
	void	add(float val)					{ add<float>(val); }
	/*!
		\brief adds an int parameter to the message
		\param val the parameter value
	*/
	void	add(int val)					{ add<int>(val); }
	/*!
		\brief adds a string parameter to the message
		\param val the parameter value
	*/
	void	add(const std::string& val)		{ add<std::string>(val); }
	
	/*!
		\brief adds a parameter to the message
		\param val the parameter
	*/
	void	add( argPtr val )				{ fArguments.push_back( val ); }

	/*!
		\brief sets the message address
		\param addr the address
	*/
	void				setSrcIP(unsigned long addr)		{ fSrcIP = addr; }

	/*!
		\brief sets the message address
		\param addr the address
	*/
	void				setAddress(const std::string& addr)		{ fAddress = addr; }
	/*!
		\brief print the message
		\param out the output stream
	*/
	void				print(std::ostream& out) const;
	/*!
		\brief send the message to OSC
		\param out the OSC output stream
	*/
	void				print(OSCStream& out) const;
	/*!
		\brief print message arguments
		\param out the OSC output stream
	*/
	void				printArgs(OSCStream& out) const;

	/// \brief gives the message address
	const std::string&	address() const		{ return fAddress; }
	/// \brief gives the message parameters list
	const argslist&		params() const		{ return fArguments; }
	/// \brief gives the message parameters list
	argslist&			params()			{ return fArguments; }
	/// \brief gives the message source IP 
	unsigned long		src() const			{ return fSrcIP; }
	/// \brief gives the message parameters count
	int					size() const		{ return fArguments.size(); }
	
	bool operator == (const Message& other) const;	


	/*!
		\brief gives a message float parameter
		\param i the parameter index (0 <= i < size())
		\param val on output: the parameter value when the parameter type matches
		\return false when types don't match
	*/
	bool	param(int i, float& val) const		{ val = params()[i]->value<float>(val); return params()[i]->isType<float>(); }
	/*!
		\brief gives a message int parameter
		\param i the parameter index (0 <= i < size())
		\param val on output: the parameter value when the parameter type matches
		\return false when types don't match
	*/
	bool	param(int i, int& val) const		{ val = params()[i]->value<int>(val); return params()[i]->isType<int>(); }
	/*!
		\brief gives a message int parameter
		\param i the parameter index (0 <= i < size())
		\param val on output: the parameter value when the parameter type matches
		\return false when types don't match
	*/
	bool	param(int i, unsigned int& val) const		{ val = params()[i]->value<int>(val); return params()[i]->isType<int>(); }
	/*!
		\brief gives a message int parameter
		\param i the parameter index (0 <= i < size())
		\param val on output: the parameter value when the parameter type matches
		\return false when types don't match
		\note a boolean value is handled as integer
	*/
	bool	param(int i, bool& val) const		{ int ival = 0; ival = params()[i]->value<int>(ival); val = ival!=0; return params()[i]->isType<int>(); }
	/*!
		\brief gives a message int parameter
		\param i the parameter index (0 <= i < size())
		\param val on output: the parameter value when the parameter type matches
		\return false when types don't match
	*/
	bool	param(int i, long int& val) const	{ val = long(params()[i]->value<int>(val)); return params()[i]->isType<int>(); }
	/*!
		\brief gives a message string parameter
		\param i the parameter index (0 <= i < size())
		\param val on output: the parameter value when the parameter type matches
		\return false when types don't match
	*/
	bool	param(int i, std::string& val) const { val = params()[i]->value<std::string>(val); return params()[i]->isType<std::string>(); }
};


} // end namespoace

#endif

class GUI;

namespace oscfaust
{

/**
 * map (rescale) input values to output values
 */
template <typename C> struct mapping
{
//	const C fMinIn;	
//	const C fMaxIn;
	const C fMinOut;
	const C fMaxOut;
//	const C fScale;

//	mapping(C imin, C imax, C omin, C omax) : fMinIn(imin), fMaxIn(imax), 
//											fMinOut(omin), fMaxOut(omax), 
//											fScale( (fMaxOut-fMinOut)/(fMaxIn-fMinIn) ) {}
	mapping(C omin, C omax) : fMinOut(omin), fMaxOut(omax) {}
//	C scale (C x) { C z = (x < fMinIn) ? fMinIn : (x > fMaxIn) ? fMaxIn : x; return fMinOut + (z - fMinIn) * fScale; }
	C clip (C x) { return (x < fMinOut) ? fMinOut : (x > fMaxOut) ? fMaxOut : x; }
};

//--------------------------------------------------------------------------
/*!
	\brief a faust node is a terminal node and represents a faust parameter controler
*/
template <typename C> class FaustNode : public MessageDriven, public uiItem
{
	mapping<C>	fMapping;
	
	bool	store (C val)			{ *fZone = fMapping.clip(val); return true; }
	void	sendOSC () const;


	protected:
		FaustNode(const char *name, C* zone, C init, C min, C max, const char* prefix, GUI* ui, bool initZone) 
			: MessageDriven (name, prefix), uiItem (ui, zone), fMapping(min, max)
			{ 
                if(initZone)
                    *zone = init; 
            }
			
		virtual ~FaustNode() {}

	public:
		typedef SMARTP<FaustNode<C> > SFaustNode;
		static SFaustNode create (const char* name, C* zone, C init, C min, C max, const char* prefix, GUI* ui, bool initZone)	
        { 
            SFaustNode node = new FaustNode(name, zone, init, min, max, prefix, ui, initZone); 
            /*
                Since FaustNode is a subclass of uiItem, the pointer will also be kept in the GUI class, and it's desallocation will be done there.
                So we don't want to have smartpointer logic desallocate it and we increment the refcount.
            */
            node->addReference(); 
            return node; 
        }

		virtual bool	accept( const Message* msg )			///< handler for the 'accept' message
		{
			if (msg->size() == 1) {			// checks for the message parameters count
											// messages with a param count other than 1 are rejected
				int ival; float fval;
				if (msg->param(0, fval)) return store (C(fval));				// accepts float values
				else if (msg->param(0, ival)) return store (C(ival));	// but accepts also int values
			}
			return MessageDriven::accept(msg);
		}

		virtual void	get (unsigned long ipdest) const;		///< handler for the 'get' message
		virtual void 	reflectZone()			{ sendOSC (); fCache = *fZone;}
};



} // end namespoace

#endif

class GUI;
namespace oscfaust
{

class OSCIO;
class RootNode;
typedef class SMARTP<RootNode>		SRootNode;
class MessageDriven;
typedef class SMARTP<MessageDriven>	SMessageDriven;

//--------------------------------------------------------------------------
/*!
	\brief a factory to build a OSC UI hierarchy
	
	Actually, makes use of a stack to build the UI hierarchy.
	It includes a pointer to a OSCIO controler, but just to give it to the root node.
*/
class FaustFactory
{
	std::stack<SMessageDriven>	fNodes;		///< maintains the current hierarchy level
	SRootNode					fRoot;		///< keep track of the root node
	OSCIO * 					fIO;		///< hack to support audio IO via OSC, actually the field is given to the root node
	GUI *						fGUI;		///< a GUI pointer to support updateAllGuis(), required for bi-directionnal OSC

	private:
		std::string addressFirst (const std::string& address) const;
		std::string addressTail  (const std::string& address) const;

	public:
				 FaustFactory(GUI* ui, OSCIO * io=0); // : fIO(io), fGUI(ui) {}
		virtual ~FaustFactory(); // {}

		template <typename C> void addnode (const char* label, C* zone, C init, C min, C max, bool initZone);
		template <typename C> void addAlias (const std::string& fullpath, C* zone, C imin, C imax, C init, C min, C max, const char* label);
		void addAlias (const char* alias, const char* address, float imin, float imax, float omin, float omax);
		void opengroup (const char* label);
		void closegroup ();

		SRootNode		root() const; //	{ return fRoot; }
};

/**
 * Add a node to the OSC UI tree in the current group at the top of the stack 
 */
template <typename C> void FaustFactory::addnode (const char* label, C* zone, C init, C min, C max, bool initZone) 
{
//	SMessageDriven top = fNodes.size() ? fNodes.top() : fRoot;
	SMessageDriven top;
	if (fNodes.size()) top = fNodes.top();
	if (top) {
		std::string prefix = top->getOSCAddress();
		top->add( FaustNode<C>::create (label, zone, init, min, max, prefix.c_str(), fGUI, initZone));
	}
}

/**
 * Add an alias (actually stored and handled at root node level
 */
template <typename C> void FaustFactory::addAlias (const std::string& fullpath, C* zone, C imin, C imax, C init, C min, C max, const char* label)
{
	std::istringstream 	ss(fullpath);
	std::string 		realpath; 

	ss >> realpath >> imin >> imax;
	SMessageDriven top = fNodes.top();
	if (top ) {
		std::string target = top->getOSCAddress() + "/" + label;
		addAlias (realpath.c_str(), target.c_str(), float(imin), float(imax), float(min), float(max));
	}
}

} // end namespoace

#endif

class GUI;

typedef void (*ErrorCallback)(void*);  

namespace oscfaust
{

class OSCIO;
class OSCSetup;
class OSCRegexp;
    
//--------------------------------------------------------------------------
/*!
	\brief the main Faust OSC Lib API
	
	The OSCControler is essentially a glue between the memory representation (in charge of the FaustFactory),
	and the network services (in charge of OSCSetup).
*/
class OSCControler
{
	int fUDPPort, fUDPOut, fUPDErr;		// the udp ports numbers
	std::string		fDestAddress;		// the osc messages destination address, used at initialization only
										// to collect the address from the command line
	OSCSetup*		fOsc;				// the network manager (handles the udp sockets)
	OSCIO*			fIO;				// hack for OSC IO support (actually only relayed to the factory)
	FaustFactory *	fFactory;			// a factory to build the memory represetnatin

    bool            fInit;
    
	public:
		/*
			base udp port is chosen in an unassigned range from IANA PORT NUMBERS (last updated 2011-01-24)
			see at http://www.iana.org/assignments/port-numbers
			5507-5552  Unassigned
		*/
		enum { kUDPBasePort = 5510};
            
        OSCControler (int argc, char *argv[], GUI* ui, OSCIO* io = 0, ErrorCallback errCallback = NULL, void* arg = NULL, bool init = true);

        virtual ~OSCControler ();
	
		//--------------------------------------------------------------------------
		// addnode, opengroup and closegroup are simply relayed to the factory
		//--------------------------------------------------------------------------
		// Add a node in the current group (top of the group stack)
		template <typename T> void addnode (const char* label, T* zone, T init, T min, T max)
							{ fFactory->addnode (label, zone, init, min, max, fInit); }
		
		//--------------------------------------------------------------------------
		// This method is used for alias messages. The arguments imin and imax allow
		// to map incomming values from the alias input range to the actual range 
		template <typename T> void addAlias (const std::string& fullpath, T* zone, T imin, T imax, T init, T min, T max, const char* label)
							{ fFactory->addAlias (fullpath, zone, imin, imax, init, min, max, label); }

		void opengroup (const char* label)		{ fFactory->opengroup (label); }
		void closegroup ()						{ fFactory->closegroup(); }
	   
		//--------------------------------------------------------------------------
		void run ();				// starts the network services
		void quit ();				// stop the network services
		
		int	getUDPPort() const			{ return fUDPPort; }
		int	getUDPOut()	const			{ return fUDPOut; }
		int	getUDPErr()	const			{ return fUPDErr; }
		const char*	getDestAddress() const { return fDestAddress.c_str(); }
		const char*	getRootName() const;	// probably useless, introduced for UI extension experiments

    
//      By default, an osc interface emits all parameters. You can filter specific params dynamically.
        static std::vector<OSCRegexp*>     fFilteredPaths; // filtered paths will not be emitted
        static void addFilteredPath(std::string path);
        static bool isPathFiltered(std::string path);
        static void resetFilteredPaths();
    
		static float version();				// the Faust OSC library version number
		static const char* versionstr();	// the Faust OSC library version number as a string
		static bool	gXmit;				// a static variable to control the transmission of values
										// i.e. the use of the interface as a controler
};

}

#endif
#include <vector>

/******************************************************************************
*******************************************************************************

					OSC (Open Sound Control) USER INTERFACE

*******************************************************************************
*******************************************************************************/
/*

Note about the OSC addresses and the Faust UI names:
----------------------------------------------------
There are potential conflicts between the Faust UI objects naming scheme and 
the OSC address space. An OSC symbolic names is an ASCII string consisting of
printable characters other than the following:
	space 
#	number sign
*	asterisk
,	comma
/	forward
?	question mark
[	open bracket
]	close bracket
{	open curly brace
}	close curly brace

a simple solution to address the problem consists in replacing 
space or tabulation with '_' (underscore)
all the other osc excluded characters with '-' (hyphen)

This solution is implemented in the proposed OSC UI;
*/

///using namespace std;

//class oscfaust::OSCIO;

class OSCUI : public GUI 
{
     
	oscfaust::OSCControler*	fCtrl;
	std::vector<const char*> fAlias;
	
	const char* tr(const char* label) const;
	
	// add all accumulated alias
	void addalias(FAUSTFLOAT* zone, FAUSTFLOAT init, FAUSTFLOAT min, FAUSTFLOAT max, const char* label)
	{
		for (unsigned int i=0; i<fAlias.size(); i++) {
			fCtrl->addAlias(fAlias[i], zone, (FAUSTFLOAT)0, (FAUSTFLOAT)1, init, min, max, label);
		}
		fAlias.clear();
	}
	
 public:

    OSCUI(const char* /*applicationname*/, int argc, char *argv[], oscfaust::OSCIO* io=0, ErrorCallback errCallback = NULL, void* arg = NULL, bool init = true) : GUI() 
    { 
		fCtrl = new oscfaust::OSCControler(argc, argv, this, io, errCallback, arg, init); 
        //		fCtrl->opengroup(applicationname);
	}
    
	virtual ~OSCUI() { delete fCtrl; }
    
    // -- widget's layouts
    
  	virtual void openTabBox(const char* label) 			{ fCtrl->opengroup( tr(label)); }
	virtual void openHorizontalBox(const char* label) 	{ fCtrl->opengroup( tr(label)); }
	virtual void openVerticalBox(const char* label) 	{ fCtrl->opengroup( tr(label)); }
	virtual void closeBox() 							{ fCtrl->closegroup(); }

	
	// -- active widgets
	virtual void addButton(const char* label, FAUSTFLOAT* zone) 		{ const char* l= tr(label); addalias(zone, 0, 0, 1, l); fCtrl->addnode( l, zone, (FAUSTFLOAT)0, (FAUSTFLOAT)0, (FAUSTFLOAT)1); }
	virtual void addCheckButton(const char* label, FAUSTFLOAT* zone) 	{ const char* l= tr(label); addalias(zone, 0, 0, 1, l); fCtrl->addnode( l, zone, (FAUSTFLOAT)0, (FAUSTFLOAT)0, (FAUSTFLOAT)1); }
	virtual void addVerticalSlider(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT init, FAUSTFLOAT min, FAUSTFLOAT max, FAUSTFLOAT /*step*/)
																		{ const char* l= tr(label); addalias(zone, init, min, max, l); fCtrl->addnode( l, zone, init, min, max); }
	virtual void addHorizontalSlider(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT init, FAUSTFLOAT min, FAUSTFLOAT max, FAUSTFLOAT /*step*/)
																		{ const char* l= tr(label); addalias(zone, init, min, max, l); fCtrl->addnode( l, zone, init, min, max); }
	virtual void addNumEntry(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT init, FAUSTFLOAT min, FAUSTFLOAT max, FAUSTFLOAT /*step*/)
																		{ const char* l= tr(label); addalias(zone, init, min, max, l); fCtrl->addnode( l, zone, init, min, max); }
	
	// -- passive widgets
	
	virtual void addHorizontalBargraph(const char* /*label*/, FAUSTFLOAT* /*zone*/, FAUSTFLOAT /*min*/, FAUSTFLOAT /*max*/) {}
	virtual void addVerticalBargraph(const char* /*label*/, FAUSTFLOAT* /*zone*/, FAUSTFLOAT /*min*/, FAUSTFLOAT /*max*/) {}
		
	// -- metadata declarations
    
	virtual void declare(FAUSTFLOAT* , const char* key , const char* alias) 
	{ 
		if (strcasecmp(key,"OSC")==0) fAlias.push_back(alias);
	}

	virtual void show() {}

	void run()
    {
        fCtrl->run(); 
    }
	const char* getRootName()		{ return fCtrl->getRootName(); }
    int getUDPPort()                { return fCtrl->getUDPPort(); }
    int	getUDPOut()                 { return fCtrl->getUDPOut(); }
    int	getUDPErr()                 { return fCtrl->getUDPErr(); }
    const char* getDestAddress()    {return fCtrl->getDestAddress();}
};

const char* OSCUI::tr(const char* label) const
{
	static char buffer[1024];
	char * ptr = buffer; int n=1;
	while (*label && (n++ < 1024)) {
		switch (*label) {
			case ' ': case '	':
				*ptr++ = '_';
				break;
			case '#': case '*': case ',': case '/': case '?':
			case '[': case ']': case '{': case '}':
				*ptr++ = '_';
				break;
			default: 
				*ptr++ = *label;
		}
		label++;
	}
	*ptr = 0;
	return buffer;
}

#endif
#endif

#ifdef HTTPCTRL
/*
   Copyright (C) 2012 Grame - Lyon
   All rights reserved.
   Redistribution and use in source and binary forms, with or without
   modification, are permitted.
*/

#ifndef __httpdUI__
#define __httpdUI__

//#ifdef _WIN32
//#include "HTTPDControler.h"
//#include "UI.h"
//#else
/*

  Faust Project

  Copyright (C) 2012 Grame

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

  Grame Research Laboratory, 9 rue du Garet, 69001 Lyon - France
  research@grame.fr

*/

#ifndef __HTTPDControler__
#define __HTTPDControler__

#include <string>
#include <map>

namespace httpdfaust
{

class HTTPDSetup;
class JSONDesc;
class FaustFactory;
class jsonfactory;
class htmlfactory;

//--------------------------------------------------------------------------
/*!
	\brief the main Faust HTTPD Lib API
	
	The HTTPDControler is essentially a glue between the memory representation (in charge of the FaustFactory), 
	and the network services (in charge of HTTPDSetup).
*/
class HTTPDControler
{
	int fTCPPort;				// the tcp port number
	FaustFactory *	fFactory;	// a factory to build the memory representation
	jsonfactory*	fJson;
	htmlfactory*	fHtml;
	HTTPDSetup*		fHttpd;		// the network manager
	std::string		fHTML;		// the corresponding HTML page
	std::map<std::string, std::string>	fCurrentMeta;	// the current meta declarations 

    bool            fInit;
    
	public:
		/*
			base udp port is chosen in an unassigned range from IANA PORT NUMBERS (last updated 2011-01-24)
			see at http://www.iana.org/assignments/port-numbers
			5507-5552  Unassigned
		*/
		enum { kTCPBasePort = 5510};

				 HTTPDControler (int argc, char *argv[], const char* applicationname, bool init = true);
		virtual ~HTTPDControler ();
	
		//--------------------------------------------------------------------------
		// addnode, opengroup and closegroup are simply relayed to the factory
		//--------------------------------------------------------------------------
		template <typename C> void addnode (const char* type, const char* label, C* zone);
		template <typename C> void addnode (const char* type, const char* label, C* zone, C min, C max);
		template <typename C> void addnode (const char* type, const char* label, C* zone, C init, C min, C max, C step);
							  void declare (const char* key, const char* val ) { fCurrentMeta[key] = val; }

		void opengroup (const char* type, const char* label);
		void closegroup ();

		//--------------------------------------------------------------------------
		void run ();				// start the httpd server
		void quit ();				// stop the httpd server
		
		int	getTCPPort()			{ return fTCPPort; }
        std::string getJSONInterface();
        void        setInputs(int numInputs);
        void        setOutputs(int numOutputs);

		static float version();				// the Faust httpd library version number
		static const char* versionstr();	// the Faust httpd library version number as a string
};

}

#endif
//#endif
/******************************************************************************
*******************************************************************************

					HTTPD USER INTERFACE

*******************************************************************************
*******************************************************************************/
/*

Note about URLs and the Faust UI names:
----------------------------------------------------
Characters in a url could be:
1. Reserved: ; / ? : @ & = + $ ,
   These characters delimit URL parts.
2. Unreserved: alphanum - _ . ! ~ * ' ( )
   These characters have no special meaning and can be used as is.
3. Excluded: control characters, space, < > # % ", { } | \ ^ [ ] `

To solve potential conflicts between the Faust UI objects naming scheme and
the URL allowed characters, the reserved and excluded characters are replaced
with '-' (hyphen).
Space or tabulation are replaced with '_' (underscore)
*/

//using namespace std;

class httpdUI : public UI 
{
	httpdfaust::HTTPDControler*	fCtrl;	
	const char* tr(const char* label) const;

 public:
		
	httpdUI(const char* applicationname, int inputs, int outputs, int argc, char *argv[], bool init = true) 
    { 
		fCtrl = new httpdfaust::HTTPDControler(argc, argv, applicationname, init); 
        fCtrl->setInputs(inputs);
        fCtrl->setOutputs(outputs);
	}
	
	virtual ~httpdUI() { delete fCtrl; }
		
    // -- widget's layouts
	virtual void openTabBox(const char* label) 			{ fCtrl->opengroup( "tgroup", tr(label)); }
	virtual void openHorizontalBox(const char* label) 	{ fCtrl->opengroup( "hgroup", tr(label)); }
	virtual void openVerticalBox(const char* label) 	{ fCtrl->opengroup( "vgroup", tr(label)); }
	virtual void closeBox() 							{ fCtrl->closegroup(); }
	
	// -- active widgets
	virtual void addButton(const char* label, FAUSTFLOAT* zone)			{ fCtrl->addnode( "button", tr(label), zone); }
	virtual void addCheckButton(const char* label, FAUSTFLOAT* zone)	{ fCtrl->addnode( "checkbox", tr(label), zone); }
	virtual void addVerticalSlider(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT init, FAUSTFLOAT min, FAUSTFLOAT max, FAUSTFLOAT step)
									{ fCtrl->addnode( "vslider", tr(label), zone, init, min, max, step); }
	virtual void addHorizontalSlider(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT init, FAUSTFLOAT min, FAUSTFLOAT max, FAUSTFLOAT step) 	
									{ fCtrl->addnode( "hslider", tr(label), zone, init, min, max, step); }
	virtual void addNumEntry(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT init, FAUSTFLOAT min, FAUSTFLOAT max, FAUSTFLOAT step) 			
									{ fCtrl->addnode( "nentry", tr(label), zone, init, min, max, step); }
	
	// -- passive widgets	
	virtual void addHorizontalBargraph(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT min, FAUSTFLOAT max) 
									{ fCtrl->addnode( "hbargraph", tr(label), zone, min, max); }
	virtual void addVerticalBargraph(const char* label, FAUSTFLOAT* zone, FAUSTFLOAT min, FAUSTFLOAT max)
									{ fCtrl->addnode( "vbargraph", tr(label), zone, min, max); }
	
    virtual void declare (FAUSTFLOAT*, const char* key, const char* val) { fCtrl->declare(key, val); }

	void run()						{ fCtrl->run(); }
	int getTCPPort()                { return fCtrl->getTCPPort(); }
    
    std::string getJSONInterface(){ return fCtrl->getJSONInterface(); }

};
					
const char* httpdUI::tr(const char* label) const
{
	static char buffer[1024];
	char * ptr = buffer; int n=1;
	while (*label && (n++ < 1024)) {
		switch (*label) {
			case ' ': case '	':
				*ptr++ = '_';
				break;
			case ';': case '/': case '?': case ':': case '@': 
			case '&': case '=': case '+': case '$': case ',':
			case '<': case '>': case '#': case '%': case '"': 
			case '{': case '}': case '|': case '\\': case '^': 
			case '[': case ']': case '`':
				*ptr++ = '_';
				break;
			default: 
				*ptr++ = *label;
		}
		label++;
	}
	*ptr = 0;
	return buffer;
}

#endif
#endif


/**************************BEGIN USER SECTION **************************/
/******************************************************************************
*******************************************************************************

							       VECTOR INTRINSICS

*******************************************************************************
*******************************************************************************/



#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif  


#ifndef FAUSTCLASS 
#define FAUSTCLASS mydsp
#endif

class mydsp : public dsp {
  private:
	class SIG0 {
	  private:
		int 	fSamplingFreq;
	  public:
		int getNumInputs() 	{ return 0; }
		int getNumOutputs() 	{ return 1; }
		void init(int samplingFreq) {
			fSamplingFreq = samplingFreq;
		}
		void fill (int count, float output[]) {
			for (int i=0; i<count; i++) {
				output[i] = 0.0f;
			}
		}
	};


	FAUSTFLOAT 	fslider0;
	int 	iVec0[2];
	int 	iRec1[2];
	FAUSTFLOAT 	fslider1;
	int 	iRec0[2];
	int 	iRec65[2];
	int 	iRec66[2];
	float 	ftbl0[480000];
	int 	iRec67[2];
	FAUSTFLOAT 	fslider2;
	int 	iRec68[2];
	int 	iRec69[2];
	int 	iRec70[2];
	int 	iRec71[2];
	int 	iRec72[2];
	int 	iRec73[2];
	int 	iRec74[2];
	int 	iRec75[2];
	int 	iRec76[2];
	int 	iRec77[2];
	int 	iRec78[2];
	int 	iRec79[2];
	int 	iRec80[2];
	int 	iRec81[2];
	int 	iRec82[2];
	int 	iRec83[2];
	int 	iRec84[2];
	int 	iRec85[2];
	int 	iRec86[2];
	int 	iRec87[2];
	int 	iRec88[2];
	int 	iRec89[2];
	int 	iRec90[2];
	int 	iRec91[2];
	int 	iRec92[2];
	int 	iRec93[2];
	int 	iRec94[2];
	int 	iRec95[2];
	int 	iRec96[2];
	int 	iRec97[2];
	int 	iRec98[2];
	int 	iRec99[2];
	int 	iRec100[2];
	int 	iRec101[2];
	int 	iRec102[2];
	int 	iRec103[2];
	int 	iRec104[2];
	int 	iRec105[2];
	int 	iRec106[2];
	int 	iRec107[2];
	int 	iRec108[2];
	int 	iRec109[2];
	int 	iRec110[2];
	int 	iRec111[2];
	int 	iRec112[2];
	int 	iRec113[2];
	int 	iRec114[2];
	int 	iRec115[2];
	int 	iRec116[2];
	int 	iRec117[2];
	int 	iRec118[2];
	int 	iRec119[2];
	int 	iRec120[2];
	int 	iRec121[2];
	int 	iRec122[2];
	int 	iRec123[2];
	int 	iRec124[2];
	int 	iRec125[2];
	int 	iRec126[2];
	int 	iRec127[2];
	int 	iRec128[2];
	int 	iRec129[2];
	int 	iRec130[2];
	int 	iRec131[2];
	int 	iRec132[2];
	int 	iRec133[2];
	int 	iRec134[2];
	int 	iRec135[2];
	int 	iRec136[2];
	int 	iRec137[2];
	int 	iRec138[2];
	int 	iRec139[2];
	int 	iRec140[2];
	int 	iRec141[2];
	int 	iRec142[2];
	int 	iRec143[2];
	int 	iRec144[2];
	int 	iRec145[2];
	int 	iRec146[2];
	int 	iRec147[2];
	int 	iRec148[2];
	int 	iRec149[2];
	int 	iRec150[2];
	int 	iRec151[2];
	int 	iRec152[2];
	int 	iRec153[2];
	int 	iRec154[2];
	int 	iRec155[2];
	int 	iRec156[2];
	int 	iRec157[2];
	int 	iRec158[2];
	int 	iRec159[2];
	int 	iRec160[2];
	int 	iRec161[2];
	int 	iRec162[2];
	int 	iRec163[2];
	int 	iRec164[2];
	int 	iRec165[2];
	int 	iRec166[2];
	int 	iRec167[2];
	int 	iRec168[2];
	int 	iRec169[2];
	int 	iRec170[2];
	int 	iRec171[2];
	int 	iRec172[2];
	int 	iRec173[2];
	int 	iRec174[2];
	int 	iRec175[2];
	int 	iRec176[2];
	int 	iRec177[2];
	int 	iRec178[2];
	int 	iRec179[2];
	int 	iRec180[2];
	int 	iRec181[2];
	int 	iRec182[2];
	int 	iRec183[2];
	int 	iRec184[2];
	int 	iRec185[2];
	int 	iRec186[2];
	int 	iRec187[2];
	int 	iRec188[2];
	int 	iRec189[2];
	int 	iRec190[2];
	int 	iRec191[2];
	int 	iRec192[2];
	int 	iRec193[2];
  public:
	static void metadata(Meta* m) 	{ 
		m->declare("name", "Granulator");
		m->declare("author", "Mayank Sanganeria");
		m->declare("version", "1.0");
	}

	virtual int getNumInputs() 	{ return 1; }
	virtual int getNumOutputs() 	{ return 2; }
	static void classInit(int samplingFreq) {
	}
	virtual void instanceInit(int samplingFreq) {
		fSamplingFreq = samplingFreq;
		fslider0 = 0.1f;
		for (int i=0; i<2; i++) iVec0[i] = 0;
		for (int i=0; i<2; i++) iRec1[i] = 0;
		fslider1 = 1e+01f;
		for (int i=0; i<2; i++) iRec0[i] = 0;
		for (int i=0; i<2; i++) iRec65[i] = 0;
		for (int i=0; i<2; i++) iRec66[i] = 0;
		SIG0 sig0;
		sig0.init(samplingFreq);
		sig0.fill(480000,ftbl0);
		for (int i=0; i<2; i++) iRec67[i] = 0;
		fslider2 = 1.0f;
		for (int i=0; i<2; i++) iRec68[i] = 0;
		for (int i=0; i<2; i++) iRec69[i] = 0;
		for (int i=0; i<2; i++) iRec70[i] = 0;
		for (int i=0; i<2; i++) iRec71[i] = 0;
		for (int i=0; i<2; i++) iRec72[i] = 0;
		for (int i=0; i<2; i++) iRec73[i] = 0;
		for (int i=0; i<2; i++) iRec74[i] = 0;
		for (int i=0; i<2; i++) iRec75[i] = 0;
		for (int i=0; i<2; i++) iRec76[i] = 0;
		for (int i=0; i<2; i++) iRec77[i] = 0;
		for (int i=0; i<2; i++) iRec78[i] = 0;
		for (int i=0; i<2; i++) iRec79[i] = 0;
		for (int i=0; i<2; i++) iRec80[i] = 0;
		for (int i=0; i<2; i++) iRec81[i] = 0;
		for (int i=0; i<2; i++) iRec82[i] = 0;
		for (int i=0; i<2; i++) iRec83[i] = 0;
		for (int i=0; i<2; i++) iRec84[i] = 0;
		for (int i=0; i<2; i++) iRec85[i] = 0;
		for (int i=0; i<2; i++) iRec86[i] = 0;
		for (int i=0; i<2; i++) iRec87[i] = 0;
		for (int i=0; i<2; i++) iRec88[i] = 0;
		for (int i=0; i<2; i++) iRec89[i] = 0;
		for (int i=0; i<2; i++) iRec90[i] = 0;
		for (int i=0; i<2; i++) iRec91[i] = 0;
		for (int i=0; i<2; i++) iRec92[i] = 0;
		for (int i=0; i<2; i++) iRec93[i] = 0;
		for (int i=0; i<2; i++) iRec94[i] = 0;
		for (int i=0; i<2; i++) iRec95[i] = 0;
		for (int i=0; i<2; i++) iRec96[i] = 0;
		for (int i=0; i<2; i++) iRec97[i] = 0;
		for (int i=0; i<2; i++) iRec98[i] = 0;
		for (int i=0; i<2; i++) iRec99[i] = 0;
		for (int i=0; i<2; i++) iRec100[i] = 0;
		for (int i=0; i<2; i++) iRec101[i] = 0;
		for (int i=0; i<2; i++) iRec102[i] = 0;
		for (int i=0; i<2; i++) iRec103[i] = 0;
		for (int i=0; i<2; i++) iRec104[i] = 0;
		for (int i=0; i<2; i++) iRec105[i] = 0;
		for (int i=0; i<2; i++) iRec106[i] = 0;
		for (int i=0; i<2; i++) iRec107[i] = 0;
		for (int i=0; i<2; i++) iRec108[i] = 0;
		for (int i=0; i<2; i++) iRec109[i] = 0;
		for (int i=0; i<2; i++) iRec110[i] = 0;
		for (int i=0; i<2; i++) iRec111[i] = 0;
		for (int i=0; i<2; i++) iRec112[i] = 0;
		for (int i=0; i<2; i++) iRec113[i] = 0;
		for (int i=0; i<2; i++) iRec114[i] = 0;
		for (int i=0; i<2; i++) iRec115[i] = 0;
		for (int i=0; i<2; i++) iRec116[i] = 0;
		for (int i=0; i<2; i++) iRec117[i] = 0;
		for (int i=0; i<2; i++) iRec118[i] = 0;
		for (int i=0; i<2; i++) iRec119[i] = 0;
		for (int i=0; i<2; i++) iRec120[i] = 0;
		for (int i=0; i<2; i++) iRec121[i] = 0;
		for (int i=0; i<2; i++) iRec122[i] = 0;
		for (int i=0; i<2; i++) iRec123[i] = 0;
		for (int i=0; i<2; i++) iRec124[i] = 0;
		for (int i=0; i<2; i++) iRec125[i] = 0;
		for (int i=0; i<2; i++) iRec126[i] = 0;
		for (int i=0; i<2; i++) iRec127[i] = 0;
		for (int i=0; i<2; i++) iRec128[i] = 0;
		for (int i=0; i<2; i++) iRec129[i] = 0;
		for (int i=0; i<2; i++) iRec130[i] = 0;
		for (int i=0; i<2; i++) iRec131[i] = 0;
		for (int i=0; i<2; i++) iRec132[i] = 0;
		for (int i=0; i<2; i++) iRec133[i] = 0;
		for (int i=0; i<2; i++) iRec134[i] = 0;
		for (int i=0; i<2; i++) iRec135[i] = 0;
		for (int i=0; i<2; i++) iRec136[i] = 0;
		for (int i=0; i<2; i++) iRec137[i] = 0;
		for (int i=0; i<2; i++) iRec138[i] = 0;
		for (int i=0; i<2; i++) iRec139[i] = 0;
		for (int i=0; i<2; i++) iRec140[i] = 0;
		for (int i=0; i<2; i++) iRec141[i] = 0;
		for (int i=0; i<2; i++) iRec142[i] = 0;
		for (int i=0; i<2; i++) iRec143[i] = 0;
		for (int i=0; i<2; i++) iRec144[i] = 0;
		for (int i=0; i<2; i++) iRec145[i] = 0;
		for (int i=0; i<2; i++) iRec146[i] = 0;
		for (int i=0; i<2; i++) iRec147[i] = 0;
		for (int i=0; i<2; i++) iRec148[i] = 0;
		for (int i=0; i<2; i++) iRec149[i] = 0;
		for (int i=0; i<2; i++) iRec150[i] = 0;
		for (int i=0; i<2; i++) iRec151[i] = 0;
		for (int i=0; i<2; i++) iRec152[i] = 0;
		for (int i=0; i<2; i++) iRec153[i] = 0;
		for (int i=0; i<2; i++) iRec154[i] = 0;
		for (int i=0; i<2; i++) iRec155[i] = 0;
		for (int i=0; i<2; i++) iRec156[i] = 0;
		for (int i=0; i<2; i++) iRec157[i] = 0;
		for (int i=0; i<2; i++) iRec158[i] = 0;
		for (int i=0; i<2; i++) iRec159[i] = 0;
		for (int i=0; i<2; i++) iRec160[i] = 0;
		for (int i=0; i<2; i++) iRec161[i] = 0;
		for (int i=0; i<2; i++) iRec162[i] = 0;
		for (int i=0; i<2; i++) iRec163[i] = 0;
		for (int i=0; i<2; i++) iRec164[i] = 0;
		for (int i=0; i<2; i++) iRec165[i] = 0;
		for (int i=0; i<2; i++) iRec166[i] = 0;
		for (int i=0; i<2; i++) iRec167[i] = 0;
		for (int i=0; i<2; i++) iRec168[i] = 0;
		for (int i=0; i<2; i++) iRec169[i] = 0;
		for (int i=0; i<2; i++) iRec170[i] = 0;
		for (int i=0; i<2; i++) iRec171[i] = 0;
		for (int i=0; i<2; i++) iRec172[i] = 0;
		for (int i=0; i<2; i++) iRec173[i] = 0;
		for (int i=0; i<2; i++) iRec174[i] = 0;
		for (int i=0; i<2; i++) iRec175[i] = 0;
		for (int i=0; i<2; i++) iRec176[i] = 0;
		for (int i=0; i<2; i++) iRec177[i] = 0;
		for (int i=0; i<2; i++) iRec178[i] = 0;
		for (int i=0; i<2; i++) iRec179[i] = 0;
		for (int i=0; i<2; i++) iRec180[i] = 0;
		for (int i=0; i<2; i++) iRec181[i] = 0;
		for (int i=0; i<2; i++) iRec182[i] = 0;
		for (int i=0; i<2; i++) iRec183[i] = 0;
		for (int i=0; i<2; i++) iRec184[i] = 0;
		for (int i=0; i<2; i++) iRec185[i] = 0;
		for (int i=0; i<2; i++) iRec186[i] = 0;
		for (int i=0; i<2; i++) iRec187[i] = 0;
		for (int i=0; i<2; i++) iRec188[i] = 0;
		for (int i=0; i<2; i++) iRec189[i] = 0;
		for (int i=0; i<2; i++) iRec190[i] = 0;
		for (int i=0; i<2; i++) iRec191[i] = 0;
		for (int i=0; i<2; i++) iRec192[i] = 0;
		for (int i=0; i<2; i++) iRec193[i] = 0;
	}
	virtual void init(int samplingFreq) {
		classInit(samplingFreq);
		instanceInit(samplingFreq);
	}
	virtual void buildUserInterface(UI* interface) {
		interface->openVerticalBox("0x00");
		interface->addHorizontalSlider("delay length", &fslider1, 1e+01f, 0.5f, 1e+01f, 0.1f);
		interface->addHorizontalSlider("grain density", &fslider2, 1.0f, 1.0f, 64.0f, 1.0f);
		interface->addHorizontalSlider("grain length", &fslider0, 0.1f, 0.01f, 0.5f, 0.01f);
		interface->closeBox();
	}
	virtual void compute (int count, FAUSTFLOAT** input, FAUSTFLOAT** output) {
		int 	iSlow0 = int((fSamplingFreq * float(fslider0)));
		int 	iSlow1 = int((fSamplingFreq * float(fslider1)));
		float 	fSlow2 = (0.5f * iSlow1);
		int 	iSlow3 = (iSlow0 - 1);
		float 	fSlow4 = (6.28318f / float(iSlow3));
		float 	fSlow5 = (1.0f / float(iSlow3));
		float 	fSlow6 = float(fslider2);
		int 	iSlow7 = (62 < fSlow6);
		int 	iSlow8 = (60 < fSlow6);
		int 	iSlow9 = (58 < fSlow6);
		int 	iSlow10 = (56 < fSlow6);
		int 	iSlow11 = (54 < fSlow6);
		int 	iSlow12 = (52 < fSlow6);
		int 	iSlow13 = (50 < fSlow6);
		int 	iSlow14 = (48 < fSlow6);
		int 	iSlow15 = (46 < fSlow6);
		int 	iSlow16 = (44 < fSlow6);
		int 	iSlow17 = (42 < fSlow6);
		int 	iSlow18 = (40 < fSlow6);
		int 	iSlow19 = (38 < fSlow6);
		int 	iSlow20 = (36 < fSlow6);
		int 	iSlow21 = (34 < fSlow6);
		int 	iSlow22 = (32 < fSlow6);
		int 	iSlow23 = (30 < fSlow6);
		int 	iSlow24 = (28 < fSlow6);
		int 	iSlow25 = (26 < fSlow6);
		int 	iSlow26 = (24 < fSlow6);
		int 	iSlow27 = (22 < fSlow6);
		int 	iSlow28 = (20 < fSlow6);
		int 	iSlow29 = (18 < fSlow6);
		int 	iSlow30 = (16 < fSlow6);
		int 	iSlow31 = (14 < fSlow6);
		int 	iSlow32 = (12 < fSlow6);
		int 	iSlow33 = (10 < fSlow6);
		int 	iSlow34 = (8 < fSlow6);
		int 	iSlow35 = (6 < fSlow6);
		int 	iSlow36 = (4 < fSlow6);
		int 	iSlow37 = (2 < fSlow6);
		int 	iSlow38 = (0 < fSlow6);
		float 	fSlow39 = (1.0f / fSlow6);
		int 	iSlow40 = (63 < fSlow6);
		int 	iSlow41 = (61 < fSlow6);
		int 	iSlow42 = (59 < fSlow6);
		int 	iSlow43 = (57 < fSlow6);
		int 	iSlow44 = (55 < fSlow6);
		int 	iSlow45 = (53 < fSlow6);
		int 	iSlow46 = (51 < fSlow6);
		int 	iSlow47 = (49 < fSlow6);
		int 	iSlow48 = (47 < fSlow6);
		int 	iSlow49 = (45 < fSlow6);
		int 	iSlow50 = (43 < fSlow6);
		int 	iSlow51 = (41 < fSlow6);
		int 	iSlow52 = (39 < fSlow6);
		int 	iSlow53 = (37 < fSlow6);
		int 	iSlow54 = (35 < fSlow6);
		int 	iSlow55 = (33 < fSlow6);
		int 	iSlow56 = (31 < fSlow6);
		int 	iSlow57 = (29 < fSlow6);
		int 	iSlow58 = (27 < fSlow6);
		int 	iSlow59 = (25 < fSlow6);
		int 	iSlow60 = (23 < fSlow6);
		int 	iSlow61 = (21 < fSlow6);
		int 	iSlow62 = (19 < fSlow6);
		int 	iSlow63 = (17 < fSlow6);
		int 	iSlow64 = (15 < fSlow6);
		int 	iSlow65 = (13 < fSlow6);
		int 	iSlow66 = (11 < fSlow6);
		int 	iSlow67 = (9 < fSlow6);
		int 	iSlow68 = (7 < fSlow6);
		int 	iSlow69 = (5 < fSlow6);
		int 	iSlow70 = (3 < fSlow6);
		int 	iSlow71 = (1 < fSlow6);
		FAUSTFLOAT* input0 = input[0];
		FAUSTFLOAT* output0 = output[0];
		FAUSTFLOAT* output1 = output[1];
		for (int i=0; i<count; i++) {
			iVec0[0] = 1;
			int iTemp0 = (1103515245 * (12345 + iRec1[1]));
			int iTemp1 = (1103515245 * (12345 + iTemp0));
			int iTemp2 = (1103515245 * (12345 + iTemp1));
			int iTemp3 = (1103515245 * (12345 + iTemp2));
			int iTemp4 = (1103515245 * (12345 + iTemp3));
			int iTemp5 = (1103515245 * (12345 + iTemp4));
			int iTemp6 = (1103515245 * (12345 + iTemp5));
			int iTemp7 = (1103515245 * (12345 + iTemp6));
			int iTemp8 = (1103515245 * (12345 + iTemp7));
			int iTemp9 = (1103515245 * (12345 + iTemp8));
			int iTemp10 = (1103515245 * (12345 + iTemp9));
			int iTemp11 = (1103515245 * (12345 + iTemp10));
			int iTemp12 = (1103515245 * (12345 + iTemp11));
			int iTemp13 = (1103515245 * (12345 + iTemp12));
			int iTemp14 = (1103515245 * (12345 + iTemp13));
			int iTemp15 = (1103515245 * (12345 + iTemp14));
			int iTemp16 = (1103515245 * (12345 + iTemp15));
			int iTemp17 = (1103515245 * (12345 + iTemp16));
			int iTemp18 = (1103515245 * (12345 + iTemp17));
			int iTemp19 = (1103515245 * (12345 + iTemp18));
			int iTemp20 = (1103515245 * (12345 + iTemp19));
			int iTemp21 = (1103515245 * (12345 + iTemp20));
			int iTemp22 = (1103515245 * (12345 + iTemp21));
			int iTemp23 = (1103515245 * (12345 + iTemp22));
			int iTemp24 = (1103515245 * (12345 + iTemp23));
			int iTemp25 = (1103515245 * (12345 + iTemp24));
			int iTemp26 = (1103515245 * (12345 + iTemp25));
			int iTemp27 = (1103515245 * (12345 + iTemp26));
			int iTemp28 = (1103515245 * (12345 + iTemp27));
			int iTemp29 = (1103515245 * (12345 + iTemp28));
			int iTemp30 = (1103515245 * (12345 + iTemp29));
			int iTemp31 = (1103515245 * (12345 + iTemp30));
			int iTemp32 = (1103515245 * (12345 + iTemp31));
			int iTemp33 = (1103515245 * (12345 + iTemp32));
			int iTemp34 = (1103515245 * (12345 + iTemp33));
			int iTemp35 = (1103515245 * (12345 + iTemp34));
			int iTemp36 = (1103515245 * (12345 + iTemp35));
			int iTemp37 = (1103515245 * (12345 + iTemp36));
			int iTemp38 = (1103515245 * (12345 + iTemp37));
			int iTemp39 = (1103515245 * (12345 + iTemp38));
			int iTemp40 = (1103515245 * (12345 + iTemp39));
			int iTemp41 = (1103515245 * (12345 + iTemp40));
			int iTemp42 = (1103515245 * (12345 + iTemp41));
			int iTemp43 = (1103515245 * (12345 + iTemp42));
			int iTemp44 = (1103515245 * (12345 + iTemp43));
			int iTemp45 = (1103515245 * (12345 + iTemp44));
			int iTemp46 = (1103515245 * (12345 + iTemp45));
			int iTemp47 = (1103515245 * (12345 + iTemp46));
			int iTemp48 = (1103515245 * (12345 + iTemp47));
			int iTemp49 = (1103515245 * (12345 + iTemp48));
			int iTemp50 = (1103515245 * (12345 + iTemp49));
			int iTemp51 = (1103515245 * (12345 + iTemp50));
			int iTemp52 = (1103515245 * (12345 + iTemp51));
			int iTemp53 = (1103515245 * (12345 + iTemp52));
			int iTemp54 = (1103515245 * (12345 + iTemp53));
			int iTemp55 = (1103515245 * (12345 + iTemp54));
			int iTemp56 = (1103515245 * (12345 + iTemp55));
			int iTemp57 = (1103515245 * (12345 + iTemp56));
			int iTemp58 = (1103515245 * (12345 + iTemp57));
			int iTemp59 = (1103515245 * (12345 + iTemp58));
			int iTemp60 = (1103515245 * (12345 + iTemp59));
			int iTemp61 = (1103515245 * (12345 + iTemp60));
			int iTemp62 = (1103515245 * (12345 + iTemp61));
			int iTemp63 = (1103515245 * (12345 + iTemp62));
			iRec1[0] = (1103515245 * (12345 + iTemp63));
			int 	iRec2 = iTemp63;
			int 	iRec3 = iTemp62;
			int 	iRec4 = iTemp61;
			int 	iRec5 = iTemp60;
			int 	iRec6 = iTemp59;
			int 	iRec7 = iTemp58;
			int 	iRec8 = iTemp57;
			int 	iRec9 = iTemp56;
			int 	iRec10 = iTemp55;
			int 	iRec11 = iTemp54;
			int 	iRec12 = iTemp53;
			int 	iRec13 = iTemp52;
			int 	iRec14 = iTemp51;
			int 	iRec15 = iTemp50;
			int 	iRec16 = iTemp49;
			int 	iRec17 = iTemp48;
			int 	iRec18 = iTemp47;
			int 	iRec19 = iTemp46;
			int 	iRec20 = iTemp45;
			int 	iRec21 = iTemp44;
			int 	iRec22 = iTemp43;
			int 	iRec23 = iTemp42;
			int 	iRec24 = iTemp41;
			int 	iRec25 = iTemp40;
			int 	iRec26 = iTemp39;
			int 	iRec27 = iTemp38;
			int 	iRec28 = iTemp37;
			int 	iRec29 = iTemp36;
			int 	iRec30 = iTemp35;
			int 	iRec31 = iTemp34;
			int 	iRec32 = iTemp33;
			int 	iRec33 = iTemp32;
			int 	iRec34 = iTemp31;
			int 	iRec35 = iTemp30;
			int 	iRec36 = iTemp29;
			int 	iRec37 = iTemp28;
			int 	iRec38 = iTemp27;
			int 	iRec39 = iTemp26;
			int 	iRec40 = iTemp25;
			int 	iRec41 = iTemp24;
			int 	iRec42 = iTemp23;
			int 	iRec43 = iTemp22;
			int 	iRec44 = iTemp21;
			int 	iRec45 = iTemp20;
			int 	iRec46 = iTemp19;
			int 	iRec47 = iTemp18;
			int 	iRec48 = iTemp17;
			int 	iRec49 = iTemp16;
			int 	iRec50 = iTemp15;
			int 	iRec51 = iTemp14;
			int 	iRec52 = iTemp13;
			int 	iRec53 = iTemp12;
			int 	iRec54 = iTemp11;
			int 	iRec55 = iTemp10;
			int 	iRec56 = iTemp9;
			int 	iRec57 = iTemp8;
			int 	iRec58 = iTemp7;
			int 	iRec59 = iTemp6;
			int 	iRec60 = iTemp5;
			int 	iRec61 = iTemp4;
			int 	iRec62 = iTemp3;
			int 	iRec63 = iTemp2;
			int 	iRec64 = iTemp1;
			int iTemp64 = int((fSlow2 * (1 + (2.3283064370807974e-10f * iRec63))));
			int iTemp65 = (1 - iVec0[1]);
			iRec0[0] = ((iVec0[1] * iRec0[1]) + (iTemp65 * iTemp64));
			iRec65[0] = ((1 + iRec65[1]) % iSlow0);
			int iTemp66 = ((iRec65[0] + int(iRec0[0])) % iSlow0);
			int iTemp67 = int((fSlow5 * iTemp66));
			iRec66[0] = ((iRec66[1] * (1 - iTemp67)) + (iTemp64 * iTemp67));
			iRec67[0] = ((1 + iRec67[1]) % iSlow1);
			ftbl0[(iRec67[0] % iSlow1)] = (float)input0[i];
			int iTemp68 = int((fSlow2 * (1 + (2.3283064370807974e-10f * iRec61))));
			iRec68[0] = ((iVec0[1] * iRec68[1]) + (iTemp65 * iTemp68));
			int iTemp69 = ((iRec65[0] + int(iRec68[0])) % iSlow0);
			int iTemp70 = int((fSlow5 * iTemp69));
			iRec69[0] = ((iRec69[1] * (1 - iTemp70)) + (iTemp68 * iTemp70));
			int iTemp71 = int((fSlow2 * (1 + (2.3283064370807974e-10f * iRec59))));
			iRec70[0] = ((iVec0[1] * iRec70[1]) + (iTemp65 * iTemp71));
			int iTemp72 = ((iRec65[0] + int(iRec70[0])) % iSlow0);
			int iTemp73 = int((fSlow5 * iTemp72));
			iRec71[0] = ((iRec71[1] * (1 - iTemp73)) + (iTemp71 * iTemp73));
			int iTemp74 = int((fSlow2 * (1 + (2.3283064370807974e-10f * iRec57))));
			iRec72[0] = ((iVec0[1] * iRec72[1]) + (iTemp65 * iTemp74));
			int iTemp75 = ((iRec65[0] + int(iRec72[0])) % iSlow0);
			int iTemp76 = int((fSlow5 * iTemp75));
			iRec73[0] = ((iRec73[1] * (1 - iTemp76)) + (iTemp74 * iTemp76));
			int iTemp77 = int((fSlow2 * (1 + (2.3283064370807974e-10f * iRec55))));
			iRec74[0] = ((iVec0[1] * iRec74[1]) + (iTemp65 * iTemp77));
			int iTemp78 = ((iRec65[0] + int(iRec74[0])) % iSlow0);
			int iTemp79 = int((fSlow5 * iTemp78));
			iRec75[0] = ((iRec75[1] * (1 - iTemp79)) + (iTemp77 * iTemp79));
			int iTemp80 = int((fSlow2 * (1 + (2.3283064370807974e-10f * iRec53))));
			iRec76[0] = ((iVec0[1] * iRec76[1]) + (iTemp65 * iTemp80));
			int iTemp81 = ((iRec65[0] + int(iRec76[0])) % iSlow0);
			int iTemp82 = int((fSlow5 * iTemp81));
			iRec77[0] = ((iRec77[1] * (1 - iTemp82)) + (iTemp80 * iTemp82));
			int iTemp83 = int((fSlow2 * (1 + (2.3283064370807974e-10f * iRec51))));
			iRec78[0] = ((iVec0[1] * iRec78[1]) + (iTemp65 * iTemp83));
			int iTemp84 = ((iRec65[0] + int(iRec78[0])) % iSlow0);
			int iTemp85 = int((fSlow5 * iTemp84));
			iRec79[0] = ((iRec79[1] * (1 - iTemp85)) + (iTemp83 * iTemp85));
			int iTemp86 = int((fSlow2 * (1 + (2.3283064370807974e-10f * iRec49))));
			iRec80[0] = ((iVec0[1] * iRec80[1]) + (iTemp65 * iTemp86));
			int iTemp87 = ((iRec65[0] + int(iRec80[0])) % iSlow0);
			int iTemp88 = int((fSlow5 * iTemp87));
			iRec81[0] = ((iRec81[1] * (1 - iTemp88)) + (iTemp86 * iTemp88));
			int iTemp89 = int((fSlow2 * (1 + (2.3283064370807974e-10f * iRec47))));
			iRec82[0] = ((iVec0[1] * iRec82[1]) + (iTemp65 * iTemp89));
			int iTemp90 = ((iRec65[0] + int(iRec82[0])) % iSlow0);
			int iTemp91 = int((fSlow5 * iTemp90));
			iRec83[0] = ((iRec83[1] * (1 - iTemp91)) + (iTemp89 * iTemp91));
			int iTemp92 = int((fSlow2 * (1 + (2.3283064370807974e-10f * iRec45))));
			iRec84[0] = ((iVec0[1] * iRec84[1]) + (iTemp65 * iTemp92));
			int iTemp93 = ((iRec65[0] + int(iRec84[0])) % iSlow0);
			int iTemp94 = int((fSlow5 * iTemp93));
			iRec85[0] = ((iRec85[1] * (1 - iTemp94)) + (iTemp92 * iTemp94));
			int iTemp95 = int((fSlow2 * (1 + (2.3283064370807974e-10f * iRec43))));
			iRec86[0] = ((iVec0[1] * iRec86[1]) + (iTemp65 * iTemp95));
			int iTemp96 = ((iRec65[0] + int(iRec86[0])) % iSlow0);
			int iTemp97 = int((fSlow5 * iTemp96));
			iRec87[0] = ((iRec87[1] * (1 - iTemp97)) + (iTemp95 * iTemp97));
			int iTemp98 = int((fSlow2 * (1 + (2.3283064370807974e-10f * iRec41))));
			iRec88[0] = ((iVec0[1] * iRec88[1]) + (iTemp65 * iTemp98));
			int iTemp99 = ((iRec65[0] + int(iRec88[0])) % iSlow0);
			int iTemp100 = int((fSlow5 * iTemp99));
			iRec89[0] = ((iRec89[1] * (1 - iTemp100)) + (iTemp98 * iTemp100));
			int iTemp101 = int((fSlow2 * (1 + (2.3283064370807974e-10f * iRec39))));
			iRec90[0] = ((iVec0[1] * iRec90[1]) + (iTemp65 * iTemp101));
			int iTemp102 = ((iRec65[0] + int(iRec90[0])) % iSlow0);
			int iTemp103 = int((fSlow5 * iTemp102));
			iRec91[0] = ((iRec91[1] * (1 - iTemp103)) + (iTemp101 * iTemp103));
			int iTemp104 = int((fSlow2 * (1 + (2.3283064370807974e-10f * iRec37))));
			iRec92[0] = ((iVec0[1] * iRec92[1]) + (iTemp65 * iTemp104));
			int iTemp105 = ((iRec65[0] + int(iRec92[0])) % iSlow0);
			int iTemp106 = int((fSlow5 * iTemp105));
			iRec93[0] = ((iRec93[1] * (1 - iTemp106)) + (iTemp104 * iTemp106));
			int iTemp107 = int((fSlow2 * (1 + (2.3283064370807974e-10f * iRec35))));
			iRec94[0] = ((iVec0[1] * iRec94[1]) + (iTemp65 * iTemp107));
			int iTemp108 = ((iRec65[0] + int(iRec94[0])) % iSlow0);
			int iTemp109 = int((fSlow5 * iTemp108));
			iRec95[0] = ((iRec95[1] * (1 - iTemp109)) + (iTemp107 * iTemp109));
			int iTemp110 = int((fSlow2 * (1 + (2.3283064370807974e-10f * iRec33))));
			iRec96[0] = ((iVec0[1] * iRec96[1]) + (iTemp65 * iTemp110));
			int iTemp111 = ((iRec65[0] + int(iRec96[0])) % iSlow0);
			int iTemp112 = int((fSlow5 * iTemp111));
			iRec97[0] = ((iRec97[1] * (1 - iTemp112)) + (iTemp110 * iTemp112));
			int iTemp113 = int((fSlow2 * (1 + (2.3283064370807974e-10f * iRec31))));
			iRec98[0] = ((iVec0[1] * iRec98[1]) + (iTemp65 * iTemp113));
			int iTemp114 = ((iRec65[0] + int(iRec98[0])) % iSlow0);
			int iTemp115 = int((fSlow5 * iTemp114));
			iRec99[0] = ((iRec99[1] * (1 - iTemp115)) + (iTemp113 * iTemp115));
			int iTemp116 = int((fSlow2 * (1 + (2.3283064370807974e-10f * iRec29))));
			iRec100[0] = ((iVec0[1] * iRec100[1]) + (iTemp65 * iTemp116));
			int iTemp117 = ((iRec65[0] + int(iRec100[0])) % iSlow0);
			int iTemp118 = int((fSlow5 * iTemp117));
			iRec101[0] = ((iRec101[1] * (1 - iTemp118)) + (iTemp116 * iTemp118));
			int iTemp119 = int((fSlow2 * (1 + (2.3283064370807974e-10f * iRec27))));
			iRec102[0] = ((iVec0[1] * iRec102[1]) + (iTemp65 * iTemp119));
			int iTemp120 = ((iRec65[0] + int(iRec102[0])) % iSlow0);
			int iTemp121 = int((fSlow5 * iTemp120));
			iRec103[0] = ((iRec103[1] * (1 - iTemp121)) + (iTemp119 * iTemp121));
			int iTemp122 = int((fSlow2 * (1 + (2.3283064370807974e-10f * iRec25))));
			iRec104[0] = ((iVec0[1] * iRec104[1]) + (iTemp65 * iTemp122));
			int iTemp123 = ((iRec65[0] + int(iRec104[0])) % iSlow0);
			int iTemp124 = int((fSlow5 * iTemp123));
			iRec105[0] = ((iRec105[1] * (1 - iTemp124)) + (iTemp122 * iTemp124));
			int iTemp125 = int((fSlow2 * (1 + (2.3283064370807974e-10f * iRec23))));
			iRec106[0] = ((iVec0[1] * iRec106[1]) + (iTemp65 * iTemp125));
			int iTemp126 = ((iRec65[0] + int(iRec106[0])) % iSlow0);
			int iTemp127 = int((fSlow5 * iTemp126));
			iRec107[0] = ((iRec107[1] * (1 - iTemp127)) + (iTemp125 * iTemp127));
			int iTemp128 = int((fSlow2 * (1 + (2.3283064370807974e-10f * iRec21))));
			iRec108[0] = ((iVec0[1] * iRec108[1]) + (iTemp65 * iTemp128));
			int iTemp129 = ((iRec65[0] + int(iRec108[0])) % iSlow0);
			int iTemp130 = int((fSlow5 * iTemp129));
			iRec109[0] = ((iRec109[1] * (1 - iTemp130)) + (iTemp128 * iTemp130));
			int iTemp131 = int((fSlow2 * (1 + (2.3283064370807974e-10f * iRec19))));
			iRec110[0] = ((iVec0[1] * iRec110[1]) + (iTemp65 * iTemp131));
			int iTemp132 = ((iRec65[0] + int(iRec110[0])) % iSlow0);
			int iTemp133 = int((fSlow5 * iTemp132));
			iRec111[0] = ((iRec111[1] * (1 - iTemp133)) + (iTemp131 * iTemp133));
			int iTemp134 = int((fSlow2 * (1 + (2.3283064370807974e-10f * iRec17))));
			iRec112[0] = ((iVec0[1] * iRec112[1]) + (iTemp65 * iTemp134));
			int iTemp135 = ((iRec65[0] + int(iRec112[0])) % iSlow0);
			int iTemp136 = int((fSlow5 * iTemp135));
			iRec113[0] = ((iRec113[1] * (1 - iTemp136)) + (iTemp134 * iTemp136));
			int iTemp137 = int((fSlow2 * (1 + (2.3283064370807974e-10f * iRec15))));
			iRec114[0] = ((iVec0[1] * iRec114[1]) + (iTemp65 * iTemp137));
			int iTemp138 = ((iRec65[0] + int(iRec114[0])) % iSlow0);
			int iTemp139 = int((fSlow5 * iTemp138));
			iRec115[0] = ((iRec115[1] * (1 - iTemp139)) + (iTemp137 * iTemp139));
			int iTemp140 = int((fSlow2 * (1 + (2.3283064370807974e-10f * iRec13))));
			iRec116[0] = ((iVec0[1] * iRec116[1]) + (iTemp65 * iTemp140));
			int iTemp141 = ((iRec65[0] + int(iRec116[0])) % iSlow0);
			int iTemp142 = int((fSlow5 * iTemp141));
			iRec117[0] = ((iRec117[1] * (1 - iTemp142)) + (iTemp140 * iTemp142));
			int iTemp143 = int((fSlow2 * (1 + (2.3283064370807974e-10f * iRec11))));
			iRec118[0] = ((iVec0[1] * iRec118[1]) + (iTemp65 * iTemp143));
			int iTemp144 = ((iRec65[0] + int(iRec118[0])) % iSlow0);
			int iTemp145 = int((fSlow5 * iTemp144));
			iRec119[0] = ((iRec119[1] * (1 - iTemp145)) + (iTemp143 * iTemp145));
			int iTemp146 = int((fSlow2 * (1 + (2.3283064370807974e-10f * iRec9))));
			iRec120[0] = ((iVec0[1] * iRec120[1]) + (iTemp65 * iTemp146));
			int iTemp147 = ((iRec65[0] + int(iRec120[0])) % iSlow0);
			int iTemp148 = int((fSlow5 * iTemp147));
			iRec121[0] = ((iRec121[1] * (1 - iTemp148)) + (iTemp146 * iTemp148));
			int iTemp149 = int((fSlow2 * (1 + (2.3283064370807974e-10f * iRec7))));
			iRec122[0] = ((iVec0[1] * iRec122[1]) + (iTemp65 * iTemp149));
			int iTemp150 = ((iRec65[0] + int(iRec122[0])) % iSlow0);
			int iTemp151 = int((fSlow5 * iTemp150));
			iRec123[0] = ((iRec123[1] * (1 - iTemp151)) + (iTemp149 * iTemp151));
			int iTemp152 = int((fSlow2 * (1 + (2.3283064370807974e-10f * iRec5))));
			iRec124[0] = ((iVec0[1] * iRec124[1]) + (iTemp65 * iTemp152));
			int iTemp153 = ((iRec65[0] + int(iRec124[0])) % iSlow0);
			int iTemp154 = int((fSlow5 * iTemp153));
			iRec125[0] = ((iRec125[1] * (1 - iTemp154)) + (iTemp152 * iTemp154));
			int iTemp155 = int((fSlow2 * (1 + (2.3283064370807974e-10f * iRec3))));
			iRec126[0] = ((iVec0[1] * iRec126[1]) + (iTemp65 * iTemp155));
			int iTemp156 = ((iRec65[0] + int(iRec126[0])) % iSlow0);
			int iTemp157 = int((fSlow5 * iTemp156));
			iRec127[0] = ((iRec127[1] * (1 - iTemp157)) + (iTemp155 * iTemp157));
			int iTemp158 = int((fSlow2 * (1 + (2.3283064370807974e-10f * iRec1[0]))));
			iRec128[0] = ((iVec0[1] * iRec128[1]) + (iTemp65 * iTemp158));
			int iTemp159 = ((iRec65[0] + int(iRec128[0])) % iSlow0);
			int iTemp160 = int((fSlow5 * iTemp159));
			iRec129[0] = ((iRec129[1] * (1 - iTemp160)) + (iTemp158 * iTemp160));
			output0[i] = (FAUSTFLOAT)(fSlow39 * ((((((((((((((((((((((((((((((((iSlow38 * (ftbl0[((iTemp159 + int(iRec129[0])) % iSlow1)] * sinf((fSlow4 * iTemp159)))) + (iSlow37 * (ftbl0[((iTemp156 + int(iRec127[0])) % iSlow1)] * sinf((fSlow4 * iTemp156))))) + (iSlow36 * (ftbl0[((iTemp153 + int(iRec125[0])) % iSlow1)] * sinf((fSlow4 * iTemp153))))) + (iSlow35 * (ftbl0[((iTemp150 + int(iRec123[0])) % iSlow1)] * sinf((fSlow4 * iTemp150))))) + (iSlow34 * (ftbl0[((iTemp147 + int(iRec121[0])) % iSlow1)] * sinf((fSlow4 * iTemp147))))) + (iSlow33 * (ftbl0[((iTemp144 + int(iRec119[0])) % iSlow1)] * sinf((fSlow4 * iTemp144))))) + (iSlow32 * (ftbl0[((iTemp141 + int(iRec117[0])) % iSlow1)] * sinf((fSlow4 * iTemp141))))) + (iSlow31 * (ftbl0[((iTemp138 + int(iRec115[0])) % iSlow1)] * sinf((fSlow4 * iTemp138))))) + (iSlow30 * (ftbl0[((iTemp135 + int(iRec113[0])) % iSlow1)] * sinf((fSlow4 * iTemp135))))) + (iSlow29 * (ftbl0[((iTemp132 + int(iRec111[0])) % iSlow1)] * sinf((fSlow4 * iTemp132))))) + (iSlow28 * (ftbl0[((iTemp129 + int(iRec109[0])) % iSlow1)] * sinf((fSlow4 * iTemp129))))) + (iSlow27 * (ftbl0[((iTemp126 + int(iRec107[0])) % iSlow1)] * sinf((fSlow4 * iTemp126))))) + (iSlow26 * (ftbl0[((iTemp123 + int(iRec105[0])) % iSlow1)] * sinf((fSlow4 * iTemp123))))) + (iSlow25 * (ftbl0[((iTemp120 + int(iRec103[0])) % iSlow1)] * sinf((fSlow4 * iTemp120))))) + (iSlow24 * (ftbl0[((iTemp117 + int(iRec101[0])) % iSlow1)] * sinf((fSlow4 * iTemp117))))) + (iSlow23 * (ftbl0[((iTemp114 + int(iRec99[0])) % iSlow1)] * sinf((fSlow4 * iTemp114))))) + (iSlow22 * (ftbl0[((iTemp111 + int(iRec97[0])) % iSlow1)] * sinf((fSlow4 * iTemp111))))) + (iSlow21 * (ftbl0[((iTemp108 + int(iRec95[0])) % iSlow1)] * sinf((fSlow4 * iTemp108))))) + (iSlow20 * (ftbl0[((iTemp105 + int(iRec93[0])) % iSlow1)] * sinf((fSlow4 * iTemp105))))) + (iSlow19 * (ftbl0[((iTemp102 + int(iRec91[0])) % iSlow1)] * sinf((fSlow4 * iTemp102))))) + (iSlow18 * (ftbl0[((iTemp99 + int(iRec89[0])) % iSlow1)] * sinf((fSlow4 * iTemp99))))) + (iSlow17 * (ftbl0[((iTemp96 + int(iRec87[0])) % iSlow1)] * sinf((fSlow4 * iTemp96))))) + (iSlow16 * (ftbl0[((iTemp93 + int(iRec85[0])) % iSlow1)] * sinf((fSlow4 * iTemp93))))) + (iSlow15 * (ftbl0[((iTemp90 + int(iRec83[0])) % iSlow1)] * sinf((fSlow4 * iTemp90))))) + (iSlow14 * (ftbl0[((iTemp87 + int(iRec81[0])) % iSlow1)] * sinf((fSlow4 * iTemp87))))) + (iSlow13 * (ftbl0[((iTemp84 + int(iRec79[0])) % iSlow1)] * sinf((fSlow4 * iTemp84))))) + (iSlow12 * (ftbl0[((iTemp81 + int(iRec77[0])) % iSlow1)] * sinf((fSlow4 * iTemp81))))) + (iSlow11 * (ftbl0[((iTemp78 + int(iRec75[0])) % iSlow1)] * sinf((fSlow4 * iTemp78))))) + (iSlow10 * (ftbl0[((iTemp75 + int(iRec73[0])) % iSlow1)] * sinf((fSlow4 * iTemp75))))) + (iSlow9 * (ftbl0[((iTemp72 + int(iRec71[0])) % iSlow1)] * sinf((fSlow4 * iTemp72))))) + (iSlow8 * (ftbl0[((iTemp69 + int(iRec69[0])) % iSlow1)] * sinf((fSlow4 * iTemp69))))) + (iSlow7 * (ftbl0[((iTemp66 + int(iRec66[0])) % iSlow1)] * sinf((fSlow4 * iTemp66))))));
			int iTemp161 = int((fSlow2 * (1 + (2.3283064370807974e-10f * iRec64))));
			iRec130[0] = ((iVec0[1] * iRec130[1]) + (iTemp65 * iTemp161));
			int iTemp162 = ((iRec65[0] + int(iRec130[0])) % iSlow0);
			int iTemp163 = int((fSlow5 * iTemp162));
			iRec131[0] = ((iRec131[1] * (1 - iTemp163)) + (iTemp161 * iTemp163));
			int iTemp164 = int((fSlow2 * (1 + (2.3283064370807974e-10f * iRec62))));
			iRec132[0] = ((iVec0[1] * iRec132[1]) + (iTemp65 * iTemp164));
			int iTemp165 = ((iRec65[0] + int(iRec132[0])) % iSlow0);
			int iTemp166 = int((fSlow5 * iTemp165));
			iRec133[0] = ((iRec133[1] * (1 - iTemp166)) + (iTemp164 * iTemp166));
			int iTemp167 = int((fSlow2 * (1 + (2.3283064370807974e-10f * iRec60))));
			iRec134[0] = ((iVec0[1] * iRec134[1]) + (iTemp65 * iTemp167));
			int iTemp168 = ((iRec65[0] + int(iRec134[0])) % iSlow0);
			int iTemp169 = int((fSlow5 * iTemp168));
			iRec135[0] = ((iRec135[1] * (1 - iTemp169)) + (iTemp167 * iTemp169));
			int iTemp170 = int((fSlow2 * (1 + (2.3283064370807974e-10f * iRec58))));
			iRec136[0] = ((iVec0[1] * iRec136[1]) + (iTemp65 * iTemp170));
			int iTemp171 = ((iRec65[0] + int(iRec136[0])) % iSlow0);
			int iTemp172 = int((fSlow5 * iTemp171));
			iRec137[0] = ((iRec137[1] * (1 - iTemp172)) + (iTemp170 * iTemp172));
			int iTemp173 = int((fSlow2 * (1 + (2.3283064370807974e-10f * iRec56))));
			iRec138[0] = ((iVec0[1] * iRec138[1]) + (iTemp65 * iTemp173));
			int iTemp174 = ((iRec65[0] + int(iRec138[0])) % iSlow0);
			int iTemp175 = int((fSlow5 * iTemp174));
			iRec139[0] = ((iRec139[1] * (1 - iTemp175)) + (iTemp173 * iTemp175));
			int iTemp176 = int((fSlow2 * (1 + (2.3283064370807974e-10f * iRec54))));
			iRec140[0] = ((iVec0[1] * iRec140[1]) + (iTemp65 * iTemp176));
			int iTemp177 = ((iRec65[0] + int(iRec140[0])) % iSlow0);
			int iTemp178 = int((fSlow5 * iTemp177));
			iRec141[0] = ((iRec141[1] * (1 - iTemp178)) + (iTemp176 * iTemp178));
			int iTemp179 = int((fSlow2 * (1 + (2.3283064370807974e-10f * iRec52))));
			iRec142[0] = ((iVec0[1] * iRec142[1]) + (iTemp65 * iTemp179));
			int iTemp180 = ((iRec65[0] + int(iRec142[0])) % iSlow0);
			int iTemp181 = int((fSlow5 * iTemp180));
			iRec143[0] = ((iRec143[1] * (1 - iTemp181)) + (iTemp179 * iTemp181));
			int iTemp182 = int((fSlow2 * (1 + (2.3283064370807974e-10f * iRec50))));
			iRec144[0] = ((iVec0[1] * iRec144[1]) + (iTemp65 * iTemp182));
			int iTemp183 = ((iRec65[0] + int(iRec144[0])) % iSlow0);
			int iTemp184 = int((fSlow5 * iTemp183));
			iRec145[0] = ((iRec145[1] * (1 - iTemp184)) + (iTemp182 * iTemp184));
			int iTemp185 = int((fSlow2 * (1 + (2.3283064370807974e-10f * iRec48))));
			iRec146[0] = ((iVec0[1] * iRec146[1]) + (iTemp65 * iTemp185));
			int iTemp186 = ((iRec65[0] + int(iRec146[0])) % iSlow0);
			int iTemp187 = int((fSlow5 * iTemp186));
			iRec147[0] = ((iRec147[1] * (1 - iTemp187)) + (iTemp185 * iTemp187));
			int iTemp188 = int((fSlow2 * (1 + (2.3283064370807974e-10f * iRec46))));
			iRec148[0] = ((iVec0[1] * iRec148[1]) + (iTemp65 * iTemp188));
			int iTemp189 = ((iRec65[0] + int(iRec148[0])) % iSlow0);
			int iTemp190 = int((fSlow5 * iTemp189));
			iRec149[0] = ((iRec149[1] * (1 - iTemp190)) + (iTemp188 * iTemp190));
			int iTemp191 = int((fSlow2 * (1 + (2.3283064370807974e-10f * iRec44))));
			iRec150[0] = ((iVec0[1] * iRec150[1]) + (iTemp65 * iTemp191));
			int iTemp192 = ((iRec65[0] + int(iRec150[0])) % iSlow0);
			int iTemp193 = int((fSlow5 * iTemp192));
			iRec151[0] = ((iRec151[1] * (1 - iTemp193)) + (iTemp191 * iTemp193));
			int iTemp194 = int((fSlow2 * (1 + (2.3283064370807974e-10f * iRec42))));
			iRec152[0] = ((iVec0[1] * iRec152[1]) + (iTemp65 * iTemp194));
			int iTemp195 = ((iRec65[0] + int(iRec152[0])) % iSlow0);
			int iTemp196 = int((fSlow5 * iTemp195));
			iRec153[0] = ((iRec153[1] * (1 - iTemp196)) + (iTemp194 * iTemp196));
			int iTemp197 = int((fSlow2 * (1 + (2.3283064370807974e-10f * iRec40))));
			iRec154[0] = ((iVec0[1] * iRec154[1]) + (iTemp65 * iTemp197));
			int iTemp198 = ((iRec65[0] + int(iRec154[0])) % iSlow0);
			int iTemp199 = int((fSlow5 * iTemp198));
			iRec155[0] = ((iRec155[1] * (1 - iTemp199)) + (iTemp197 * iTemp199));
			int iTemp200 = int((fSlow2 * (1 + (2.3283064370807974e-10f * iRec38))));
			iRec156[0] = ((iVec0[1] * iRec156[1]) + (iTemp65 * iTemp200));
			int iTemp201 = ((iRec65[0] + int(iRec156[0])) % iSlow0);
			int iTemp202 = int((fSlow5 * iTemp201));
			iRec157[0] = ((iRec157[1] * (1 - iTemp202)) + (iTemp200 * iTemp202));
			int iTemp203 = int((fSlow2 * (1 + (2.3283064370807974e-10f * iRec36))));
			iRec158[0] = ((iVec0[1] * iRec158[1]) + (iTemp65 * iTemp203));
			int iTemp204 = ((iRec65[0] + int(iRec158[0])) % iSlow0);
			int iTemp205 = int((fSlow5 * iTemp204));
			iRec159[0] = ((iRec159[1] * (1 - iTemp205)) + (iTemp203 * iTemp205));
			int iTemp206 = int((fSlow2 * (1 + (2.3283064370807974e-10f * iRec34))));
			iRec160[0] = ((iVec0[1] * iRec160[1]) + (iTemp65 * iTemp206));
			int iTemp207 = ((iRec65[0] + int(iRec160[0])) % iSlow0);
			int iTemp208 = int((fSlow5 * iTemp207));
			iRec161[0] = ((iRec161[1] * (1 - iTemp208)) + (iTemp206 * iTemp208));
			int iTemp209 = int((fSlow2 * (1 + (2.3283064370807974e-10f * iRec32))));
			iRec162[0] = ((iVec0[1] * iRec162[1]) + (iTemp65 * iTemp209));
			int iTemp210 = ((iRec65[0] + int(iRec162[0])) % iSlow0);
			int iTemp211 = int((fSlow5 * iTemp210));
			iRec163[0] = ((iRec163[1] * (1 - iTemp211)) + (iTemp209 * iTemp211));
			int iTemp212 = int((fSlow2 * (1 + (2.3283064370807974e-10f * iRec30))));
			iRec164[0] = ((iVec0[1] * iRec164[1]) + (iTemp65 * iTemp212));
			int iTemp213 = ((iRec65[0] + int(iRec164[0])) % iSlow0);
			int iTemp214 = int((fSlow5 * iTemp213));
			iRec165[0] = ((iRec165[1] * (1 - iTemp214)) + (iTemp212 * iTemp214));
			int iTemp215 = int((fSlow2 * (1 + (2.3283064370807974e-10f * iRec28))));
			iRec166[0] = ((iVec0[1] * iRec166[1]) + (iTemp65 * iTemp215));
			int iTemp216 = ((iRec65[0] + int(iRec166[0])) % iSlow0);
			int iTemp217 = int((fSlow5 * iTemp216));
			iRec167[0] = ((iRec167[1] * (1 - iTemp217)) + (iTemp215 * iTemp217));
			int iTemp218 = int((fSlow2 * (1 + (2.3283064370807974e-10f * iRec26))));
			iRec168[0] = ((iVec0[1] * iRec168[1]) + (iTemp65 * iTemp218));
			int iTemp219 = ((iRec65[0] + int(iRec168[0])) % iSlow0);
			int iTemp220 = int((fSlow5 * iTemp219));
			iRec169[0] = ((iRec169[1] * (1 - iTemp220)) + (iTemp218 * iTemp220));
			int iTemp221 = int((fSlow2 * (1 + (2.3283064370807974e-10f * iRec24))));
			iRec170[0] = ((iVec0[1] * iRec170[1]) + (iTemp65 * iTemp221));
			int iTemp222 = ((iRec65[0] + int(iRec170[0])) % iSlow0);
			int iTemp223 = int((fSlow5 * iTemp222));
			iRec171[0] = ((iRec171[1] * (1 - iTemp223)) + (iTemp221 * iTemp223));
			int iTemp224 = int((fSlow2 * (1 + (2.3283064370807974e-10f * iRec22))));
			iRec172[0] = ((iVec0[1] * iRec172[1]) + (iTemp65 * iTemp224));
			int iTemp225 = ((iRec65[0] + int(iRec172[0])) % iSlow0);
			int iTemp226 = int((fSlow5 * iTemp225));
			iRec173[0] = ((iRec173[1] * (1 - iTemp226)) + (iTemp224 * iTemp226));
			int iTemp227 = int((fSlow2 * (1 + (2.3283064370807974e-10f * iRec20))));
			iRec174[0] = ((iVec0[1] * iRec174[1]) + (iTemp65 * iTemp227));
			int iTemp228 = ((iRec65[0] + int(iRec174[0])) % iSlow0);
			int iTemp229 = int((fSlow5 * iTemp228));
			iRec175[0] = ((iRec175[1] * (1 - iTemp229)) + (iTemp227 * iTemp229));
			int iTemp230 = int((fSlow2 * (1 + (2.3283064370807974e-10f * iRec18))));
			iRec176[0] = ((iVec0[1] * iRec176[1]) + (iTemp65 * iTemp230));
			int iTemp231 = ((iRec65[0] + int(iRec176[0])) % iSlow0);
			int iTemp232 = int((fSlow5 * iTemp231));
			iRec177[0] = ((iRec177[1] * (1 - iTemp232)) + (iTemp230 * iTemp232));
			int iTemp233 = int((fSlow2 * (1 + (2.3283064370807974e-10f * iRec16))));
			iRec178[0] = ((iTemp65 * iTemp233) + (iVec0[1] * iRec178[1]));
			int iTemp234 = ((iRec65[0] + int(iRec178[0])) % iSlow0);
			int iTemp235 = int((fSlow5 * iTemp234));
			iRec179[0] = ((iRec179[1] * (1 - iTemp235)) + (iTemp233 * iTemp235));
			int iTemp236 = int((fSlow2 * (1 + (2.3283064370807974e-10f * iRec14))));
			iRec180[0] = ((iVec0[1] * iRec180[1]) + (iTemp65 * iTemp236));
			int iTemp237 = ((iRec65[0] + int(iRec180[0])) % iSlow0);
			int iTemp238 = int((fSlow5 * iTemp237));
			iRec181[0] = ((iRec181[1] * (1 - iTemp238)) + (iTemp236 * iTemp238));
			int iTemp239 = int((fSlow2 * (1 + (2.3283064370807974e-10f * iRec12))));
			iRec182[0] = ((iVec0[1] * iRec182[1]) + (iTemp65 * iTemp239));
			int iTemp240 = ((iRec65[0] + int(iRec182[0])) % iSlow0);
			int iTemp241 = int((fSlow5 * iTemp240));
			iRec183[0] = ((iRec183[1] * (1 - iTemp241)) + (iTemp239 * iTemp241));
			int iTemp242 = int((fSlow2 * (1 + (2.3283064370807974e-10f * iRec10))));
			iRec184[0] = ((iVec0[1] * iRec184[1]) + (iTemp65 * iTemp242));
			int iTemp243 = ((iRec65[0] + int(iRec184[0])) % iSlow0);
			int iTemp244 = int((fSlow5 * iTemp243));
			iRec185[0] = ((iRec185[1] * (1 - iTemp244)) + (iTemp242 * iTemp244));
			int iTemp245 = int((fSlow2 * (1 + (2.3283064370807974e-10f * iRec8))));
			iRec186[0] = ((iVec0[1] * iRec186[1]) + (iTemp65 * iTemp245));
			int iTemp246 = ((iRec65[0] + int(iRec186[0])) % iSlow0);
			int iTemp247 = int((fSlow5 * iTemp246));
			iRec187[0] = ((iRec187[1] * (1 - iTemp247)) + (iTemp245 * iTemp247));
			int iTemp248 = int((fSlow2 * (1 + (2.3283064370807974e-10f * iRec6))));
			iRec188[0] = ((iVec0[1] * iRec188[1]) + (iTemp65 * iTemp248));
			int iTemp249 = ((iRec65[0] + int(iRec188[0])) % iSlow0);
			int iTemp250 = int((fSlow5 * iTemp249));
			iRec189[0] = ((iRec189[1] * (1 - iTemp250)) + (iTemp248 * iTemp250));
			int iTemp251 = int((fSlow2 * (1 + (2.3283064370807974e-10f * iRec4))));
			iRec190[0] = ((iVec0[1] * iRec190[1]) + (iTemp65 * iTemp251));
			int iTemp252 = ((iRec65[0] + int(iRec190[0])) % iSlow0);
			int iTemp253 = int((fSlow5 * iTemp252));
			iRec191[0] = ((iRec191[1] * (1 - iTemp253)) + (iTemp251 * iTemp253));
			int iTemp254 = int((fSlow2 * (1 + (2.3283064370807974e-10f * iRec2))));
			iRec192[0] = ((iVec0[1] * iRec192[1]) + (iTemp65 * iTemp254));
			int iTemp255 = ((iRec65[0] + int(iRec192[0])) % iSlow0);
			int iTemp256 = int((fSlow5 * iTemp255));
			iRec193[0] = ((iRec193[1] * (1 - iTemp256)) + (iTemp254 * iTemp256));
			output1[i] = (FAUSTFLOAT)(fSlow39 * ((((((((((((((((((((((((((((((((iSlow71 * (ftbl0[((iTemp255 + int(iRec193[0])) % iSlow1)] * sinf((fSlow4 * iTemp255)))) + (iSlow70 * (ftbl0[((iTemp252 + int(iRec191[0])) % iSlow1)] * sinf((fSlow4 * iTemp252))))) + (iSlow69 * (ftbl0[((iTemp249 + int(iRec189[0])) % iSlow1)] * sinf((fSlow4 * iTemp249))))) + (iSlow68 * (ftbl0[((iTemp246 + int(iRec187[0])) % iSlow1)] * sinf((fSlow4 * iTemp246))))) + (iSlow67 * (ftbl0[((iTemp243 + int(iRec185[0])) % iSlow1)] * sinf((fSlow4 * iTemp243))))) + (iSlow66 * (ftbl0[((iTemp240 + int(iRec183[0])) % iSlow1)] * sinf((fSlow4 * iTemp240))))) + (iSlow65 * (ftbl0[((iTemp237 + int(iRec181[0])) % iSlow1)] * sinf((fSlow4 * iTemp237))))) + (iSlow64 * (ftbl0[((iTemp234 + int(iRec179[0])) % iSlow1)] * sinf((fSlow4 * iTemp234))))) + (iSlow63 * (ftbl0[((iTemp231 + int(iRec177[0])) % iSlow1)] * sinf((fSlow4 * iTemp231))))) + (iSlow62 * (ftbl0[((iTemp228 + int(iRec175[0])) % iSlow1)] * sinf((fSlow4 * iTemp228))))) + (iSlow61 * (ftbl0[((iTemp225 + int(iRec173[0])) % iSlow1)] * sinf((fSlow4 * iTemp225))))) + (iSlow60 * (ftbl0[((iTemp222 + int(iRec171[0])) % iSlow1)] * sinf((fSlow4 * iTemp222))))) + (iSlow59 * (ftbl0[((iTemp219 + int(iRec169[0])) % iSlow1)] * sinf((fSlow4 * iTemp219))))) + (iSlow58 * (ftbl0[((iTemp216 + int(iRec167[0])) % iSlow1)] * sinf((fSlow4 * iTemp216))))) + (iSlow57 * (ftbl0[((iTemp213 + int(iRec165[0])) % iSlow1)] * sinf((fSlow4 * iTemp213))))) + (iSlow56 * (ftbl0[((iTemp210 + int(iRec163[0])) % iSlow1)] * sinf((fSlow4 * iTemp210))))) + (iSlow55 * (ftbl0[((iTemp207 + int(iRec161[0])) % iSlow1)] * sinf((fSlow4 * iTemp207))))) + (iSlow54 * (ftbl0[((iTemp204 + int(iRec159[0])) % iSlow1)] * sinf((fSlow4 * iTemp204))))) + (iSlow53 * (ftbl0[((iTemp201 + int(iRec157[0])) % iSlow1)] * sinf((fSlow4 * iTemp201))))) + (iSlow52 * (ftbl0[((iTemp198 + int(iRec155[0])) % iSlow1)] * sinf((fSlow4 * iTemp198))))) + (iSlow51 * (ftbl0[((iTemp195 + int(iRec153[0])) % iSlow1)] * sinf((fSlow4 * iTemp195))))) + (iSlow50 * (ftbl0[((iTemp192 + int(iRec151[0])) % iSlow1)] * sinf((fSlow4 * iTemp192))))) + (iSlow49 * (ftbl0[((iTemp189 + int(iRec149[0])) % iSlow1)] * sinf((fSlow4 * iTemp189))))) + (iSlow48 * (ftbl0[((iTemp186 + int(iRec147[0])) % iSlow1)] * sinf((fSlow4 * iTemp186))))) + (iSlow47 * (ftbl0[((iTemp183 + int(iRec145[0])) % iSlow1)] * sinf((fSlow4 * iTemp183))))) + (iSlow46 * (ftbl0[((iTemp180 + int(iRec143[0])) % iSlow1)] * sinf((fSlow4 * iTemp180))))) + (iSlow45 * (ftbl0[((iTemp177 + int(iRec141[0])) % iSlow1)] * sinf((fSlow4 * iTemp177))))) + (iSlow44 * (ftbl0[((iTemp174 + int(iRec139[0])) % iSlow1)] * sinf((fSlow4 * iTemp174))))) + (iSlow43 * (ftbl0[((iTemp171 + int(iRec137[0])) % iSlow1)] * sinf((fSlow4 * iTemp171))))) + (iSlow42 * (ftbl0[((iTemp168 + int(iRec135[0])) % iSlow1)] * sinf((fSlow4 * iTemp168))))) + (iSlow41 * (ftbl0[((iTemp165 + int(iRec133[0])) % iSlow1)] * sinf((fSlow4 * iTemp165))))) + (iSlow40 * (ftbl0[((iTemp162 + int(iRec131[0])) % iSlow1)] * sinf((fSlow4 * iTemp162))))));
			// post processing
			iRec193[1] = iRec193[0];
			iRec192[1] = iRec192[0];
			iRec191[1] = iRec191[0];
			iRec190[1] = iRec190[0];
			iRec189[1] = iRec189[0];
			iRec188[1] = iRec188[0];
			iRec187[1] = iRec187[0];
			iRec186[1] = iRec186[0];
			iRec185[1] = iRec185[0];
			iRec184[1] = iRec184[0];
			iRec183[1] = iRec183[0];
			iRec182[1] = iRec182[0];
			iRec181[1] = iRec181[0];
			iRec180[1] = iRec180[0];
			iRec179[1] = iRec179[0];
			iRec178[1] = iRec178[0];
			iRec177[1] = iRec177[0];
			iRec176[1] = iRec176[0];
			iRec175[1] = iRec175[0];
			iRec174[1] = iRec174[0];
			iRec173[1] = iRec173[0];
			iRec172[1] = iRec172[0];
			iRec171[1] = iRec171[0];
			iRec170[1] = iRec170[0];
			iRec169[1] = iRec169[0];
			iRec168[1] = iRec168[0];
			iRec167[1] = iRec167[0];
			iRec166[1] = iRec166[0];
			iRec165[1] = iRec165[0];
			iRec164[1] = iRec164[0];
			iRec163[1] = iRec163[0];
			iRec162[1] = iRec162[0];
			iRec161[1] = iRec161[0];
			iRec160[1] = iRec160[0];
			iRec159[1] = iRec159[0];
			iRec158[1] = iRec158[0];
			iRec157[1] = iRec157[0];
			iRec156[1] = iRec156[0];
			iRec155[1] = iRec155[0];
			iRec154[1] = iRec154[0];
			iRec153[1] = iRec153[0];
			iRec152[1] = iRec152[0];
			iRec151[1] = iRec151[0];
			iRec150[1] = iRec150[0];
			iRec149[1] = iRec149[0];
			iRec148[1] = iRec148[0];
			iRec147[1] = iRec147[0];
			iRec146[1] = iRec146[0];
			iRec145[1] = iRec145[0];
			iRec144[1] = iRec144[0];
			iRec143[1] = iRec143[0];
			iRec142[1] = iRec142[0];
			iRec141[1] = iRec141[0];
			iRec140[1] = iRec140[0];
			iRec139[1] = iRec139[0];
			iRec138[1] = iRec138[0];
			iRec137[1] = iRec137[0];
			iRec136[1] = iRec136[0];
			iRec135[1] = iRec135[0];
			iRec134[1] = iRec134[0];
			iRec133[1] = iRec133[0];
			iRec132[1] = iRec132[0];
			iRec131[1] = iRec131[0];
			iRec130[1] = iRec130[0];
			iRec129[1] = iRec129[0];
			iRec128[1] = iRec128[0];
			iRec127[1] = iRec127[0];
			iRec126[1] = iRec126[0];
			iRec125[1] = iRec125[0];
			iRec124[1] = iRec124[0];
			iRec123[1] = iRec123[0];
			iRec122[1] = iRec122[0];
			iRec121[1] = iRec121[0];
			iRec120[1] = iRec120[0];
			iRec119[1] = iRec119[0];
			iRec118[1] = iRec118[0];
			iRec117[1] = iRec117[0];
			iRec116[1] = iRec116[0];
			iRec115[1] = iRec115[0];
			iRec114[1] = iRec114[0];
			iRec113[1] = iRec113[0];
			iRec112[1] = iRec112[0];
			iRec111[1] = iRec111[0];
			iRec110[1] = iRec110[0];
			iRec109[1] = iRec109[0];
			iRec108[1] = iRec108[0];
			iRec107[1] = iRec107[0];
			iRec106[1] = iRec106[0];
			iRec105[1] = iRec105[0];
			iRec104[1] = iRec104[0];
			iRec103[1] = iRec103[0];
			iRec102[1] = iRec102[0];
			iRec101[1] = iRec101[0];
			iRec100[1] = iRec100[0];
			iRec99[1] = iRec99[0];
			iRec98[1] = iRec98[0];
			iRec97[1] = iRec97[0];
			iRec96[1] = iRec96[0];
			iRec95[1] = iRec95[0];
			iRec94[1] = iRec94[0];
			iRec93[1] = iRec93[0];
			iRec92[1] = iRec92[0];
			iRec91[1] = iRec91[0];
			iRec90[1] = iRec90[0];
			iRec89[1] = iRec89[0];
			iRec88[1] = iRec88[0];
			iRec87[1] = iRec87[0];
			iRec86[1] = iRec86[0];
			iRec85[1] = iRec85[0];
			iRec84[1] = iRec84[0];
			iRec83[1] = iRec83[0];
			iRec82[1] = iRec82[0];
			iRec81[1] = iRec81[0];
			iRec80[1] = iRec80[0];
			iRec79[1] = iRec79[0];
			iRec78[1] = iRec78[0];
			iRec77[1] = iRec77[0];
			iRec76[1] = iRec76[0];
			iRec75[1] = iRec75[0];
			iRec74[1] = iRec74[0];
			iRec73[1] = iRec73[0];
			iRec72[1] = iRec72[0];
			iRec71[1] = iRec71[0];
			iRec70[1] = iRec70[0];
			iRec69[1] = iRec69[0];
			iRec68[1] = iRec68[0];
			iRec67[1] = iRec67[0];
			iRec66[1] = iRec66[0];
			iRec65[1] = iRec65[0];
			iRec0[1] = iRec0[0];
			iRec1[1] = iRec1[0];
			iVec0[1] = iVec0[0];
		}
	}
};



/***************************END USER SECTION ***************************/

/*******************BEGIN ARCHITECTURE SECTION (part 2/2)***************/
					
mydsp*	DSP;

std::list<GUI*>               GUI::fGuiList;

//-------------------------------------------------------------------------
// 									MAIN
//-------------------------------------------------------------------------
int main(int argc, char *argv[] )
{
	char* appname = basename (argv [0]);
    char  rcfilename[256];
	char* home = getenv("HOME");
	snprintf(rcfilename, 255, "%s/.%src", home, appname);
	
	DSP = new mydsp();
	if (DSP==0) {
        std::cerr << "Unable to allocate Faust DSP object" << std::endl;
		exit(1);
	}

	CMDUI* interface = new CMDUI(argc, argv);
	FUI* finterface	= new FUI();
	DSP->buildUserInterface(interface);
	DSP->buildUserInterface(finterface);

#ifdef HTTPCTRL
	httpdUI* httpdinterface = new httpdUI(appname, DSP->getNumInputs(), DSP->getNumOutputs(), argc, argv);
	DSP->buildUserInterface(httpdinterface);
    std::cout << "HTTPD is on" << std::endl;
#endif

#ifdef OSCCTRL
	GUI* oscinterface = new OSCUI(appname, argc, argv);
	DSP->buildUserInterface(oscinterface);
#endif

	alsaaudio audio (argc, argv, DSP);
	audio.init(appname, DSP);
	finterface->recallState(rcfilename);	
	audio.start();
	
#ifdef HTTPCTRL
	httpdinterface->run();
#endif
	
#ifdef OSCCTRL
	oscinterface->run();
#endif
	interface->run();
	
	audio.stop();
	finterface->saveState(rcfilename);
    
    // desallocation
    delete interface;
    delete finterface;
#ifdef HTTPCTRL
	 delete httpdinterface;
#endif
#ifdef OSCCTRL
	 delete oscinterface;
#endif

  	return 0;
}
/********************END ARCHITECTURE SECTION (part 2/2)****************/

