#pragma once

#ifndef MAC_PLATFORM
#include "PIHeaders.h"    //acrobat
#include "AVCalls.h"      //for listeners
#include "ASCalls.h"//
#endif
#include <thread>
#include <chrono>  
#include <string>

//Navigator settings are in Plugin_Config.h
class PDFNavigator {
private:
	AVPageView pageView;
	int nPages;

	AVDevRect visibleRect;
public:
	PDFNavigator();

	//methods/enums for changing the PDF-view
	enum class ScrollDirection {
		Up,
		Down
	};
	enum class PageDirection {
		Next,
		Prev
	};
	enum class ZoomDirection {
		In,
		Out
	};
	void scroll(ScrollDirection direction);
	void zoom(ZoomDirection direction);
	void goPage(PageDirection direction);

	//main method for handling input of the recognition system. Uses particular pdf chhange method in response to the python event.
	void handleNavigationEvent(uint8_t actionVal);

	//for keeping track of the current doc inside the navigator
	void closeDoc();
	void openDoc(AVDoc avDoc);
};
//wrapper methods needed for registering listeners in BasicPluginInit.cpp
extern void DocOpenedCallback(AVDoc doc, ASInt32 error, void* clientData);
extern void DocWillCloseCallback(AVDoc doc, void* clientData);

extern PDFNavigator navigator; //extern field for accesability

