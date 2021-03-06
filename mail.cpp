#pragma once



#include "mail.h"





bool gInit;

logprintf_t logprintf;

boost::mutex gMutex;
boost::regex gExpression;
//boost::atomic<bool> gInit;
std::queue<mailData> amxThreadQueue;
std::queue<mailData> amxProcessTickQueue;
std::list<AMX *> amxList;


extern void *pAMXFunctions;
extern amxProcess *gProcess;





PLUGIN_EXPORT unsigned int PLUGIN_CALL Supports() 
{
    return (SUPPORTS_VERSION | SUPPORTS_AMX_NATIVES | SUPPORTS_PROCESS_TICK);
}



PLUGIN_EXPORT bool PLUGIN_CALL Load(void **ppData) 
{
    pAMXFunctions = ppData[PLUGIN_DATA_AMX_EXPORTS];
    logprintf = (logprintf_t)ppData[PLUGIN_DATA_LOGPRINTF];

	gExpression = "[a-zA-Z0-9_\\.]+@([a-zA-Z0-9\\-]+\\.)+[a-zA-Z]{2,4}";
	gProcess = new amxProcess();

	logprintf("  Mail v%s by BJIADOKC loaded", PLUGIN_VERSION);

    return true;
}



PLUGIN_EXPORT void PLUGIN_CALL Unload()
{
	amxList.clear();

	logprintf("  Mail v%s by BJIADOKC unloaded", PLUGIN_VERSION);

	delete gProcess;
}



PLUGIN_EXPORT int PLUGIN_CALL AmxLoad(AMX *amx) 
{
	amxList.push_back(amx);

    return amx_Register(amx, amxNatives::mailNatives, -1);
}



PLUGIN_EXPORT int PLUGIN_CALL AmxUnload(AMX *amx) 
{
	for(std::list<AMX*>::iterator i = amxList.begin(); i != amxList.end(); i++) 
	{
		if(*i == amx) 
		{
			amxList.erase(i);

			break;
		}
	}

    return AMX_ERR_NONE;
}



PLUGIN_EXPORT void PLUGIN_CALL ProcessTick()
{
	while(!amxProcessTickQueue.empty())
	{
		int amx_idx;

		cell amxAddress[3];
		mailData getme;

		std::list<AMX *>::iterator end = amxList.end();

		boost::mutex::scoped_lock lock(gMutex);
		getme = amxProcessTickQueue.front();
		amxProcessTickQueue.pop();
		lock.unlock();

		if(getme.errorCode == 250)
		{
			for(std::list<AMX *>::iterator amx = amxList.begin(); amx != end; amx++)
			{
				//forward OnMailSendSuccess(index, to[], subject[], message[], type);
				if(!amx_FindPublic(*amx, "OnMailSendSuccess", &amx_idx))
				{
					amx_Push(*amx, getme.type);
					amx_PushString(*amx, &amxAddress[0], NULL, getme.message.c_str(), NULL, NULL);
					amx_PushString(*amx, &amxAddress[1], NULL, getme.subject.c_str(), NULL, NULL);
					amx_PushString(*amx, &amxAddress[2], NULL, getme.to.c_str(), NULL, NULL);
					amx_Push(*amx, getme.index);

					amx_Exec(*amx, NULL, amx_idx);

					amx_Release(*amx, amxAddress[2]);
					amx_Release(*amx, amxAddress[1]);
					amx_Release(*amx, amxAddress[0]);
				}
			}
		}
		else
		{
			cell addAddress;

			for(std::list<AMX *>::iterator amx = amxList.begin(); amx != end; amx++)
			{
				//forward OnMailSendError(index, to[], subject[], message[], type, error[], error_code);
				if(!amx_FindPublic(*amx, "OnMailSendError", &amx_idx))
				{
					amx_Push(*amx, getme.errorCode);
					amx_PushString(*amx, &amxAddress[0], NULL, getme.error.c_str(), NULL, NULL);
					amx_Push(*amx, getme.type);
					amx_PushString(*amx, &amxAddress[1], NULL, getme.message.c_str(), NULL, NULL);
					amx_PushString(*amx, &amxAddress[2], NULL, getme.subject.c_str(), NULL, NULL);
					amx_PushString(*amx, &addAddress, NULL, getme.to.c_str(), NULL, NULL);
					amx_Push(*amx, getme.index);

					amx_Exec(*amx, NULL, amx_idx);
						
					amx_Release(*amx, addAddress);
					amx_Release(*amx, amxAddress[2]);
					amx_Release(*amx, amxAddress[1]);
					amx_Release(*amx, amxAddress[0]);
				}
			}
		}
	}
}
