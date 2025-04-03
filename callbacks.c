typedef struct __attribute__((packed))
{
	void(*run)(void*, void*);
} callback_vtable;

typedef struct __attribute__((packed))
{
	callback_vtable* vtable;
} callback;

typedef struct __attribute__((packed))
{
	bool active;
} friends_game_overlay_activated;
typedef struct __attribute__((packed))
{
	uint64 game_id;
	uint64 result;
	uint64 user_id;	
} user_stats_received;
typedef struct __attribute__((packed))
{
	uint64 game_id;
	uint64 result;
} user_stats_stored;

friends_game_overlay_activated game_overlay_activated_data = { .active = 0 };
user_stats_received user_stats_received_data = { .game_id = 1, .result = 1, .user_id = 1 };
user_stats_stored user_stats_stored_data = { .game_id = 1, .result = 1 };

#define CALLBACKS_COUNT 0x4b4

#define CALLBACK_ID_USER_VALIDATE_AUTH_TICKET_RESPONSE 0x8f

#define CALLBACK_ID_FRIENDS_PERSONA_STATE_CHANGE 0x130
#define CALLBACK_ID_FRIENDS_GAME_OVERLAY_ACTIVATED 0x14b
#define CALLBACK_ID_FRIENDS_GAME_INVITE_SELECTED 0x14d

#define CALLBACK_ID_MATCHMAKING_LOBBY_DATA_UPDATE 0x1f9
#define CALLBACK_ID_MATCHMAKING_LOBBY_CHAT_UPDATE 0x1fa

#define CALLBACK_ID_USER_STATS_RECEIVED 0x44d
#define CALLBACK_ID_USER_STATS_STORED 0x44e

#define CALLBACK_ID_NETWORKING_P2P_SESSION_REQUEST 0x4b2
#define CALLBACK_ID_NETWORKING_P2P_SESSION_CONNECT_FAIL 0x4b3

bool callbacks_status[CALLBACKS_COUNT] = {};
void* callbacks_data[CALLBACKS_COUNT] = {};

typedef struct
{
	bool free;
	uint32 id;
	callback* base;
} callback_slot;
typedef struct
{
	size count;
	callback_slot* array;
} callbacks_list;

private callbacks_list callbacks_list_init()
{
	callbacks_list list = {.count=5, .array=malloc(5 * sizeof(callback_slot))};
	for(uint8 i = 0; i < 5; i++)
	{
		list.array[i].free = true;
	}

	return list;
}
private void callbacks_list_expand(callbacks_list* list)
{
	list->count += 5;
	list->array = realloc(list->array, (list->count) * sizeof(callback_slot));
	for(size i = 0; i < 5; i++)
	{
		list->array[i].free = true;
	}
}
private void callbacks_list_register(callbacks_list* list, callback* c, uint32 id)
{
	for(size i = 0; i < list->count; i++)
	{
		if(list->array[i].free)
		{
			list->array[i].free = false;
			list->array[i].id = id;
			list->array[i].base = c;
			return;
		}
	}
	size index = list->count;
	callbacks_list_expand(list);
	list->array[index].free = false;
	list->array[index].id = id;
	list->array[index].base = c;
}
private void callbacks_list_unregister(callbacks_list* list, callback* c)
{
	for(size i = 0; i < list->count; i++)
	{
		if(!list->array[i].free && list->array[i].base == c)
		{
			list->array[i].free = true;
			return;
		}
	}
}
private void callbacks_list_destroy(callbacks_list* list)
{
	free(list->array);
}

callbacks_list callbacks = {};