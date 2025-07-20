#include "NavigationUtil.h"
#include <stdexcept>
#include "Plugin_Config.h"

	PDFNavigator::PDFNavigator() {
		pageView = NULL;
		PDDoc pdDoc = NULL;
		nPages = 0;

		visibleRect.bottom = 0;
		visibleRect.left = 0;
		visibleRect.right = 0;
		visibleRect.top = 0;
	}

	void PDFNavigator::closeDoc() {
		pageView = NULL;
		PDDoc pdDoc = NULL;
		nPages = 0;

		visibleRect.bottom = 0;
		visibleRect.left = 0;
		visibleRect.right = 0;
		visibleRect.top = 0;
	}

	void PDFNavigator::openDoc(AVDoc avDoc) {
		pageView = AVDocGetPageView(avDoc);
		PDDoc pdDoc = AVDocGetPDDoc(avDoc);
		
		nPages = PDDocGetNumPages(pdDoc);

		AVPageViewGetAperture(pageView, &visibleRect);
	}

	void PDFNavigator::handleNavigationEvent(uint8_t actionVal) {
		if (pageView == NULL) return;
		switch (actionVal) {
		//No zeros because the corresponding case "nothing" doesn't get sent over the pipeline

		case 1:
			scroll(ScrollDirection::Up);
			break;

		case 2:
			scroll(ScrollDirection::Down);
			break;

		case 3: //Swipeleft
			goPage(PageDirection::Next);
			break;

		case 4: //Swiperight
			goPage(PageDirection::Prev);
			break;

		case 5:
			zoom(ZoomDirection::In);
			break;

		case 6:
			zoom(ZoomDirection::Out);
			break;

		default:
			throw std::runtime_error("ERROR: Pipelinemessage can not be assigned to an action");
			break;
		}
	}


	void PDFNavigator::goPage(PageDirection direction) {
		int curPageIndex = AVPageViewGetPageNum(pageView); //get cur index here because it can change in case of userkeyboard interactions
		if (direction == PageDirection::Next) {
			if (curPageIndex < nPages - 1) {
				curPageIndex++;
			}
		}
		else if (direction == PageDirection::Prev) {
			if (curPageIndex > 0) {
				curPageIndex--;
			}
		}
		else {
			throw std::runtime_error("An error occurred during page change: PageDirection not properly set");
		}
		if (DEBUG_ENABLED) {
			AVAlertNote(("going to page: " + std::to_string(curPageIndex)).c_str());
		}
		AVPageViewGoTo(pageView, curPageIndex);
		AVPageViewDrawNow(pageView);
	}
	

	void PDFNavigator::scroll(ScrollDirection direction) {
		int curStep;
		ASFixed curZoom = AVPageViewGetZoom(pageView);  //scroll distance is relative zoom
		float zoom = ASFixedToFloat(curZoom);

		if (direction == ScrollDirection::Up) {
			curStep = STEP_SIZE * -1 * zoom;
		}
		else if (direction == ScrollDirection::Down) {
			curStep = STEP_SIZE * zoom;
		}
		else {
			throw std::runtime_error("An error occurred during scrolling: Scrolldirection not properly set");
		}

		for (int i = 0; i < SCROLL_COUNT; i++) {
			AVPageViewGetAperture(pageView, &visibleRect);
			visibleRect.top += curStep;
			visibleRect.bottom += curStep;

			if (DEBUG_ENABLED) {
				char buffer[128];
				sprintf(buffer, "scrolling to top-Y: %.4f, bottom-Y: %.4f",
					ASFixedToFloat(visibleRect.top),
					ASFixedToFloat(visibleRect.bottom));
				AVAlertNote(buffer);
			}

			AVPageViewScrollToRect(pageView, &visibleRect, false, false, 0);
			AVPageViewDrawNow(pageView);

			std::this_thread::sleep_for(std::chrono::milliseconds(SCROLL_STEP_DELAY_MS));
		}
	}

	void PDFNavigator::zoom(ZoomDirection direction) {
		ASFixed curZoom = AVPageViewGetZoom(pageView); //get cur zoom here because it can change in case of userkeyboard interactions
		float zoomPercent = ASFixedToFloat(curZoom);

		if (direction == ZoomDirection::In) {
			zoomPercent += ZOOM_STEP;

		}
		else if (direction == ZoomDirection::Out) {
			zoomPercent -= ZOOM_STEP;
		}
		else {
			throw std::runtime_error("An error occurred during zooming: Zoomdirection not properly set");
		}

		if (zoomPercent < ZOOM_MIN) zoomPercent = ZOOM_MIN;
		if (zoomPercent > ZOOM_MAX) zoomPercent = ZOOM_MAX;

		ASFixed newZoom = ASFloatToFixed(zoomPercent);

		if (DEBUG_ENABLED) {
			char buffer[64];
			sprintf(buffer, "zooming to: %.2f", zoomPercent);
			AVAlertNote(buffer);
		}
		
		AVPageViewZoomTo(pageView, AVZoomNoVary, newZoom);
		AVPageViewDrawNow(pageView);
	}

	PDFNavigator navigator = PDFNavigator();

	void DocOpenedCallback(AVDoc doc, ASInt32 error, void* clientData) {
		navigator.openDoc(doc);
	}

	void DocWillCloseCallback(AVDoc doc, void* clientData) {
		navigator.closeDoc();
	}