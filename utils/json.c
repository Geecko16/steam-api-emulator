enum
{
	json_value_type_null,
	json_value_type_number,
	json_value_type_floating_point,
	json_value_type_string,
	json_value_type_boolean,
	json_value_type_object,
	json_value_type_array,
};

typedef struct __json_object json_object;
typedef struct __json_array json_array;

typedef union
{
	json_object* object;
	json_array* array;
	sint number;
	double floating_point;
	char* string;
	char boolean;
} json_value_storage;

typedef struct
{
	uint8 type;
	json_value_storage storage;
} json_value;

struct __json_array
{
	size items_count;
	json_value* items; 
};

typedef struct
{
	char* name;
	json_value value;
} json_node;

struct __json_object
{
	size nodes_count;
	json_node* nodes;
};

typedef struct
{
	size nodes_count;
	json_node* nodes;

	size heap_size;
	uint8* heap;
} json_string;

private uint8* __reserve_heap(size* heap_size, size reserve_size)
{
	// if(heap->used_size + reserve_size > heap->total_size)
	// {
	// 	heap->total_size += reserve_size + 100;
	// 	heap->buffer = realloc(heap->buffer, heap->total_size + reserve_size + 100);
	// }
	// uint8* address = &heap->buffer[heap->used_size];
	// heap->used_size += reserve_size;

	// return address;
	*heap_size += reserve_size;
	return malloc(reserve_size);
}
private uint8* __replace_with_heap(void** value, size value_size, uint8* heap, size* current_offset)
{
	memcpy(&heap[*current_offset], *value, value_size);
	free(*value);
	*value = &heap[*current_offset];
	*current_offset += value_size;
}

private void __json_parse_object(char* string, json_object* json, size* heap_size);
private size __parse_value(char* string, size* heap_size, json_value* value, char end_character)
{
	size offset = 0;
	if(string[0] == '{')
	{
		value->type = json_value_type_object;
		value->storage.object = __reserve_heap(heap_size, sizeof(json_string));
		__json_parse_object(string, value->storage.object, heap_size);
	} 
	else if (string[0] == '[')
	{
		size length = 1;
		size items_count = 1;
		while(string[length] != ']')
		{
			if(string[length] == ',')
			{
				items_count += 1;
			}
			length += 1;
		}

		value->type = json_value_type_array;
		value->storage.array = __reserve_heap(heap_size, sizeof(json_array));
		value->storage.array->items_count = items_count;
		value->storage.array->items = __reserve_heap(heap_size, sizeof(json_value) * items_count);
		offset = 1;
		for(size i = 0; i < items_count; i++)
		{
			offset += __parse_value(&string[offset], heap_size, &value->storage.array->items[i], ']');
			offset += 1;
		}
	}
	else if(string[0] == '"')
	{
		size length = 0;
		while(string[1+length] != '"')
		{
			length += 1;
		}

		value->type = json_value_type_string;
		value->storage.string = __reserve_heap(heap_size, length+1);
		memcpy(value->storage.string, &string[1], length);
		value->storage.string[length] = 0;

		offset += length+2;
	}
	else
	{
		size full_value_length = 1;
		while(string[full_value_length] != ',' && string[full_value_length] != end_character)
		{
			full_value_length += 1;
		}
		size value_length = full_value_length;
		while(string[value_length] <= 32)
		{
			value_length -= 1;
		}
		if(value_length == 4)
		{
			if(string_equal(&string[offset], "null", 4))
			{
				value->type = json_value_type_null;
			}
			else if(string_equal(&string[offset], "true", 4))
			{
				value->type = json_value_type_boolean;
				value->storage.boolean = true;
			}
			else
			{
				goto number;
			}
			goto complete;
		}
		else if(value_length == 5)
		{
			if(string_equal(&string[offset], "false", 5))
			{
				value->type = json_value_type_boolean;
				value->storage.boolean = false;
			}
			else
			{
				goto number;
			}
			goto complete;
		}
		number:
		bool floating_point = 0;
		for(size i = 0; i < value_length; i++)
		{
			if(string[offset + i] == '.')
			{
				floating_point = 1;
				break;
			}
		}
		if(floating_point)
		{
			error(0, 0, "JSON floating point values aren't supported yet.");
		}
		else
		{
			uint8 multiplier = 1;
			if(string[offset] == '-')
			{
				multiplier = -1;
				offset += 1;
				value_length -= 1;
				full_value_length -= 1;
			}
			value->type = json_value_type_number;
			value->storage.number = 0;
			for(size i = value_length; i > 0; i--)
			{
				value->storage.number *= 10;
				value->storage.number += string[offset+i-1] - 48;
			}
			value->storage.number *= multiplier;
		}
		complete:
		offset += full_value_length;
	}

	return offset;
}
private void __json_parse_object(char* string, json_object* object, size* heap_size)
{
	size total_nodes_count = 5;
	object->nodes = malloc(sizeof(json_node) * 5);
	object->nodes_count = 0;

	size offset = 1;
	while(string[offset])
	{
		while(string[offset] <= 32)
		{
			offset += 1;
		}
		if (string[offset] == '}')
		{
			break;
		}
		else
		{
			if(object->nodes_count == total_nodes_count)
			{
				total_nodes_count += 5;
				object->nodes = realloc(object->nodes, sizeof(json_node) * total_nodes_count);
			}
			size current_node = object->nodes_count;
			object->nodes_count += 1;
			offset += 1;
			size name_size = 0;
			while(string[offset+name_size] != '"')
			{
				name_size += 1;
			}
			object->nodes[current_node].name = __reserve_heap(heap_size, name_size+1);
			memcpy(object->nodes[current_node].name, &string[offset], name_size);
			object->nodes[current_node].name[name_size] = 0;
			offset += name_size + 1;

			while(string[offset] != ':')
			{
				offset += 1;
			}
			offset += 1;
			while(string[offset] <= 32)
			{
				offset += 1;
			}
			offset += __parse_value(&string[offset], heap_size, &object->nodes[current_node].value, '}');
		}
		if(string[offset] != ',')
		{
			break;
		}
		offset += 1;
	}

	if(object->nodes_count != total_nodes_count)
	{
		object->nodes = realloc(object->nodes, sizeof(json_node) * object->nodes_count);	
	}
	*heap_size += sizeof(json_node) * object->nodes_count;
}

