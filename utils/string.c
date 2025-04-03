private char* string_concatenate(char* string1, size length1, char* string2, size length2)
{
	char* output_string = malloc(length1 + length2 + 1);
	memcpy(output_string, string1, length1);
	memcpy(&output_string[length1], string2, length2);
	output_string[length1+length2] = 0;

	return output_string;
}
private bool string_equal(char* string1, char* string2, size length)
{
	for(size i = 0; i < length; i++)
	{
		if(string1[i] != string2[i])
		{
			return false;
		}
	}
	return true;
}

private size string_null_terminated_length(char* string)
{
	size length = 0;
	while(string[length])
	{
		length += 1;
	}
	return length;
}
private bool string_null_terminated_equal(char* string1, char* string2)
{
	if(string_null_terminated_length(string1) != string_null_terminated_length(string2))
	{
		return false;
	}

	uint64 i = 0;
	while(string1[i] != 0)
	{
		if(string1[i] != string2[i])
		{
			return false;
		}
		i++;
	}
	return true;
}
