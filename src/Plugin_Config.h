#include <stdbool.h>
//-------------------------------------------------------
			//Global Settings
//-------------------------------------------------------*/
//used for sharing setting status
extern bool DEBUG_ENABLED; //only dummy values gets defined in BasicPlugin.cpp
extern bool SHOW_CMD_WINDOW;

//Naming
constexpr char* PLUGIN_TITLE = "Gesture Recognition";
constexpr char* DEBUG_TITLE = "Debug";
constexpr char* WINDOW_ONLY_TITLE = "Window Only";
constexpr char* NORMAL_TITLE = "Normal";
//-------------------------------------------------------
			//Navigator Settings
//-------------------------------------------------------
//Zoom
constexpr int ZOOM_INTENSITY = 4; //Acrobat reader only support 25% steps 4 is the perfect value for that
constexpr float ZOOM_MIN = 0.25f;
constexpr float ZOOM_MAX = 2.0f;
constexpr float ZOOM_STEP = 0.25f;

//Scroll
constexpr int SCROLL_COUNT = 30; //num of yOffset changes per scroll
constexpr int SCROLL_STEP_DELAY_MS = 1; //delay between yOffset changes
constexpr int STEP_SIZE = 12;

struct DebugConfig {
	bool debugEnabled;
	bool windowEnabled;
};