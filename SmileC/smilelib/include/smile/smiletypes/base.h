#ifndef __SMILE_SMILETYPES_BASE_H__
#define __SMILE_SMILETYPES_BASE_H__

#define SetupFunction(__name__, __function__, __param__, __argNames__, __argCheckFlags__, __minArgs__, __maxArgs__, __numArgsToTypeCheck__, __argTypeChecks__) \
	(SmileUserObject_SetupFunction(base, (__function__), (__param__), \
		(__name__), (__argNames__), (__argCheckFlags__), (__minArgs__), (__maxArgs__), (__numArgsToTypeCheck__), (__argTypeChecks__)))

#define SetupSynonym(__oldName__, __newName__) \
	(SmileUserObject_SetupSynonym(base, (__oldName__), (__newName__)))

#endif