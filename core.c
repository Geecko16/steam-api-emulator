#include <error.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <dirent.h>

char* game_metadata_path;

#include "common.h"
#include "./utils/string.c"
#include "./utils/json.c"
#include "callbacks.c"
#include "call_results.c"

#ifdef inventory_used
char* game_items_data_file_path;
#include "inventory.c"
#endif

#include "./interfaces/interfaces.c"

size context_counter = 1;

typedef void(*context_callback)(void*);
typedef struct __attribute__((packed))
{
	context_callback callback;
	size counter;
	uint8 data[];
} context;

public bool SteamAPI_Init()
{
	callbacks_data[CALLBACK_ID_FRIENDS_GAME_OVERLAY_ACTIVATED] = &game_overlay_activated_data;
	callbacks_data[CALLBACK_ID_USER_STATS_RECEIVED] = &user_stats_received_data;
	callbacks_data[CALLBACK_ID_USER_STATS_STORED] = &user_stats_stored_data;

	callbacks = callbacks_list_init();
	call_results = call_results_list_init();

	game_coordinator_messages = game_coordinator_messages_list_init();

	char* root_path = getenv("STEAM_API_EMULATOR");
	size root_path_length = string_null_terminated_length(root_path);
	char* game_folder = macro_to_string(steam_app_id)"/";
	size game_folder_length = sizeof(macro_to_string(steam_app_id)"/")-1;
	game_metadata_path = string_concatenate(root_path, root_path_length, game_folder, game_folder_length);
	#ifdef inventory_used
	game_items_data_file_path = string_concatenate(game_metadata_path, root_path_length+game_folder_length, "items.dat", sizeof("items.dat"));
	inventory_init();
	#endif

	return true;
}
// Deprecated? Not mentioned anywhere in Steamworks API documentation.
public bool SteamAPI_IsSteamRunning()
{
	return true;
}
public bool SteamAPI_RestartAppIfNecessary(uint32 id)
{
	return false;
}
public void SteamAPI_Shutdown()
{
	callbacks_list_destroy(&callbacks);
	call_results_list_destroy(&call_results);
	return;
}

public uint32 SteamAPI_GetHSteamPipe()
{
	return 1;
}
public uint32 SteamAPI_GetHSteamUser()
{
	return 1;
}

public void* SteamInternal_ContextInit(context* ctx)
{
	if(ctx->counter != context_counter)
	{
		ctx->callback(&ctx->data);
		ctx->counter = context_counter;
	}
	return &ctx->data;
}

public structure_pointer SteamInternal_CreateInterface(char* version)
{
	return create_interface(version);
}

public void SteamAPI_RunCallbacks()
{
	for(size i = 0; i < callbacks.count; i++)
	{
		if(!callbacks.array[i].free && callbacks_status[callbacks.array[i].id])
		{
			callbacks_status[callbacks.array[i].id] = false;
			callbacks.array[i].base->vtable->run(callbacks.array[i].base, callbacks_data[callbacks.array[i].id]);
		}
	}
	for(size i = 0; i < call_results.count; i++)
	{
		if(!call_results.array[i].free && call_results.array[i].registered)
		{
			call_results.array[i].base->vtable->run(call_results.array[i].base, call_results.array[i].data);
		}
	}
}

public void SteamAPI_RegisterCallback(callback* c, uint32 id)
{
	callbacks_list_register(&callbacks, c, id);
}
public void SteamAPI_UnregisterCallback(callback* c)
{
	callbacks_list_unregister(&callbacks, c);
}
public void SteamAPI_RegisterCallResult(call_result* c, uint64 id)
{
	call_results.array[id].base = c;
	call_results.array[id].registered = true;
}
public void SteamAPI_UnregisterCallResult(call_result* c)
{
	for(size i = 0; i < call_results.count; i++)
	{
		if(call_results.array[i].base == c)
		{
			call_results.array[i].free = true;
			call_results.array[i].registered = false;
			break;
		}
	}
}