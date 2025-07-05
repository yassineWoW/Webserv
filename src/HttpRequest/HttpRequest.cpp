#include "HttpRequest.hpp"

HttpRequest::HttpRequest():r_content_length(0), r_has_content_length(false), r_has_transfer_encoding(false) {

}

HttpRequest::~HttpRequest () { }