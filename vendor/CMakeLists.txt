set(SAFETYHOOK_FETCH_ZYDIS ON)
add_subdirectory(safetyhook)

set_target_properties(
	safetyhook
	Zydis
	Zycore
	PROPERTIES FOLDER Libraries
)