#include "os_utils.h"
#include <CoreGraphics/CoreGraphics.h>

float idle_time() {
	CFTimeInterval idle = CGEventSourceSecondsSinceLastEventType(kCGEventSourceStateHIDSystemState, kCGAnyInputEventType);
	if (idle < 0.0) {
		return 0.0f;
	}
	return (float)idle;
}

void dispose_os_resources() {
	// No resources to dispose for the Core Graphics implementation
}

