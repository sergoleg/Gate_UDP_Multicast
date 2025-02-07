/**
 * @file    Manager.cpp
 * @ingroup LoggerCpp
 * @brief   The static class that manage the registered channels and outputs
 *
 * Copyright (c) 2013-2018 Sebastien Rombauts (sebastien.rombauts@gmail.com)
 *
 * Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
 * or copy at http://opensource.org/licenses/MIT)
 */

#include <Manager.h>
#include <Exception.h>

#include <OutputConsole.h>
#include <OutputFile.h>

#ifdef __unix__
#include <OutputSyslog.h>
#endif
#ifdef WIN32
#include <OutputDebug.h>
#endif

#include <stdexcept>
#include <string>

namespace Log {

Channel::Map Manager::mChannelMap;
Output::Vector Manager::mOutputList;
Log::Level Manager::mDefaultLevel = Log::eDebug;

// Create and configure the Output objects.
void Manager::configure(const Config::Vector& aConfigList) {
	// List of all Output class ; those names are in the form
	// - "class Log::OutputConsole" under Visual Studio 2010
	// - "N3Log13OutputConsoleE" under GCC
	std::string outputConsole = typeid(OutputConsole).name();
	std::string outputFile = typeid(OutputFile).name();
#ifdef __unix__
	std::string outputSyslog = typeid(OutputSyslog).name();
#endif
#ifdef WIN32
	std::string outputDebug = typeid(OutputDebug).name();
#endif

	Config::Vector::const_iterator iConfig;
	for (iConfig = aConfigList.begin(); iConfig != aConfigList.end();
			++iConfig) {
		Output::Ptr outputPtr;
		const std::string& configName = (*iConfig)->getName();

		// Compare the provided Output name with the known class name
		if (std::string::npos != outputConsole.find(configName)) {
			outputPtr.reset(new OutputConsole((*iConfig)));
		} else if (std::string::npos != outputFile.find(configName)) {
			outputPtr.reset(new OutputFile((*iConfig)));
#ifdef __unix__
		} else if (std::string::npos != outputSyslog.find(configName)) {
			outputPtr.reset(new OutputSyslog((*iConfig)));
#endif
#ifdef WIN32
		} else if (std::string::npos != outputDebug.find(configName)) {
			outputPtr.reset(new OutputDebug((*iConfig)));
#endif
		} else {
			LOGGER_THROW("Unknown Output name '" << configName << "'");
		}
		mOutputList.push_back(outputPtr);
	}
}

// Destroy the Output objects.
void Manager::terminate(void) {
	// This effectively destroys the Output objects
	mOutputList.clear();
}

// Return the Channel corresponding to the provided name
Channel::Ptr Manager::get(const char* apChannelName) {
	Channel::Ptr ChannelPtr;
	Channel::Map::iterator iChannelPtr = mChannelMap.find(apChannelName);

	if (mChannelMap.end() != iChannelPtr) {
		ChannelPtr = iChannelPtr->second;
	} else {
		/// @todo Add a basic thread-safety security (throw if multiple threads create Loggers)
		ChannelPtr.reset(new Channel(apChannelName, mDefaultLevel));
		mChannelMap[apChannelName] = ChannelPtr;
	}

	return ChannelPtr;
}

// Output the Log to all the active Output objects.
void Manager::output(const Channel::Ptr& aChannelPtr, const Log& aLog) {
	Output::Vector::iterator iOutputPtr;

	for (iOutputPtr = mOutputList.begin(); iOutputPtr != mOutputList.end();
			++iOutputPtr) {
		(*iOutputPtr)->output(aChannelPtr, aLog);
	}
}

// Serialize the current Log::Level of Channel objects and return them as a Config instance
Config::Ptr Manager::getChannelConfig(void) {
	Config::Ptr ConfigPtr(new Config("ChannelConfig"));

	Channel::Map::const_iterator iChannel;
	for (iChannel = mChannelMap.begin(); iChannel != mChannelMap.end();
			++iChannel) {
		ConfigPtr->setValue(iChannel->first.c_str(),
				Log::toString(iChannel->second->getLevel()));
	}

	return ConfigPtr;
}

// Set the Log::Level of Channel objects from the provided Config instance
void Manager::setChannelConfig(const Config::Ptr& aConfigPtr) {
	const Config::Values& ConfigValues = aConfigPtr->getValues();

	Config::Values::const_iterator iValue;
	for (iValue = ConfigValues.begin(); iValue != ConfigValues.end();
			++iValue) {
		Manager::get(iValue->first.c_str())->setLevel(
				Log::toLevel(iValue->second.c_str()));
	}
}

} // namespace Log

