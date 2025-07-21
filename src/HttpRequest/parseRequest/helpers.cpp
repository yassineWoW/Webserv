#include "HttpRequest.hpp"

bool    find_and_get(std::string &request, std::string &dest, std::string delimeter)
{
    std::size_t index = request.find(delimeter);    
    if ( index == std::string::npos ) {
        return (false);
    }
    dest = request.substr(0, index);
    request.erase(0, index + delimeter.size());
    return (true);
}

bool check_valid_spaces(std::string &to_check, int authorized_space_number) 
{
    int counter = 0;
    while (to_check[counter] == ' ') {
        counter ++;
    }
    to_check.erase(0, counter);
    return  (counter > authorized_space_number ? false : true); // "host:value" || "host: value"
}



std::string to_lower(std::string header)
{
    for (size_t i = 0; i < header.length()- 1; i++)
    {
        header[i] = std::tolower(header[i]);
    }
    return (header);
}

ParseResult check_repeated_key(std::vector<S_Header>& header)
{
    std::string keys[] = {
        "host:", "content-length:", "authorization:", "content-type:", "user-agent:", "referer:", "cookie:", "if-modified-since:", "if-none-match:", "END"
    };

    int i = 0;
    while (keys[i] != "END")
    {
        int counter = 0;
        std::vector<S_Header>::iterator begin = header.begin();
        std::vector<S_Header>::iterator end = header.end();

        while (begin != end)
        {
            std::string header_key = to_lower(begin->key);
            if (header_key == keys[i]) {
                counter++;
                if (counter > 1)
                    return (BadRequest);
            }
            begin++;
        }
        i++;
    }
    return (OK);
}

bool is_invalid_key_char(unsigned char c) 
{
    return std::iscntrl(c) || c >= 128;
}

bool is_invalid_value_char(unsigned char c) 
{
    if (c != '\t' && std::iscntrl(c)) return true;  // tab is allowed
    if (c >= 128) return true;
    return false;
}


bool header_invalid_chars(const std::string& key, const std::string& value) 
{
    for (std::string::size_type i = 0; i < key.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(key[i]);
        if ( is_invalid_key_char(c) ) return true;
    }

    for (std::string::size_type i = 0; i < value.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(value[i]);
        if ( is_invalid_value_char(c) ) return true;
    }
    return false;
}

int hex_char_to_int(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1; // invalid hex char
}

int hex_to_int(const std::string& hex_str) {
    size_t result = 0;
    for (size_t i = 0; i < hex_str.size(); ++i) {
        int val = hex_char_to_int(hex_str[i]);
        if (val == -1)
            return -1; // Invalid hex char
        result = result * 16 + val;
    }
    if (result > UINT_MAX)
        return (-1);
    return (static_cast<int>(result));
}