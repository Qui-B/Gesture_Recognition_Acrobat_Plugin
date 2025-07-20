/*********************************************************************

 ADOBE SYSTEMS INCORPORATED
 Copyright (C) 1998-2006 Adobe Systems Incorporated
 All rights reserved.

 NOTICE: Adobe permits you to use, modify, and distribute this file
 in accordance with the terms of the Adobe license agreement
 accompanying it. If you have received this file from a source other
 than Adobe, then your use, modification, or distribution of it
 requires the prior written permission of Adobe.

 -------------------------------------------------------------------*/
 /**
 \file BasicPlugin.cpp

   - This file implements the functionality of the BasicPlugin.
 *********************************************************************/


 // Acrobat Headers.
#ifndef MAC_PLATFORM
#include "PIHeaders.h"
#endif

#include <sddl.h> //security descriptor
#include <stdexcept> //exceptions
#include <fstream>

#include "NavigationUtil.h" //pdf navigator


/*-------------------------------------------------------
	Constants/Declarations
-------------------------------------------------------*/
// This plug-in's name, you should specify your own unique name here.
const char* MyPluginExtensionName = "ADBE:Gesture Recognition";

const std::string PipeLineName = R"(\\.\pipe\AcrobatGestureRecognition)";

const std::string RelPythonPath = "recognition_system\\.venv\\Scripts\\python.exe";
const std::string Args = "-m recognition_system.src.main";

static HANDLE pipe = NULL;
static ASCallback idleProc = NULL;

/*-------------------------------------------------------
			Settings
-------------------------------------------------------*/
const bool SHOW_CMD_WINDOW = TRUE;
/* A convenient function to add a menu item for your plugin.
*/
ACCB1 ASBool ACCB2 PluginMenuItem(char* MyMenuItemTitle, char* MyMenuItemName);

/*-------------------------------------------------------
	Functions
r-------------------------------------------------------*/
std::string GetPluginDirectory() {
	char path[MAX_PATH];
	HMODULE hModule = nullptr;
	GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
		(LPCTSTR)GetPluginDirectory,
		&hModule);
	GetModuleFileNameA(hModule, path, MAX_PATH);

	std::string fullPath(path);
	size_t lastSlash = fullPath.find_last_of("\\/");
	return fullPath.substr(0, lastSlash);
}

void StartRecognitionSystem() {
	std::string pluginDir = GetPluginDirectory();
	std::string recognitionDir = pluginDir + "\\recognition_system";
	std::string fullPythonPath = recognitionDir + "\\.venv\\Scripts\\python.exe";
	std::string args = "-m src.Main";

	std::string command = "cmd /k \"cd /d \"" + recognitionDir + "\" && .venv\\Scripts\\python.exe " + args + "\"";
	std::ofstream logFile("C:\\Temp\\acrobat_plugin_log.txt", std::ios::app);
	logFile << "Command: " << command << std::endl;
	logFile.close();

	STARTUPINFOA si = { 0 };
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW;
	if (SHOW_CMD_WINDOW) {
		si.wShowWindow = SW_SHOW;
	}
	else {
		si.wShowWindow = SW_HIDE;
	}

	PROCESS_INFORMATION pi;

	char cmdLine[1024];
	if (command.length() < sizeof(cmdLine)) {
		strcpy_s(cmdLine, command.c_str());
	}
	else {
		throw std::runtime_error("Command too long for 'cmdLine' - buffer.");
	}
	strcpy_s(cmdLine, command.c_str());

	BOOL success = CreateProcessA(
		NULL,
		cmdLine,
		NULL,
		NULL,
		FALSE,
		0,
		NULL,
		NULL,
		&si,
		&pi
	);

	if (success) {
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
	}
	else {
		throw std::runtime_error("Failed to start Python process. Error code: " + std::to_string(GetLastError()));
	}
}

