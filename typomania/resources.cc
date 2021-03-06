#include <cstdio>

#include <string>
#include <memory>
#include <unordered_map>

#include "panic.h"

#include "font.h"
#include "gl_texture.h"
#include "gl_program.h"
#include "resources.h"

template <typename T>
class resource_cache
{
public:
	T *operator[](const std::string& path)
	{
		auto it = resource_map.find(path);

		if (it == resource_map.end()) {
			std::unique_ptr<T> resource(new T);

			fprintf(stderr, "loading %s...\n", path.c_str());

			if (!resource->load(path))
				panic("failed to load %s", path.c_str());

			it = resource_map.insert(std::make_pair(path, std::move(resource))).first;
		}

		return it->second.get();
	}

private:
	std::unordered_map<std::string, std::unique_ptr<T>> resource_map;
};

const font *get_font(const std::string& path)
{
	static resource_cache<font> cache;
	return cache[path];
}

const gl::texture *get_texture(const std::string& path)
{
	static resource_cache<gl::texture> cache;
	return cache[path];
}

const gl::program *get_program(const std::string& path)
{
	static resource_cache<gl::program> cache;
	return cache[path];
}
