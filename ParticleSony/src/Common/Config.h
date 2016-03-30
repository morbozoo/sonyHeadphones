/*
Copyright (c) 2015, The Barbarian Group
All rights reserved.

This code is designed for use with the Cinder C++ library, http://libcinder.org

Redistribution and use in source and binary forms, with or without modification, are permitted provided that
the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this list of conditions and
the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
the following disclaimer in the documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include "cinder/DataSource.h"
#include "cinder/DataTarget.h"
#include "cinder/Log.h"
#include "cinder/Utilities.h"

#include "jsoncpp/json.h"

typedef std::shared_ptr<class Config> ConfigRef;

class Config {
public:
	class Options {
		friend class Config;
	public:
		Options() : mSetIfDefault(false), mWriteIfSet(false) {}

		//! If \a enabled, add the default value to the configuration file if the key was not found.
		Options& setIfDefault(bool enabled = true) { mSetIfDefault = enabled; return *this; }
		//! If \a enabled, write the config file immediately after setting the new value.
		Options& writeIfSet(bool enabled = true) { mWriteIfSet = enabled; return *this; }

	private:
		bool  mSetIfDefault;
		bool  mWriteIfSet;
	};

public:
	~Config() {}

	static Config& instance()
	{
		static Config sInstance;
		return sInstance;
	}

	void setOptions(const Options &options) { mOptions = options; }

	bool read(const ci::DataSourceRef &source)
	{
		mSource = source;

		if (mSource->isFilePath()) {
			mTarget = ci::DataTargetPath::createRef(mSource->getFilePath());
		}
		else {
			// We can only write to files on disk.
			mTarget.reset();
		}

		try {
			std::string data = ci::loadString(source);

			Json::Reader reader;
			reader.parse(data, mRoot);

			mDirty = false;
			return true;
		}
		catch (...) {
			return false;
		}
	}

	bool write() { return write(mTarget); }

	bool write(const ci::DataTargetRef &target)
	{
		if (target) {
			const ci::fs::path &path = target->getFilePath();

			if (!ci::fs::exists(path)) {
				ci::fs::create_directories(path.parent_path());
				mDirty = true;
			}

			if (mDirty) {
				Json::StyledWriter writer;
				std::string data = writer.write(mRoot);

				CI_LOG_V(data);

				auto stream = target->getStream();
				if (stream) {
					stream->seekAbsolute(0);
					stream->write(data);
					mDirty = false;
				}
			}

			return true;
		}

		return false;
	}

	template<typename T>
	T get(const std::string &category, const std::string &key, const T &defaultValue)
	{
		auto str = mRoot["configuration"][category][key].asString();

		if (str.empty()) {
			if (mOptions.mSetIfDefault)
				set(category, key, defaultValue);

			return defaultValue;
		}

		return ci::fromString<T>(str);
	}

	template<typename T>
	bool set(const std::string &category, const std::string &key, const T &value)
	{
		mRoot["configuration"][category][key] = Json::Value(value);
		mDirty = true;

		if (mOptions.mWriteIfSet)
			write();

		return true;
	}

private:
	Config() : mDirty(false) {}
	Config(const Options &options) : mDirty(false), mOptions(options) {}

	bool               mDirty;
	Options            mOptions;

	Json::Value        mRoot;

	ci::DataSourceRef  mSource;
	ci::DataTargetRef  mTarget;
};

template<>
inline bool Config::get<bool>(const std::string &category, const std::string &key, const bool &defaultValue)
{
	return get(category, key, (int)defaultValue) != 0;
}

template<>
inline ci::ivec2 Config::get<ci::ivec2>(const std::string &category, const std::string &key, const ci::ivec2 &defaultValue)
{
	ci::ivec2 v;
	v.x = get(category, key + "_x", defaultValue.x);
	v.y = get(category, key + "_y", defaultValue.y);
	return v;
}

template<>
inline ci::ivec3 Config::get<ci::ivec3>(const std::string &category, const std::string &key, const ci::ivec3 &defaultValue)
{
	ci::ivec3 v;
	v.x = get(category, key + "_x", defaultValue.x);
	v.y = get(category, key + "_y", defaultValue.y);
	v.z = get(category, key + "_z", defaultValue.z);
	return v;
}

template<>
inline ci::vec2 Config::get<ci::vec2>(const std::string &category, const std::string &key, const ci::vec2 &defaultValue)
{
	ci::vec2 v;
	v.x = get(category, key + "_x", defaultValue.x);
	v.y = get(category, key + "_y", defaultValue.y);
	return v;
}

template<>
inline ci::vec3 Config::get<ci::vec3>(const std::string &category, const std::string &key, const ci::vec3 &defaultValue)
{
	ci::vec3 v;
	v.x = get(category, key + "_x", defaultValue.x);
	v.y = get(category, key + "_y", defaultValue.y);
	v.z = get(category, key + "_z", defaultValue.z);
	return v;
}

template<>
inline ci::vec4 Config::get<ci::vec4>(const std::string &category, const std::string &key, const ci::vec4 &defaultValue)
{
	ci::vec4 v;
	v.x = get(category, key + "_x", defaultValue.x);
	v.y = get(category, key + "_y", defaultValue.y);
	v.z = get(category, key + "_z", defaultValue.z);
	v.w = get(category, key + "_w", defaultValue.w);
	return v;
}

template<>
inline bool Config::set<bool>(const std::string &category, const std::string &key, const bool &value)
{
	return set(category, key, (int)value);
}

template<>
inline bool Config::set<ci::ivec2>(const std::string &category, const std::string &key, const ci::ivec2 &value)
{
	set(category, key + "_x", value.x);
	set(category, key + "_y", value.y);
	return true;
}

template<>
inline bool Config::set<ci::ivec3>(const std::string &category, const std::string &key, const ci::ivec3 &value)
{
	set(category, key + "_x", value.x);
	set(category, key + "_y", value.y);
	set(category, key + "_z", value.z);
	return true;
}

template<>
inline bool Config::set<ci::vec2>(const std::string &category, const std::string &key, const ci::vec2 &value)
{
	set(category, key + "_x", value.x);
	set(category, key + "_y", value.y);
	return true;
}

template<>
inline bool Config::set<ci::vec3>(const std::string &category, const std::string &key, const ci::vec3 &value)
{
	set(category, key + "_x", value.x);
	set(category, key + "_y", value.y);
	set(category, key + "_z", value.z);
	return true;
}

template<>
inline bool Config::set<ci::vec4>(const std::string &category, const std::string &key, const ci::vec4 &value)
{
	set(category, key + "_x", value.x);
	set(category, key + "_y", value.y);
	set(category, key + "_z", value.z);
	set(category, key + "_w", value.w);
	return true;
}


static inline Config& config() { return Config::instance(); }