HANDLE CreateGesturePipe(std::string pipeName) {
	SECURITY_ATTRIBUTES securityAttr; //create security descriptor to give the pipes rights
	ZeroMemory(&securityAttr, sizeof(securityAttr));
	securityAttr.nLength = sizeof(securityAttr);
	PSECURITY_DESCRIPTOR pSD = nullptr;

	bool sec_descriptor_init_success = ConvertStringSecurityDescriptorToSecurityDescriptorA(
		"D:(A;OICI;GRGW;;;WD)", //allowing GENERIC_READ and GENERIC_WRITE to Everyone
		SDDL_REVISION_1,
		&pSD,
		NULL);
	if (!sec_descriptor_init_success) {
		throw std::runtime_error("Failed initiallizing secrurity descriptor for the gesture pipe");
	}
	securityAttr.lpSecurityDescriptor = pSD;
	securityAttr.bInheritHandle = FALSE;

	HANDLE pipe = CreateNamedPipeA( //create pipe
		pipeName.c_str(),
		PIPE_ACCESS_INBOUND,
		PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
		1, 1024, 1024, 0, &securityAttr
	);
	LocalFree(pSD);
	if (pipe == INVALID_HANDLE_VALUE) {
		throw std::runtime_error("Failed to create pipe. Error code: " + std::to_string(GetLastError()));
	}
	return pipe;
}

void HandlePythonEvent(HANDLE pipe) {
	uint8_t actionVal;
	DWORD bytesRead;
	if (ReadFile(pipe, &actionVal, sizeof(actionVal), &bytesRead, NULL) && bytesRead == sizeof(actionVal)) {
		navigator.handleNavigationEvent(actionVal);
	}
	else {
		throw std::runtime_error("data in pipe found but no data availiable during read");
	}
}

void CheckForPythonEvent(HANDLE pipe) {
	try {
		DWORD bytesAvailable = 0;
		if (PeekNamedPipe(pipe, NULL, 0, NULL, &bytesAvailable, NULL)) {
			if (bytesAvailable > 0) {
				HandlePythonEvent(pipe);
			}
		}
	}
	catch (std::runtime_error e) {
		AVAlertNote(e.what());
	}
}

void IdleCallback(void* clientData) {
	if (pipe != NULL) {
		CheckForPythonEvent(pipe);
	}
}

/* MyPluginSetmenu
** ------------------------------------------------------
**
** Function to set up menu for the plugin.
** It calls a convenient function PluginMenuItem.
** Return true if successful, false if failed.
*/
ACCB1 ASBool ACCB2 MyPluginSetmenu()
{
	// Add a new menu item under Acrobat SDK submenu.
	// The new menu item name is "ADBE:BasicPluginMenu", title is "Basic Plugin".
	// Of course, you can change it to your own.
	return PluginMenuItem("Basic Plugin", "ADBE:BasicPluginMenu");
}


/**		BasicPlugin project is an Acrobat plugin sample with the minimum code
	to provide an environment for plugin developers to get started quickly.
	It can help Acrobat APIs' code testing, too.
		This file implements the functionality of the BasicPlugin. It adds a
	new menu item that will show a message of some simple information about
	the plugin and front PDF document. Users can modify and add code in this
	file only to make a simple plugin of their own.

		  MyPluginCommand is the function to be called when executing a menu.
	This is the entry point for user's code, just add your code inside.

	@see ASExtensionGetRegisteredName
	@see AVAppGetActiveDoc
	@see PDDocGetNumPages
*/


//***MAIN***
ACCB1 void ACCB2 MyPluginCommand(void* clientData)
{
	try {
		pipe = CreateGesturePipe(PipeLineName);
		if (pipe == NULL) {
			throw std::runtime_error("could not conenct to pipe");
		}

		AVAlertNote("Pipeline created");
		StartRecognitionSystem();
		BOOL connected_succesfully = ConnectNamedPipe(pipe, NULL);
		if (!connected_succesfully) {
			throw std::runtime_error("Failed to connect to pipe. Error code: " + std::to_string(GetLastError()));
		}
		//idleProc = ASCallbackCreateProto(IdleCallback);
		AVAppRegisterIdleProc(IdleCallback, NULL, 10); //TODO unnregister when done
	}
	catch (std::runtime_error e) {
		AVAlertNote(e.what());
	}
}

/* MyPluginIsEnabled
** ------------------------------------------------------
** Function to control if a menu item should be enabled.
** Return true to enable it, false not to enable it.
*/
ACCB1 ASBool ACCB2 MyPluginIsEnabled(void* clientData)
{
	// always enabled.
	return true;
	// this code make it is enabled only if there is a open PDF document. 
	/* return (AVAppGetActiveDoc() != NULL); */
}

///Callback proc for mode switch notification.
ACCB1 void ACCB2 AcroAppModeSwitchNotification(void* clientData)
{
	AVAlertNote("Acrobat SDK - Mode Switch Notification");
}