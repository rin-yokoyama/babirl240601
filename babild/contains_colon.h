int contains_colon(const char str[])
{
    int i = 0;
    while (str[i] != '\0')
    {
        if (str[i] == ':')
        {
            return 1; // Return true if colon is found
        }
        i++;
    }
    return 0; // Return false if no colon is found
}

