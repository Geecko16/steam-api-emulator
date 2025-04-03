typedef struct
{
	size global_id;
	size local_id;
	size quantity;
} inventory_item;

typedef struct
{
	size items_count;
	inventory_item* items;
} inventory;

inventory inventory_data = {};
FILE* items_data_file;

private void inventory_init()
{
	items_data_file = fopen(game_items_data_file_path, "r+");
	fread(&inventory_data.items_count, BYTES_COUNT, 1, items_data_file);
	if(inventory_data.items_count)
	{
		inventory_data.items = malloc(sizeof(inventory_item) * inventory_data.items_count);
		fread(inventory_data.items, sizeof(inventory_item), inventory_data.items_count, items_data_file);	
	}
}