#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <cstring>
#include <filesystem>
#include <iostream>

void __attribute__((visibility("default"))) *dlopen(const char *filename, int flags)
{
	typedef void *(*dlopen_t)(const char *filename, int flags);
	static dlopen_t func;

	if(!func)
		func = (dlopen_t)dlsym(RTLD_NEXT, "dlopen");

	static bool replaced = false;
	if(!replaced && filename && strstr(filename, "linuxsteamrt64/libserver.so"))
	{
		replaced = true;
		std::filesystem::path newpath = (std::filesystem::path(filename).parent_path() / ".." / ".." / "addons" / "metamod" / "bin" / "linuxsteamrt64" / "libserver.so").lexically_normal();
		if(!std::filesystem::exists(newpath))
		{
			std::cerr << "Error: Metamod install not found at " << newpath << std::endl;
			std::abort();
		}
		printf("Redirecting dlopen of %s to %s\n", filename, newpath.c_str());
		return(func(newpath.string().c_str(), flags));
	}
	return(func(filename, flags));
}