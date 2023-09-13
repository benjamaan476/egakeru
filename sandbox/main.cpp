#include "pch.h"

int main() {
	egkr::log::init(); 
	LOG_TRACE("Hello");
	LOG_INFO("Hello");
	LOG_ERROR("Hello");
	LOG_WARN("Hello"); 
	LOG_FATAL("Hello"); 
}