private void __json_optimize_object(json_object* object, uint8* heap, size* current_offset);
private void __json_optimize_value(json_value* value, uint8* heap, size* current_offset)
{
	if(value->type == json_value_type_string)
	{
		__replace_with_heap(&value->storage.string, string_null_terminated_length(value->storage.string)+1, heap, current_offset);
	}
	else if(value->type == json_value_type_object)
	{
		__replace_with_heap(&value->storage.object, sizeof(json_object), heap, current_offset);
		__json_optimize_object(value->storage.object, heap, current_offset);
	}
	else if(value->type == json_value_type_array)
	{
		__replace_with_heap(&value->storage.array, sizeof(json_array), heap, current_offset);
		__replace_with_heap(&value->storage.array->items, sizeof(json_value) * value->storage.array->items_count, heap, current_offset);
		for(size i = 0; i < value->storage.array->items_count; i++)
		{
			__json_optimize_value(&value->storage.array->items[i], heap, current_offset);
		}
	}
}
private void __json_optimize_object(json_object* object, uint8* heap, size* current_offset)
{
	__replace_with_heap(&object->nodes, sizeof(json_node) * object->nodes_count, heap, current_offset);
	for(size i = 0; i < object->nodes_count; i++)
	{
		__replace_with_heap(&object->nodes[i].name, string_null_terminated_length(object->nodes[i].name)+1, heap, current_offset);
		__json_optimize_value(&object->nodes[i].value, heap, current_offset);
	}
}

private void json_parse_string(char* string, json_string* json)
{
	__json_parse_object(string, json, &json->heap_size);
	json->heap = malloc(json->heap_size);
	size current_offset = 0;
	__json_optimize_object(json, json->heap, &current_offset);
}

typedef struct
{
	size total_size;
	size used_size;
	char* string;
} __dynamic_string_output;

