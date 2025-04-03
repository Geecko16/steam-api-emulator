typedef struct __attribute__((packed))
{
	void(*run)(void*, void*);
} call_result_vtable;

typedef struct __attribute__((packed))
{
	callback_vtable* vtable;
} call_result;

typedef struct
{
	void* data;
	call_result* base;
	bool free;
	bool registered;
} call_result_slot;
typedef struct
{
	size count;
	call_result_slot* array;
} call_results_list;

private call_results_list call_results_list_init()
{
	call_results_list list = {.count=5, .array=malloc(5 * sizeof(call_result_slot))};
	for(uint8 i = 0; i < 5; i++)
	{
		list.array[i].free = true;
		list.array[i].registered = false;
	}

	return list;
}
private void call_results_list_expand(call_results_list* list)
{
	list->count += 5;
	list->array = realloc(list->array, sizeof(call_result_slot) * (list->count));
	for(size i = 0; i < 5; i++)
	{
		list->array[i].free = true;
		list->array[i].registered = false;
	}
}
private size call_results_list_find_free(call_results_list* list)
{
	for(size i = 0; i < list->count; i++)
	{
		if(list->array[i].free)
		{
			list->array[i].free = false;
			return i;
		}
	}
	size index = list->count;
	list->array[index].free = false;
	call_results_list_expand(list);

	return index;
}
private void call_results_list_destroy(call_results_list* list)
{
	free(list->array);
}

call_results_list call_results = {};