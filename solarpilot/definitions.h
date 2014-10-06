
#ifndef _VARDEFS_
#define _VARDEFS_ 1

#ifdef _MSC_VER
#pragma warning(disable : 4267)
#endif

//Enumeration of data columns in the variable map file


//Custom module settings
#define _CUSTOM_REC 0		//If using custom geometry functions in the CustomReceiverWindow, define 1
//Sandbox mode
#define _SANDBOX 0
//demo only
#define _DEMO 0	//Is this a demo version?
const int _demo_date[] = {2014,8,1};
//Include Coretrace (relevant to fieldcore only! Disabling this option will cause SolarPILOT compilation to fail.).
//#define SP_USE_SOLTRACE
//Compile without threading functionality? Comment out to remove.
//#define SP_USE_THREADS
//crete local make-dir functions
//#define SP_USE_MKDIR

//enum VMAP
//{
//	VDOMAIN=0,		//Variable domain (solarfield, receiver, etc.)
//	STRING_NAME,	//Variable name (dictionary)
//	VAR_NAME,		//Local variable name
//	TYPE,			//Data type of the variable (bool, string, int, etc)
//	VALUE,			//Default value
//	UNITS,			//Variable units
//	RANGE,			//Allowable range of values
//	IS_PARAM,		//Include the variable in parameterizable lists
//	//DEL_NL,			//(unused) DELSOL namelist containing analogous variable
//	//DEL_NAME,		//(unused) DELSOL analogous variable name
//	CONTROL,		//Type of UI control for the variable
//	SPECIAL,		//Arguments for constructing non-textctrl controls
//	UI_DISABLE,		//Disable this variable in the UI - always
//	LABEL,			//UI label
//	DESCRIPTION		//Tooltip detailed description
//};

struct vardefs
{
     const char *domain, *name, *vname, *type, *value, *units, *range, 
				*isparam, *control, *special, *disable, *label, *description;
};

extern vardefs variable_definition_array[311];
#endif