void __output_add_character(__dynamic_string_output* output, char character)
{
	if(output->used_size == output->total_size)
	{
		output->total_size += 101;
		output->string = realloc(output->string, output->total_size);
	}
	output->string[output->used_size] = character;
	output->used_size += 1;
}
void __output_add_null_terminated_string(__dynamic_string_output* output, char* string)
{
	size string_length = string_null_terminated_length(string);

	if(output->used_size + string_length >= output->total_size)
	{
		output->total_size += 100 + string_length;
		output->string = realloc(output->string, output->total_size);
	}
	memcpy(&output->string[output->used_size], string, string_length);
	output->used_size += string_length;
}
void __output_add_string(__dynamic_string_output* output, char* string, size string_length)
{
	if(output->used_size + string_length >= output->total_size)
	{
		output->total_size += 100 + string_length;
		output->string = realloc(output->string, output->total_size);
	}
	memcpy(&output->string[output->used_size], string, string_length);
	output->used_size += string_length;
}

private void __output_add_json_object(__dynamic_string_output* output, json_object* object);
private void __output_add_value(__dynamic_string_output* output, json_value* value)
{
	if(value->type == json_value_type_null)
	{
		__output_add_string(output, "null", 4);
	}
	else if(value->type == json_value_type_number)
	{
		if(value->storage.number)
		{
			char* string_start = &output->string[output->used_size];
			uint8 length = 0;
			size next_number = value->storage.number;
			size number = value->storage.number;
			do
			{
				next_number /= 10;
				__output_add_character(output, number - next_number * 10 + '0');
				number = next_number;
				length++;
			} while(next_number);
			for(size i = 0; i < length/2; i++)
			{
				char swap = string_start[i];
				string_start[i] = string_start[length-i-1];
				string_start[length-i-1] = swap;
			}
		}
		else
		{
			__output_add_character(output, '0');
		}
	}
	else if(value->type == json_value_type_string)
	{
		__output_add_character(output, '"');
		__output_add_null_terminated_string(output, value->storage.string);
		__output_add_character(output, '"');
	}
	else if(value->type == json_value_type_boolean)
	{
		if(value->storage.boolean)
		{
			__output_add_string(output, "true", 4);	
		}
		else
		{
			__output_add_string(output, "false", 5);
		}
	}
	else if(value->type == json_value_type_object)
	{
		__output_add_json_object(output, value->storage.object);
	}
	else if(value->type == json_value_type_array)
	{
		__output_add_character(output, '[');
		if(value->storage.array->items_count)
		{
			for(size i = 0; i < value->storage.array->items_count-1; i++)
			{
				__output_add_value(output, &value->storage.array->items[i]);
				__output_add_character(output, ',');
			}
			__output_add_value(output, &value->storage.array->items[value->storage.array->items_count-1]);
		}
		__output_add_character(output, ']');
	}
}
private void __output_add_node(__dynamic_string_output* output, json_node* node)
{
	__output_add_character(output, '"');
	__output_add_null_terminated_string(output, node->name);
	__output_add_character(output, '"');
	__output_add_character(output, ':');
	__output_add_value(output, &node->value);
}

private void __output_add_json_object(__dynamic_string_output* output, json_object* object)
{
	__output_add_character(output, '{');
	for(size i = 0; i < object->nodes_count-1; i++)
	{
		__output_add_node(output, &object->nodes[i]);
		__output_add_character(output, ',');
	}
	__output_add_node(output, &object->nodes[object->nodes_count-1]);
	__output_add_character(output, '}');
}

private char* json_to_string(json_string* json, size* string_length)
{
	__dynamic_string_output output = { .total_size = 100 };
	output.string = malloc(100);

	__output_add_json_object(&output, json);
	__output_add_character(&output, 0);

	if(output.total_size != output.used_size)
	{
		output.string = realloc(output.string, output.used_size);
	}

	*string_length = output.used_size;
	return output.string;
}

bool json_string_find_node(json_string* json, char* name, size* result)
{
	for(size i = 0; i < json->nodes_count; i++)
	{
		if (string_null_terminated_equal(json->nodes[i].name, name))
		{
			*result = i;
			return true;
		}
	}
	return false;
}
