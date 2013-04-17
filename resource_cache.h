#ifndef RESOURCE_CACHE_H_
#define RESOURCE_CACHE_H_

#include <cstdio>

#include <string>
#include <map>

#include "panic.h"

template <typename T>
class resource_cache {
public:
	~resource_cache()
	{
		for (typename std::map<std::string, T *>::iterator i = resource_map.begin(); i != resource_map.end(); i++)
			delete i->second;
	}

	T *operator[](const std::string& path)
	{
		typename std::map<std::string, T *>::iterator i = resource_map.find(path);

		if (i == resource_map.end()) {
			T *resource = new T;

			fprintf(stderr, "loading %s...\n", path.c_str());

			if (!resource->load(path))
				panic("failed to load %s", path.c_str());

			i = resource_map.insert(std::pair<std::string, T *>(path, resource)).first;
		}

		return i->second;
	}

private:
	std::map<std::string, T *> resource_map;
};

#endif // RESOURCE_CACHE_H_
