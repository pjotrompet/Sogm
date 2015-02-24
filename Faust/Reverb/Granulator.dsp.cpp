//-----------------------------------------------------
//
// Code generated with Faust 0.9.71 (http://faust.grame.fr)
//-----------------------------------------------------
/* link with  */
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
  public:
	static void metadata(Meta* m) 	{ 
	}

	virtual int getNumInputs() 	{ return 2; }
	virtual int getNumOutputs() 	{ return 1; }
	static void classInit(int samplingFreq) {
	}
	virtual void instanceInit(int samplingFreq) {
		fSamplingFreq = samplingFreq;
	}
	virtual void init(int samplingFreq) {
		classInit(samplingFreq);
		instanceInit(samplingFreq);
	}
	virtual void buildUserInterface(UI* interface) {
		interface->openVerticalBox("0x00");
		interface->closeBox();
	}
	virtual void compute (int count, FAUSTFLOAT** input, FAUSTFLOAT** output) {
		FAUSTFLOAT* input0 = input[0];
		FAUSTFLOAT* input1 = input[1];
		FAUSTFLOAT* output0 = output[0];
		for (int i=0; i<count; i++) {
			output0[i] = (FAUSTFLOAT)((float)input0[i] + (float)input1[i]);
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

